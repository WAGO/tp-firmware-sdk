//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Cancellable future, capable of push-notification.
//------------------------------------------------------------------------------

#ifndef INC_WAGO_FUTURE_HPP_
#define INC_WAGO_FUTURE_HPP_

#include <wc/assertion.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <list>
#include <mutex>
#include <vector>
#include <utility>

namespace wago {


namespace impl {


template <typename T>
struct future_type
{
    using value_type = T;
    using variable_type = T;
    using future_notifier = std::function<void(T&&)>;
};


template <>
struct future_type<void>
{
    using value_type = void;
    using variable_type = bool; // bool type is used as a placeholder and not expected to be used for any specific task.
    using future_notifier = std::function<void(void)>;
};


} // namespace impl


///
/// Definition of a callback function for future's success notifier.
/// On success the function will be called and promised value will be passed via move semantics as its parameter.
/// Important: in order to avoid unnecessary copying of the input parameter implementations of the callback
/// should use std::move(..) in order to move it into its desired location.
/// @code
/// std::promise<some_class_type> p;
/// std::future<some_class_type> f = p.get_future();
///
///
/// some_class_type value_storage;
///
/// f.set_notifier([&value_storage](some_class_type&& value)
///     {
///         // Processing of the notification.
///         // Please note 'std::move' being used on the value parameter.
///         value_storage = std::move(value);
///     });
/// @endcode
/// Please note that the callback may have one of the two forms:
/// @code
/// void callback(non_void_type&& param);   // For non-void future/promise T types, e.g.: wago::future<int>.
/// void callback(void);                    // If future/promise is declared with void as T, e.g.: wago::future<void>.
/// @endcode
///
template <typename T>
using future_notifier = typename impl::future_type<T>::future_notifier;


///
/// Definition of a callback function for future's exception notifier.
/// On exception the function is called and exception pointer is passed as its parameter.
///
using future_exception_notifier = std::function<void(std::exception_ptr)>;


///
/// Definition of a callback function for future's dismissal notifier.
/// The function is called only on the first wago::future::dismiss call. Each subsequent dismiss call is ignored.
/// Calling any wago::future or wago::promise functions may lead to a deadlock. Calling dtors is safe.
///
using future_dismiss_notifier = std::function<void(void)>;


namespace impl {


template <typename T>
struct future_shared_state
{
    // Access to all variables must be protected by the guard locked using std::unique_lock.
    // Exceptions are marked individually.
    std::mutex guard{};
    std::condition_variable cv_guard{};

    bool future_retrieved = false; // Not protected by the guard.
    std::atomic_bool ready{false}; // Protected by the guard, however atomic to allow fast checks.
    std::atomic_bool execute{true}; // Not protected by the guard.
    bool has_value = false;
    bool has_exception = false;
    typename impl::future_type<T>::variable_type value{};
    std::exception_ptr exception{};

    future_notifier<T> notifier{};
    future_exception_notifier ex_notifier{};
    future_dismiss_notifier di_notifier{};

    std::condition_variable_any* notify = nullptr;
};


template <typename Iterator> struct futures_lock;
template <typename Iterator> struct futures_notifier;


} // namespace impl


template <typename T> class promise;


///
/// A [std::future](https://en.cppreference.com/w/cpp/thread/future)-like class with cancellation
/// and notification extensions.
/// In addition to the standard future functionality this variant allows to check in a non-blocking manner
/// if its result is ready and also to use a notification callbacks.
/// Here is the traditional approach:
/// @code
/// wago::promise<int> p;
/// wago::future<int> f = p.get_future();
///
/// std::thread th([&p] { std::this_thread::sleep_for(3s); p.set_value(3); });
///
/// cout << "Has value: " << f.has_value() << endl;
/// cout << "Result: " << f.get() << endl;
///
/// th.join();
///
/// // Result:
/// Has value: 0
/// Result: 3
/// @endcode
/// Please note that the initial "wago::future::has_value" result is negative. This is due to the fact that
/// the promise is not yet executed as its execution thread sleeps for 3 seconds at this point.
/// The example below shows a usage of notifier callback:
/// @code
/// wago::promise<int> p;
/// wago::future<int> f = p.get_future();
///
/// f.set_notifier([](int&& value) { cout << "Value is returned: " << value << endl; });
///
/// p.set_value(3);
///
/// cout << "Is ready: " << f.ready() << endl;
/// cout << "Has value: " << f.has_value() << endl;
///
/// // Result:
/// Value is returned: 3
/// Is ready: 1
/// Has value: 0
/// @endcode
/// Please note that the notifier function (lambda) takes a move reference. In this case it is not strictly
/// required as it is an int value, however, it might be crucial if bigger objects are passed.
/// Also the results show that the future returns 'ready' status to be true, but no value. This is due to
/// the fact that the promise was fulfilled, however, results were retrieved via notifier callback.
///
template <typename T>
class future
{
public:
    using value_type = T;

    ///
    /// Creates an invalid future.
    ///
    future() = default;
    future(const future&) = delete;
    future(future&& rhs)
        : state(std::exchange(rhs.state, nullptr)) {}
    ~future() noexcept = default;
    future& operator=(const future&) = delete;
    future& operator=(future&& rhs)
        { std::swap(state, rhs.state); return *this; }

    ///
    /// Returns true if the future object has a shared state - it is/was attached to a promise.
    ///
    bool valid() const noexcept
        { return (bool)state; }

    ///
    /// Returns true if the future is valid and its result is ready.
    /// Please note that if a future_notifier callback is set the result is likely already received
    /// and calling 'get' is going to result in an exception.
    ///
    bool ready() const
        { return valid() && state->ready; }

    ///
    /// Returns true if the future is valid and it was dismissed.
    /// Please note that despite dismissal the promise may still be processed and results provided
    /// as it depends on potentially multithreaded execution.
    ///
    bool dismissed() const
        { return valid() && !state->execute; }

    ///
    /// Returns true if the future is valid, promise execution was successfully finished and a value
    /// is ready to be received via 'get' function.
    ///
    inline bool has_value() const;

    ///
    /// Returns true if the future is valid, however promise execution failed and an exception
    /// is ready to be received via 'get_exception' function.
    ///
    inline bool has_exception() const;

    ///
    /// Retrieves futures result.
    /// The function waits until the future has a valid result.
    /// After the call future becomes invalid.
    ///
    inline typename impl::future_type<T>::value_type get();

    ///
    /// Retrieves futures exception - set if execution of promise failed.
    /// The function waits until the future has a valid result set - future's value or exception.
    /// After the call future becomes invalid.
    ///
    inline std::exception_ptr get_exception();

    ///
    /// Waits until a valid result is returned (a value or an exception).
    ///
    inline void wait() const;

    ///
    /// Waits until a valid result is returned (a value or an exception) or a timeout occurs.
    /// @code
    /// wago::promise<int> p;
    /// wago::future<int> f = p.get_future();
    ///
    /// std::thread th([&p] { std::this_thread::sleep_for(1s); p.set_value(3); });
    /// std::future_status fs = f.wait_for(3s);
    ///
    /// if (std::future_status::ready == fs)
    /// {
    ///     // Future is ready. React...
    /// }
    /// else
    /// {
    ///     // Future is NOT yet ready. React...
    /// }
    /// @endcode
    /// @param rel_time - timeout length.
    ///
    template <typename Rep, typename Period>
    inline std::future_status wait_for(const std::chrono::duration<Rep, Period>& rel_time) const;

    ///
    /// Waits until a valid result is returned (a value or an exception) or a specified time point is reached.
    /// @code
    /// wago::promise<int> p;
    /// wago::future<int> f = p.get_future();
    ///
    /// std::thread th([&p] { std::this_thread::sleep_for(1s); p.set_value(3); });
    /// std::future_status fs = f.wait_until(std::chrono::system_clock::now() + 3s);
    ///
    /// if (std::future_status::ready == fs)
    /// {
    ///     // Future is ready. React...
    /// }
    /// else
    /// {
    ///     // Future is NOT yet ready. React...
    /// }
    /// @endcode
    /// @param abs_time - time point designating a wait deadline.
    ///
    template <typename Clock, typename Duration>
    inline std::future_status wait_until(const std::chrono::time_point<Clock, Duration>& abs_time) const;

    ///
    /// Marks fulfillment of the future/promise combination as no longer required.
    /// Please note that promise may still be delivered if its execution was commenced or finished
    /// before dismissal.
    ///
    inline void dismiss();

    ///
    /// Sets a callback used on promise's successful execution.
    /// Please note that only callback or 'get' function shall be used as only one of them will
    /// may retrieve results correctly.
    /// If the callback is set after successful completion of promise execution the callback is still
    /// going to be called.
    /// @param notifier - callback function.
    ///
    inline void set_notifier(future_notifier<T>&& notifier);

    ///
    /// Sets a callback used on promise's unsuccessful execution.
    /// Please note that only callback or 'get_exception' function shall be used as only one of them will
    /// may retrieve results correctly.
    /// If the callback is set after unsuccessful completion of promise execution the callback is still
    /// going to be called.
    /// @param ex_notifier - callback function.
    ///
    inline void set_exception_notifier(future_exception_notifier&& ex_notifier);

private:
    template <typename> friend class promise;
    friend class promise<void>;
    template <typename> friend struct impl::futures_lock;
    template <typename> friend struct impl::futures_notifier;


    using shared_state = std::shared_ptr<impl::future_shared_state<T>>;

    shared_state state{};

    future(shared_state state_)
        : state(state_) {}

    inline void notify(std::condition_variable_any* n);
    inline void unnotify(std::condition_variable_any* n);
};


template <typename T>
inline bool future<T>::has_value() const
{
    if (!valid())
    {
        return false;
    }
    std::unique_lock<std::mutex> lock(state->guard);
    return state->has_value;
}


template <typename T>
inline bool future<T>::has_exception() const
{
    if (!valid())
    {
        return false;
    }
    std::unique_lock<std::mutex> lock(state->guard);
    return state->has_exception;
}


template <typename T>
inline typename impl::future_type<T>::value_type future<T>::get()
{
    if (!valid())
    {
        throw std::future_error(std::future_errc::future_already_retrieved);
    }
    shared_state s;
    {
        std::unique_lock<std::mutex> lock(state->guard);

        if (state->ready && (!state->has_value && !state->has_exception))
        {
            throw std::future_error(std::future_errc::future_already_retrieved);
            // This might happen if there is a future_notifier set and then get is called.
        }
        state->cv_guard.wait(lock, [this] { return this->state->ready.load(); });

        WC_ASSERT(state->has_value != state->has_exception);

        s = state;
        state = nullptr;
    }

    if (s->has_exception)
    {
        std::rethrow_exception(s->exception);
    }
    return std::move(s->value);
}


template <>
inline void future<void>::get()
{
    if (!valid())
    {
        throw std::future_error(std::future_errc::future_already_retrieved);
    }
    shared_state s;
    {
        std::unique_lock<std::mutex> lock(state->guard);

        if (state->ready && (!state->has_value && !state->has_exception))
        {
            throw std::future_error(std::future_errc::future_already_retrieved);
            // This might happen if there is a future_notifier set and then get is called.
        }
        state->cv_guard.wait(lock, [this] { return this->state->ready.load(); });

        WC_ASSERT(state->has_value != state->has_exception);

        s = state;
        state = nullptr;
    }

    if (s->has_exception)
    {
        std::rethrow_exception(s->exception);
    }
}


template <typename T>
inline std::exception_ptr future<T>::get_exception()
{
    if (!valid())
    {
        throw std::future_error(std::future_errc::future_already_retrieved);
    }
    shared_state s;
    {
        std::unique_lock<std::mutex> lock(state->guard);

        if (state->ready && (!state->has_value && !state->has_exception))
        {
            throw std::future_error(std::future_errc::future_already_retrieved);
            // This might happen if there is a future_notifier set and then get is called.
        }
        state->cv_guard.wait(lock, [this] { return this->state->ready.load(); });

        WC_ASSERT(state->has_value != state->has_exception);

        s = state;
        state = nullptr;
    }

    return std::move(s->exception);
}


template <typename T>
inline void future<T>::wait() const
{
    if (!valid())
    {
        throw std::future_error(std::future_errc::future_already_retrieved);
    }
    std::unique_lock<std::mutex> lock(state->guard);
    state->cv_guard.wait(lock, [this] { return this->state->ready.load(); });
}


template <typename T>
template <typename Rep, typename Period>
inline std::future_status future<T>::wait_for(const std::chrono::duration<Rep, Period>& rel_time) const
{
    if (!valid())
    {
        throw std::future_error(std::future_errc::future_already_retrieved);
    }
    std::unique_lock<std::mutex> lock(state->guard);
    const bool pred = state->cv_guard.wait_for(lock, rel_time, [this] { return this->state->ready.load(); });

    return pred ? std::future_status::ready : std::future_status::timeout;
}


template <typename T>
template <typename Clock, typename Duration>
inline std::future_status future<T>::wait_until(const std::chrono::time_point<Clock, Duration>& abs_time) const
{
    if (!valid())
    {
        throw std::future_error(std::future_errc::future_already_retrieved);
    }
    std::unique_lock<std::mutex> lock(state->guard);
    const bool pred = state->cv_guard.wait_until(lock, abs_time, [this] { return this->state->ready.load(); });

    return pred ? std::future_status::ready : std::future_status::timeout;
}


template <typename T>
inline void future<T>::dismiss()
{
    if (!valid())
    {
        return;
    }
    if (state->execute)
    {
        std::unique_lock<std::mutex> lock(state->guard);

        state->execute = false;

        if (state->di_notifier)
            state->di_notifier();
    }
}


template <typename T>
inline void future<T>::set_notifier(future_notifier<T>&& notifier)
{
    if (!valid())
    {
        throw std::future_error(std::future_errc::future_already_retrieved);
    }
    std::unique_lock<std::mutex> lock(state->guard);
    state->notifier = std::move(notifier);

    if (state->ready && state->has_value)
    {
        state->has_value = false;

        state->notifier(std::move(state->value));
    }
}


template <>
inline void future<void>::set_notifier(future_notifier<void>&& notifier)
{
    if (!valid())
    {
        throw std::future_error(std::future_errc::future_already_retrieved);
    }
    std::unique_lock<std::mutex> lock(state->guard);
    state->notifier = std::move(notifier);

    if (state->ready && state->has_value)
    {
        state->has_value = false;

        state->notifier();
    }
}


template <typename T>
inline void future<T>::set_exception_notifier(future_exception_notifier&& ex_notifier)
{
    if (!valid())
    {
        throw std::future_error(std::future_errc::future_already_retrieved);
    }
    std::unique_lock<std::mutex> lock(state->guard);
    state->ex_notifier = std::move(ex_notifier);

    if (state->ready && state->has_exception)
    {
        state->has_exception = false;

        state->ex_notifier(state->exception);
    }
}


template <typename T>
inline void future<T>::notify(std::condition_variable_any* n)
{
    if (nullptr == n || !valid())
    {
        return;
    }
    state->notify = n;
}


template <typename T>
inline void future<T>::unnotify(std::condition_variable_any* n)
{
    if (nullptr == n || !valid())
    {
        return;
    }
    WC_ASSERT(state->notify == n);
    state->notify = nullptr;
}


///
/// A [std::promise](https://en.cppreference.com/w/cpp/thread/promise)-like class with cancellation and notification extensions.
/// For more details see wago::future class description.
///
template <typename T>
class promise
{
public:
    using value_type = T;

    ///
    /// Creates a new promise and immediately sets an dismissal notifier.
    ///
    static promise<T> create_with_dismiss_notifier(future_dismiss_notifier&& di_notifier)
        { return promise<T>(future_notifier<T>{}, future_exception_notifier{}, std::move(di_notifier)); }

    ///
    /// Creates a new promise.
    ///
    promise()
        : state(std::make_shared<impl::future_shared_state<T>>()) {}
    promise(const promise&) = delete;
    promise(promise&& rhs)
        : state(std::exchange(rhs.state, std::make_shared<impl::future_shared_state<T>>())) {}

    ///
    /// Creates a new promise and immediately sets a completion notifier.
    /// Result is analogous to calling future::set_notifier function.
    ///
    promise(future_notifier<T>&& notifier)
        : promise() { state->notifier = notifier; }

    ///
    /// Creates a new promise and immediately sets an exception notifier.
    /// Result is analogous to calling future::set_exception_notifier function.
    ///
    promise(future_exception_notifier&& ex_notifier)
        : promise() { state->ex_notifier = ex_notifier; }

    ///
    /// Creates a new promise and immediately sets a completion and exception notifiers.
    /// Result is analogous to calling future::set_notifier and future::set_exception_notifier functions.
    ///
    promise(future_notifier<T>&& notifier, future_exception_notifier&& ex_notifier)
        : promise() { state->notifier = notifier; state->ex_notifier = ex_notifier; }

    ///
    /// Creates a new promise and immediately sets a completion, exception and dismissal notifiers.
    /// Result is analogous to calling future::set_notifier, future::set_exception_notifier
    /// and future::set_dismiss_notifier functions.
    ///
    promise(future_notifier<T>&& notifier,
            future_exception_notifier&& ex_notifier,
            future_dismiss_notifier&& di_notifier)
        : promise() { state->notifier = notifier; state->ex_notifier = ex_notifier; state->di_notifier = di_notifier; }

    ///
    /// If value wasn't returned it breaks the promise - an exception is returned.
    /// Otherwise no special action is executed.
    ///
    inline ~promise() noexcept;
    promise& operator=(const promise&) = delete;
    promise& operator=(promise&& rhs)
        { std::swap(state, rhs.state); return *this; }

    ///
    /// Sets a dismiss notifier.
    /// If a promise is dismissed at the time of setting of the notifier, the notifier will
    /// be called immediately.
    /// Setting notifier second time will replace its predecessor.
    /// @param di_notifier - dismiss notifier callback function 
    ///
    inline void set_dismiss_notifier(future_dismiss_notifier&& di_notifier);

    ///
    /// Returns a future connected to the promise.
    /// Please note that only one future may be returned. Each subsequent call will fail.
    /// @return future connected to the promise.
    ///
    inline future<T> get_future();

    ///
    /// Returns true if the promise was not dismissed and should still be executed.
    /// Dismissal is triggered by future::dismiss function.
    ///
    bool execute() const // Returns true if future was not dismissed and promise shall be executed.
        { return state->execute; }

    ///
    /// Sets a result to a specific value.
    /// Once the value is set the future may retrieve it and 'get' and 'wait*' functions will no longer block.
    /// If a future has notifier set, the notifier callback will be executed.
    ///
    inline void set_value(T&& value);

    ///
    /// Sets a result to a specific exception (pointer).
    /// Once the pointer is set the future may retrieve it and 'get_exception' and 'wait*'
    /// functions will no longer block.
    /// If a future has exception notifier set, the notifier callback will be executed.
    ///
    inline void set_exception(std::exception_ptr p);

private:
    std::shared_ptr<impl::future_shared_state<T>> state;
};


template <typename T>
inline promise<T>::~promise() noexcept
{
    if (!state->ready && state->execute)
    {
        try
        {
            set_exception(std::make_exception_ptr(std::future_error(std::future_errc::broken_promise)));
        }
        catch (...) // parasoft-suppress CERT_CPP-ERR56-b-2 "No further error handling possible"
        {
            // Intentionally does not throw.
        }
    }
}


template <typename T>
inline void promise<T>::set_dismiss_notifier(future_dismiss_notifier&& di_notifier)
{
    std::unique_lock<std::mutex> lock(state->guard);

    state->di_notifier = std::move(di_notifier);

    if (!state->execute && state->di_notifier)
    {
        state->di_notifier();
    }
}


template <typename T>
inline future<T> promise<T>::get_future()
{
    if (state->future_retrieved)
    {
        throw std::future_error(std::future_errc::future_already_retrieved);
    }
    state->future_retrieved = true;
    return future<T>(state);
}


template <typename T>
inline void promise<T>::set_value(T&& value)
{
    if (state->ready)
    {
        throw std::future_error(std::future_errc::promise_already_satisfied);
    }
    std::unique_lock<std::mutex> lock(state->guard);

    state->ready = true;

    if (!state->notifier)
    {
        state->value = std::move(value);
        state->has_value = true;
    }
    else
    {
        state->notifier(std::move(value));
    }

    // Remove dismiss notifier to release any objects
    state->di_notifier = nullptr;

    state->cv_guard.notify_all();
    if (nullptr != state->notify)
    {
        state->notify->notify_all();
    }
}


template <typename T>
inline void promise<T>::set_exception(std::exception_ptr p)
{
    if (state->ready)
    {
        throw std::future_error(std::future_errc::promise_already_satisfied);
    }
    std::unique_lock<std::mutex> lock(state->guard);

    state->ready = true;

    if (!state->ex_notifier)
    {
        state->exception = p;
        state->has_exception = true;
    }
    else
    {
        state->ex_notifier(p);
    }

    // Remove dismiss notifier to release any objects
    state->di_notifier = nullptr;

    state->cv_guard.notify_all();
    if (nullptr != state->notify)
    {
        state->notify->notify_all();
    }
}


///
/// Explicit specialization of wago::promise for void type.
/// For more details see wago::promise class description.
///
template <>
class promise<void>
{
public:
    using value_type = void;

    static promise<void> create_with_dismiss_notifier(future_dismiss_notifier&& di_notifier)
        { return promise<void>(future_notifier<void>{}, future_exception_notifier{}, std::move(di_notifier)); }

    promise()
        : state(std::make_shared<impl::future_shared_state<void>>()) {}
    promise(const promise&) = delete;
    promise(promise&& rhs)
        : state(std::exchange(rhs.state, std::make_shared<impl::future_shared_state<void>>())) {}

    promise(future_notifier<void>&& notifier)
        : promise() { state->notifier = notifier; }

    promise(future_exception_notifier&& ex_notifier)
        : promise() { state->ex_notifier = ex_notifier; }

    promise(future_notifier<void>&& notifier, future_exception_notifier&& ex_notifier)
        : promise() { state->notifier = notifier; state->ex_notifier = ex_notifier; }

    promise(future_notifier<void>&& notifier,
            future_exception_notifier&& ex_notifier,
            future_dismiss_notifier&& di_notifier)
        : promise() { state->notifier = notifier; state->ex_notifier = ex_notifier; state->di_notifier = di_notifier; }

    ~promise() noexcept
    {
        if (!state->ready && state->execute)
        {
            try
            {
                set_exception(std::make_exception_ptr(std::future_error(std::future_errc::broken_promise)));
            }
            catch (...) // parasoft-suppress CERT_CPP-ERR56-b-2 "No further error handling possible"
            {
                // Intentionally does not throw.
            }
        }
    }

    promise& operator=(const promise&) = delete;
    promise& operator=(promise&& rhs)
        { std::swap(state, rhs.state); return *this; }

    void set_dismiss_notifier(future_dismiss_notifier&& di_notifier)
    {
        std::unique_lock<std::mutex> lock(state->guard);

        state->di_notifier = std::move(di_notifier);

        if (!state->execute && state->di_notifier)
        {
            state->di_notifier();
        }
    }

    future<void> get_future()
    {
        if (state->future_retrieved)
        {
            throw std::future_error(std::future_errc::future_already_retrieved);
        }
        state->future_retrieved = true;
        return future<void>(state);
    }

    bool execute() const // Returns true if future was not dismissed and promise shall be executed.
        { return state->execute; }

    void set_value()
    {
        if (state->ready)
        {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }
        std::unique_lock<std::mutex> lock(state->guard);

        state->ready = true;

        if (!state->notifier)
        {
            state->has_value = true;
        }
        else
        {
            state->notifier();
        }

        state->cv_guard.notify_all();
        if (nullptr != state->notify)
        {
            state->notify->notify_all();
        }
    }

    void set_exception(std::exception_ptr p)
    {
        if (state->ready)
        {
            throw std::future_error(std::future_errc::promise_already_satisfied);
        }
        std::unique_lock<std::mutex> lock(state->guard);

        state->ready = true;

        if (!state->ex_notifier)
        {
            state->exception = p;
            state->has_exception = true;
        }
        else
        {
            state->ex_notifier(p);
        }

        state->cv_guard.notify_all();
        if (nullptr != state->notify)
        {
            state->notify->notify_all();
        }
    }

private:
    std::shared_ptr<impl::future_shared_state<void>> state;
};


namespace impl {


template <typename Iterator>
struct futures_lock
{
    futures_lock(Iterator begin, Iterator end)
        : locks()
    {
        auto count = std::distance(begin, end);

        if (count <=0 )
        {
            throw std::invalid_argument("Invalid arguments.");
        }
        locks.reserve(count);

        for (auto i = begin; i != end; ++i)
        {
            if (i->valid())
            {
                locks.push_back(std::unique_lock<std::mutex>(i->state->guard));
            }
        }
    }

    void lock()
    {
        for (std::unique_lock<std::mutex>& ul : locks)
        {
            ul.lock();
        }
    }

    void unlock()
    {
        for (std::unique_lock<std::mutex>& ul : locks)
        {
            ul.unlock();
        }
    }

private:
    std::vector<std::unique_lock<std::mutex>> locks;
};


template <typename Iterator>
struct futures_notifier
{
    futures_notifier(Iterator begin_, Iterator end_, std::condition_variable_any* n)
        : begin(begin_), end(end_), notifier(n)
    {
        WC_ASSERT(begin != end);
        WC_ASSERT(nullptr != n);

        for (auto i = begin; i != end; ++i)
        {
            i->notify(notifier);
        }
    }

    ~futures_notifier() noexcept
    {
        for (auto i = begin; i != end; ++i)
        {
            i->unnotify(notifier);
        }
    }

private:
    Iterator begin;
    Iterator end;
    std::condition_variable_any* notifier;
};


} // namespace impl


///
/// The function executes an atomic wait on all provided futures.
/// [The implementation follows boost's version.](https://www.boost.org/doc/libs/1_70_0/doc/html/thread/synchronization.html#thread.synchronization.futures.reference.wait_for_any)
/// If any future in the sequence has a notifier set or is invalid the result is undefined.
/// @code
/// std::vector<wago::future<int>> futures;
///
/// // Add some futures to the 'futures' collection.
///
/// auto first = wago::wait_for_any(std::begin(futures), std::end(futures));
///
/// for (; first != std::end(futures); ++first)
/// {
///     if (first->ready())
///     {
///         // Process ready future.
///     }
/// }
/// @endcode
/// @param begin - starting iterator to a sequence of futures.
/// @param end - ending iterator to a sequence of futures.
/// @return Iterator to the first ready future from the provided sequence.
///
template <typename Iterator>
inline Iterator wait_for_any(Iterator begin, Iterator end)
{
    if (begin == end)
    {
        return end;
    }
    std::condition_variable_any cv;
    impl::futures_lock<Iterator> guard(begin, end);
    impl::futures_notifier<Iterator> notifier(begin, end, &cv);

    while (true)
    {
        using value_type = typename std::iterator_traits<Iterator>::value_type;
        auto rdy = std::find_if(begin, end, [](const value_type& f) { return f.ready(); });

        if (rdy != end)
        {
            return rdy;
        }
        cv.wait(guard);
    }
}


///
/// The function executes an atomic wait on all provided futures.
/// If no future becomes ready the wait ends automatically after a provided period of time.
/// If any future in the sequence has a notifier set or is invalid the result is undefined.
/// @code
/// std::vector<wago::future<int>> futures;
///
/// // Add some futures to the 'futures' collection.
///
/// using namespace std::chrono_literals;
///
/// auto first = wago::wait_for_any_for(std::begin(futures), std::end(futures), 1s);
///
/// if (first != std::end(futures))
/// {
///     for (; first != std::end(futures); ++first)
///     {
///         if (first->ready())
///         {
///             // Process ready future.
///         }
///     }
/// }
/// else
/// {
///     // Timeout hit. No futures are ready.
/// }
/// @endcode
/// @param begin - starting iterator to a sequence of futures.
/// @param end - ending iterator to a sequence of futures.
/// @param rel_time - timeout length.
/// @return Iterator to the first ready future from the provided sequence. If timeout was hit
///         the 'end' iterator will be returned instead.
///
template <typename Iterator, typename Rep, typename Period>
inline Iterator wait_for_any_for(Iterator begin, Iterator end,
                                 const std::chrono::duration<Rep, Period>& rel_time)
{
    if (begin == end)
    {
        return end;
    }
    std::condition_variable_any cv;
    impl::futures_lock<Iterator> guard(begin, end);
    impl::futures_notifier<Iterator> notifier(begin, end, &cv);

    while (true)
    {
        using value_type = typename std::iterator_traits<Iterator>::value_type;
        auto rdy = std::find_if(begin, end, [](const value_type& f) { return f.ready(); });

        if (rdy != end)
        {
            return rdy;
        }
        const std::cv_status cvs = cv.wait_for(guard, rel_time);

        if (std::cv_status::timeout == cvs)
        {
            return end;
        }
    }
}


///
/// The function executes an atomic wait on all provided futures.
/// If no future becomes ready the wait ends automatically after a specified time point is reached.
/// If any future in the sequence has a notifier set or is invalid the result is undefined.
/// @code
/// std::vector<wago::future<int>> futures;
///
/// // Add some futures to the 'futures' collection.
///
/// using namespace std::chrono_literals;
///
/// auto first = wago::wait_for_any_until(std::begin(futures), std::end(futures),
///                                       std::chrono::system_clock::now() + 1s);
///
/// if (first != std::end(futures))
/// {
///     for (; first != std::end(futures); ++first)
///     {
///         if (first->ready())
///         {
///             // Process ready future.
///         }
///     }
/// }
/// else
/// {
///     // Timeout hit. No futures are ready.
/// }
/// @endcode
/// @param begin - starting iterator to a sequence of futures.
/// @param end - ending iterator to a sequence of futures.
/// @param abs_time - time point designating a wait deadline.
/// @return Iterator to the first ready future from the provided sequence. If timeout was hit
///         the 'end' iterator will be returned instead.
///
template <typename Iterator, typename Clock, typename Duration>
inline Iterator wait_for_any_until(Iterator begin, Iterator end,
                                   const std::chrono::time_point<Clock, Duration>& abs_time)
{
    if (begin == end)
    {
        return end;
    }
    std::condition_variable_any cv;
    impl::futures_lock<Iterator> guard(begin, end);
    impl::futures_notifier<Iterator> notifier(begin, end, &cv);

    while (true)
    {
        using value_type = typename std::iterator_traits<Iterator>::value_type;
        auto rdy = std::find_if(begin, end, [](const value_type& f) { return f.ready(); });

        if (rdy != end)
        {
            return rdy;
        }
        const std::cv_status cvs = cv.wait_until(guard, abs_time);

        if (std::cv_status::timeout == cvs)
        {
            return end;
        }
    }
}


///
/// Returns a resolved future with a given value set.
/// This is a convenience function which may be used in situations in which response
/// is available immediately and keeping promise object is not necessary.
/// @code
/// wago::future<int> f = wago::resolved_future(55);
///
/// // A resolved future 'f' is at this point available...
/// @endcode
/// @param value - value to be returned by the resolved future object.
/// @return A future object with 'value' already set.
///
template <typename T>
inline future<T> resolved_future(T&& value)
{
    auto p = promise<T>();
    p.set_value(std::move(value));
    return p.get_future();
}


inline future<void> resolved_future()
{
    auto p = promise<void>();
    p.set_value();
    return p.get_future();
}


///
/// Returns a resolved future with a given exception pointer set.
/// This is a convenience function which may be used in situations in which response
/// is available immediately and keeping promise object is not necessary.
/// @code
/// wago::future<int> f = wago::resolved_future<int>(std::make_exception_ptr(std::runtime_error("an exception")));
///
/// // A resolved future 'f' is at this point available...
/// @endcode
/// @param exp - exception pointer to be returned by the resolved future object.
/// @return A future object with 'ex' exception already set.
///
template <typename T>
inline future<T> resolved_future(std::exception_ptr ex)
{
    auto p = promise<T>();
    p.set_exception(ex);
    return p.get_future();
}


} // namespace wago


#endif // INC_WAGO_FUTURE_HPP_
