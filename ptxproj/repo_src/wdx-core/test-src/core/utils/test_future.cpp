//------------------------------------------------------------------------------
// Copyright (c) 2024 WAGO GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Test future/promise.
///
///  \author   MP:  WAGO GmbH & Co. KG
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/future.hpp"

#include <array>
#include <list>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

TEST(wstd_future, empty_future)
{
    wago::future<int> f;

	ASSERT_FALSE(f.valid()) << "Empty future must be invalid.";
	ASSERT_FALSE(f.ready()) << "Empty future must not be ready.";
	ASSERT_FALSE(f.dismissed()) << "Empty future must not be dismissed.";

	ASSERT_FALSE(f.has_value()) << "Empty future must not have value.";
	ASSERT_FALSE(f.has_exception()) << "Empty future must not have exception (value).";


    try {
        (void)f.get();
        FAIL() << "Empty future should throw on 'get'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'get' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'get' - unknown exception thrown instead.";
    }


    try {
        (void)f.get_exception();
        FAIL() << "Empty future should throw on 'get_exception'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'get_exception' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'get_exception'"\
                  " - unknown exception thrown instead.";
    }


    try {
        f.wait();
        FAIL() << "Empty future should throw on 'wait'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'wait' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'wait' - unknown exception thrown instead.";
    }


    try {
        (void)f.wait_for(std::chrono::seconds(3));
        FAIL() << "Empty future should throw on 'wait_for'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'wait_for' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'wait_for' - unknown exception thrown instead.";
    }


    try {
        (void)f.wait_until(std::chrono::system_clock::now() + std::chrono::seconds(3));
        FAIL() << "Empty future should throw on 'wait_until'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'wait_until' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'wait_until'"\
                  " - unknown exception thrown instead.";
    }


    try {
        f.dismiss();
    }
    catch (...) {
        FAIL() << "Empty future should allow dismissal without throwing exceptions.";
    }


    try {
        f.set_notifier([](int) {});
        FAIL() << "Empty future should throw on 'set_notifier'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'set_notifier' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'set_notifier'"\
                  " - unknown exception thrown instead.";
    }


    try {
        f.set_exception_notifier([](std::exception_ptr) {});
        FAIL() << "Empty future should throw on 'set_exception_notifier'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'set_exception_notifier' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'set_exception_notifier'"\
                  " - unknown exception thrown instead.";
    }
}


TEST(wstd_future, move)
{
    wago::promise<int> pA;
    wago::future<int> fA;

    try {
        wago::promise<int> p1;
        wago::future<int> f1(p1.get_future());
        wago::promise<int> p2(std::move(p1));
        wago::future<int> f2(std::move(f1));

        pA = std::move(p2);
        fA = std::move(f2);
    }
    catch (...) {
        FAIL() << "Moving future or promise failed (1).";
    }

    try {
        wago::promise<int> pB(std::move(pA));
        wago::future<int> fB(std::move(fA));

        pB = wago::promise<int>();
        fB = wago::future<int>();
    }
    catch (...) {
        FAIL() << "Moving future or promise failed (2).";
    }
}


TEST(wstd_future, basic_promise)
{
    {
        wago::promise<int> p;
        wago::future<int> f = p.get_future();

        ASSERT_TRUE(f.valid()) << "Freshly constructed promise shall return valide future.";
        ASSERT_TRUE(p.execute()) << "Freshly constructed promise shall be in 'execute' state.";
    }

    {
        wago::promise<int> p;

        try {
            p.set_value(3);
        }
        catch (...) {
            FAIL() << "Freshly constructed promise shall allow to set value.";
        }
    }

    {
        wago::promise<int> p;

        try {
            p.set_exception(std::make_exception_ptr(std::runtime_error("an exception")));
        }
        catch (...) {
            FAIL() << "Freshly constructed promise shall allow to set exception.";
        }
    }
}


TEST(wstd_future, single_future_get)
{
    using namespace std::chrono_literals;

    wago::promise<int> p;
    wago::future<int> f = p.get_future();

    std::thread th([&p] { std::this_thread::sleep_for(10ms); p.set_value(3); });

    ASSERT_TRUE(f.valid());
    ASSERT_FALSE(f.ready());
    ASSERT_FALSE(f.has_value());
    ASSERT_FALSE(f.has_exception());

    const int result = f.get();

    th.join();

    ASSERT_FALSE(f.valid());
    ASSERT_FALSE(f.ready());
    ASSERT_FALSE(f.has_value());
    ASSERT_FALSE(f.has_exception());
    ASSERT_TRUE(3 == result);
}


// This test check if future can return a valid result if the source promise is already gone.
// This checks if there is a bug present with deallocated memory usage. Unfortunately this bug
// does not necessarily cause failure of the unit tests as the memory is phisically still available.
// Results are visible only under valgrind.
TEST(wstd_future, promise_gone_single_future_get)
{
    using namespace std::chrono_literals;

    wago::future<int> f;

    {
        wago::promise<int> p;
        f = p.get_future();

        std::thread th([&p] { std::this_thread::sleep_for(10ms); p.set_value(3); });

        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        th.join();
    }

    const int result = f.get();

    ASSERT_FALSE(f.valid());
    ASSERT_FALSE(f.ready());
    ASSERT_FALSE(f.has_value());
    ASSERT_FALSE(f.has_exception());
    ASSERT_TRUE(3 == result);
}


// This test check if future can return a valid result if the source promise is already gone.
// This checks if there is a bug present with deallocated memory usage. Unfortunately this bug
// does not necessarily cause failure of the unit tests as the memory is phisically still available.
// Results are visible only under valgrind.
TEST(wstd_future, promise_gone_single_future_exception)
{
    using namespace std::chrono_literals;

    wago::future<int> f;

    {
        wago::promise<int> p;
        f = p.get_future();

        std::thread th([&p]
        {
            std::this_thread::sleep_for(10ms);
            p.set_exception(std::make_exception_ptr(std::runtime_error("an exception")));
        });

        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        th.join();
    }

    const std::exception_ptr result = f.get_exception();

    ASSERT_FALSE(f.valid());
    ASSERT_FALSE(f.ready());
    ASSERT_FALSE(f.has_value());
    ASSERT_FALSE(f.has_exception());
    ASSERT_TRUE(nullptr != result);
}


TEST(wstd_future, future_notifier)
{
    using namespace std::chrono_literals;

    {
        wago::promise<int> p;
        wago::future<int> f = p.get_future();
        int result = 0;

        f.set_notifier([&result](int&& value) { result = value; });

        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        p.set_value(3);

        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
        ASSERT_TRUE(3 == result);
    }
    {
        wago::promise<int> p;
        wago::future<int> f = p.get_future();
        int result = 0;

        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        p.set_value(3);
        f.set_notifier([&result](int&& value) { result = value; });

        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
        ASSERT_TRUE(3 == result);
    }
    {
        wago::promise<int> p;
        wago::future<int> f = p.get_future();
        std::mutex guard;
        std::condition_variable cv_guard;
        int result = 0;

        f.set_notifier([&result, &guard, &cv_guard](int&& value)
            {
                std::unique_lock<std::mutex> lock(guard);
                result = value;
                cv_guard.notify_all();
            });

        std::thread th([&p] { std::this_thread::sleep_for(10ms); p.set_value(3); });

        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        std::unique_lock<std::mutex> lock(guard);
        const bool success = cv_guard.wait_for(lock, 50ms, [&result] { return 3 == result; });

        th.join();

        ASSERT_TRUE(success);
        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
        ASSERT_TRUE(3 == result);
    }
}


// The purpose of this class is to detect when it was copied.
// Objects of this class should be always moved, despite the fact
// that copy ctor and operator exist.
// If any object was copied along of what should be a chain of moves
// it is considered a fatal error.
struct tmove
{
    bool copied = false;
    bool moved = false;
    int value = 0;          // This is just a test value.

    inline tmove() = default;

    inline tmove(int v) noexcept
        : value(v) {}

    inline tmove(const tmove& rhs) noexcept
        : copied(true), value(rhs.value)
    {
        //std::cout << "tmove: copy ctor." << std::endl;
    }

    inline tmove(tmove&& rhs) noexcept
        : copied(rhs.copied), moved(true), value(std::exchange(rhs.value, 0))
    {
        rhs.moved = true;
        //std::cout << "tmove: move ctor." << std::endl;
    }

    inline tmove& operator=(const tmove& rhs) noexcept
    {
        copied = true;
        value = rhs.value;
        //std::cout << "tmove: copy operator." << std::endl;
        return *this;
    }

    inline tmove& operator=(tmove&& rhs) noexcept
    {
        copied = rhs.copied; 
        moved = true;
        rhs.moved = true;
        std::swap(value, rhs.value); 
        //std::cout << "tmove: move operator." << std::endl;
        return *this;
    }

    inline bool is_consistent() const noexcept
        { return !(copied && moved); }
};


TEST(wstd_future, tmove)
{
    // Just to check if tmove does what it should do.

    tmove t1(3);
    tmove t2 = t1;

    ASSERT_TRUE(t2.is_consistent());
    ASSERT_TRUE(t2.copied);

    tmove t3 = std::move(t1);

    ASSERT_TRUE(t3.is_consistent());
    ASSERT_TRUE(t3.moved);
    ASSERT_TRUE(0 == t1.value);
    ASSERT_TRUE(3 == t3.value);

    tmove t4 = std::move(t2);

    ASSERT_FALSE(t4.is_consistent());
    ASSERT_TRUE(t4.copied);
    ASSERT_TRUE(t4.moved);
}


TEST(wstd_future, value_move_get)
{
    wago::promise<tmove> p;
    wago::future<tmove> f = p.get_future();

    tmove src_val(3);

    // p.set_value(src_val); // This statement should fail compilation.
    p.set_value(std::move(src_val));
    ASSERT_TRUE(0 == src_val.value);
    ASSERT_TRUE(src_val.is_consistent());
    ASSERT_FALSE(src_val.copied);
    ASSERT_TRUE(src_val.moved);

    tmove res_val = f.get();
    ASSERT_TRUE(3 == res_val.value);
    ASSERT_TRUE(res_val.is_consistent());
    ASSERT_FALSE(res_val.copied);
    ASSERT_TRUE(res_val.moved);
}


TEST(wstd_future, value_move_notifier)
{
    {
        wago::promise<tmove> p;
        wago::future<tmove> f = p.get_future();
        tmove res_val;

        f.set_notifier([&res_val](tmove&& value)
        {
            res_val = std::move(value);
        });

        tmove src_val(3);

        p.set_value(std::move(src_val));

        ASSERT_TRUE(0 == src_val.value);
        ASSERT_TRUE(src_val.is_consistent());
        ASSERT_FALSE(src_val.copied);
        ASSERT_TRUE(src_val.moved);
        
        ASSERT_TRUE(3 == res_val.value);
        ASSERT_TRUE(res_val.is_consistent());
        ASSERT_FALSE(res_val.copied);
        ASSERT_TRUE(res_val.moved);
    }
    {
        wago::promise<tmove> p;
        wago::future<tmove> f = p.get_future();
        tmove res_val;

        tmove src_val(3);

        p.set_value(std::move(src_val));

        f.set_notifier([&res_val](tmove&& value)
        {
            res_val = std::move(value);
        });
    
        ASSERT_TRUE(0 == src_val.value);
        ASSERT_TRUE(src_val.is_consistent());
        ASSERT_FALSE(src_val.copied);
        ASSERT_TRUE(src_val.moved);
        
        ASSERT_TRUE(3 == res_val.value);
        ASSERT_TRUE(res_val.is_consistent());
        ASSERT_FALSE(res_val.copied);
        ASSERT_TRUE(res_val.moved);
    }
}


TEST(wstd_future, future_wait)
{
    using namespace std::chrono_literals;

    wago::promise<int> p;
    wago::future<int> f = p.get_future();

    std::thread th([&p] { std::this_thread::sleep_for(10ms); p.set_value(3); });
    
    ASSERT_TRUE(f.valid());
    ASSERT_FALSE(f.ready());
    ASSERT_FALSE(f.has_value());
    ASSERT_FALSE(f.has_exception());

    f.wait();
    th.join();

    ASSERT_TRUE(f.valid());
    ASSERT_TRUE(f.ready());
    ASSERT_TRUE(f.has_value());
    ASSERT_FALSE(f.has_exception());

    const int result = f.get();

    ASSERT_FALSE(f.valid());
    ASSERT_FALSE(f.ready());
    ASSERT_FALSE(f.has_value());
    ASSERT_FALSE(f.has_exception());
}


TEST(wstd_future, future_wait_for)
{
    using namespace std::chrono_literals;
    
    {
        wago::promise<int> p;
        wago::future<int> f = p.get_future();
        
        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        std::thread th([&p] { std::this_thread::sleep_for(10ms); p.set_value(3); });
        std::future_status fs = f.wait_for(30ms);

        ASSERT_TRUE(std::future_status::ready == fs);
        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_TRUE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        const int result = f.get();
        th.join();

        ASSERT_FALSE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
    }
    {
        wago::promise<int> p;
        wago::future<int> f = p.get_future();
        
        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        std::thread th([&p] { std::this_thread::sleep_for(30ms); p.set_value(3); });
        std::future_status fs = f.wait_for(10ms);

        ASSERT_TRUE(std::future_status::timeout == fs);
        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        const int result = f.get();
        th.join();

        ASSERT_FALSE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
    }
}


TEST(wstd_future, future_wait_until)
{
    using namespace std::chrono_literals;
    
    {
        wago::promise<int> p;
        wago::future<int> f = p.get_future();
        
        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        std::thread th([&p] { std::this_thread::sleep_for(10ms); p.set_value(3); });
        std::future_status fs = f.wait_until(std::chrono::system_clock::now() + 30ms);

        ASSERT_TRUE(std::future_status::ready == fs);
        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_TRUE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        const int result = f.get();
        th.join();

        ASSERT_FALSE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
    }
    {
        wago::promise<int> p;
        wago::future<int> f = p.get_future();
        
        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        std::thread th([&p] { std::this_thread::sleep_for(30ms); p.set_value(3); });
        std::future_status fs = f.wait_until(std::chrono::system_clock::now() + 10ms);

        ASSERT_TRUE(std::future_status::timeout == fs);
        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        const int result = f.get();
        th.join();

        ASSERT_FALSE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
    }
}


TEST(wstd_future, wait_for_any)
{
    using namespace std::chrono_literals;

    constexpr std::size_t size = 100;
    constexpr std::size_t active = 33;

    std::array<wago::promise<int>, size> promises;
    std::array<wago::future<int>, size> futures;

    for (std::size_t i = 0; i < size; ++i)
        futures[i] = promises[i].get_future();

    std::thread th([&promises] { std::this_thread::sleep_for(10ms); promises[active].set_value(33); });

    auto first = wago::wait_for_any(std::begin(futures), std::end(futures));

    th.join();

    ASSERT_TRUE(first != std::begin(futures));
    ASSERT_TRUE(first != std::end(futures));
    ASSERT_TRUE(33 == first->get());
}


TEST(wstd_future, wait_for_any_vector)
{
    using namespace std::chrono_literals;

    constexpr std::size_t size = 100;
    constexpr std::size_t active = 33;

    std::array<wago::promise<int>, size> promises;
    std::vector<wago::future<int>> futures;

    for (wago::promise<int>& p : promises)
        futures.push_back(p.get_future());

    ASSERT_TRUE(size == futures.size());

    std::thread th([&promises] { std::this_thread::sleep_for(10ms); promises[active].set_value(33); });

    auto first = wago::wait_for_any(std::begin(futures), std::end(futures));

    th.join();

    ASSERT_TRUE(first != std::begin(futures));
    ASSERT_TRUE(first != std::end(futures));
    ASSERT_TRUE(33 == first->get());
}


TEST(wstd_future, wait_for_any_list)
{
    using namespace std::chrono_literals;

    constexpr std::size_t size = 100;
    constexpr std::size_t active = 33;

    std::array<wago::promise<int>, size> promises;
    std::list<wago::future<int>> futures;

    for (wago::promise<int>& p : promises)
        futures.push_back(p.get_future());

    ASSERT_TRUE(size == futures.size());

    std::thread th([&promises] { std::this_thread::sleep_for(10ms); promises[active].set_value(33); });

    auto first = wago::wait_for_any(std::begin(futures), std::end(futures));

    th.join();

    ASSERT_TRUE(first != std::begin(futures));
    ASSERT_TRUE(first != std::end(futures));
    ASSERT_TRUE(33 == first->get());
}


TEST(wstd_future, wait_for_any_for)
{
    using namespace std::chrono_literals;

    {
        constexpr std::size_t size = 100;
        constexpr std::size_t active = 33;

        std::array<wago::promise<int>, size> promises;
        std::array<wago::future<int>, size> futures;

        for (std::size_t i = 0; i < size; ++i)
            futures[i] = promises[i].get_future();

        std::thread th([&promises] { std::this_thread::sleep_for(10ms); promises[active].set_value(33); });

        auto first = wago::wait_for_any_for(std::begin(futures), std::end(futures), 50ms);

        th.join();

        ASSERT_TRUE(first != std::begin(futures));
        ASSERT_TRUE(first != std::end(futures));
        ASSERT_TRUE(33 == first->get());
    }
    {
        constexpr std::size_t size = 100;
        constexpr std::size_t active = 33;

        std::array<wago::promise<int>, size> promises;
        std::array<wago::future<int>, size> futures;

        for (std::size_t i = 0; i < size; ++i)
            futures[i] = promises[i].get_future();

        std::thread th([&promises] { std::this_thread::sleep_for(20ms); promises[active].set_value(33); });

        auto first = wago::wait_for_any_for(std::begin(futures), std::end(futures), 10ms);


        ASSERT_TRUE(first != std::begin(futures));
        ASSERT_TRUE(first == std::end(futures));

        first = wago::wait_for_any(std::begin(futures), std::end(futures));
        th.join();

        ASSERT_TRUE(first != std::begin(futures));
        ASSERT_TRUE(first != std::end(futures));
        ASSERT_TRUE(33 == first->get());
    }
}


TEST(wstd_future, wait_for_any_until)
{
    using namespace std::chrono_literals;

    {
        constexpr std::size_t size = 100;
        constexpr std::size_t active = 33;

        std::array<wago::promise<int>, size> promises;
        std::array<wago::future<int>, size> futures;

        for (std::size_t i = 0; i < size; ++i)
            futures[i] = promises[i].get_future();

        std::thread th([&promises] { std::this_thread::sleep_for(10ms); promises[active].set_value(33); });

        auto first = wago::wait_for_any_until(std::begin(futures), std::end(futures),
                                              std::chrono::system_clock::now() + 50ms);

        th.join();

        ASSERT_TRUE(first != std::begin(futures));
        ASSERT_TRUE(first != std::end(futures));
        ASSERT_TRUE(33 == first->get());
    }
    {
        constexpr std::size_t size = 100;
        constexpr std::size_t active = 33;

        std::array<wago::promise<int>, size> promises;
        std::array<wago::future<int>, size> futures;

        for (std::size_t i = 0; i < size; ++i)
            futures[i] = promises[i].get_future();

        std::thread th([&promises] { std::this_thread::sleep_for(20ms); promises[active].set_value(33); });

        auto first = wago::wait_for_any_until(std::begin(futures), std::end(futures),
                                              std::chrono::system_clock::now() + 10ms);


        ASSERT_TRUE(first != std::begin(futures));
        ASSERT_TRUE(first == std::end(futures));

        first = wago::wait_for_any(std::begin(futures), std::end(futures));
        th.join();

        ASSERT_TRUE(first != std::begin(futures));
        ASSERT_TRUE(first != std::end(futures));
        ASSERT_TRUE(33 == first->get());
    }
}


TEST(wstd_future, resolved_future_value)
{
    using namespace std::chrono_literals;

    {
        wago::future<int> f = wago::resolved_future(55);

        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_TRUE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        const int result = f.get();

        ASSERT_FALSE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
        ASSERT_TRUE(55 == result);
    }
    {
        const int result = wago::resolved_future(66).get();

        ASSERT_TRUE(66 == result);
    }
}


TEST(wstd_future, resolved_future_exception)
{
    using namespace std::chrono_literals;

    {
        wago::future<int> f = wago::resolved_future<int>(
                    std::make_exception_ptr(std::runtime_error("an exception")));

        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_TRUE(f.has_exception());

        const std::exception_ptr result = f.get_exception();

        ASSERT_FALSE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
        ASSERT_TRUE(nullptr != result);
    }
    {
        const std::exception_ptr result = wago::resolved_future<int>(
                    std::make_exception_ptr(std::runtime_error("an exception"))).get_exception();

        ASSERT_TRUE(nullptr != result);
    }
}


TEST(wstd_future, dismissal_notifier_promise_ctor)
{
    int value = 0;
    wago::promise<int> prom = wago::promise<int>::create_with_dismiss_notifier([&value]() { value = 1; });
    wago::future<int> fut(prom.get_future());

    ASSERT_EQ(0, value);

    fut.dismiss();

    ASSERT_EQ(1, value);

    fut.dismiss();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_future_set_notifier_double_future)
{
    int value = 0;


    wago::promise<int> promB = wago::promise<int>::create_with_dismiss_notifier([&value]() { ++value; });
    wago::future<int> futB(promB.get_future());


    wago::promise<int> promF = wago::promise<int>::create_with_dismiss_notifier([&futB]() { futB.dismiss(); });
    wago::future<int> futF(promF.get_future());


    ASSERT_EQ(0, value);

    futF.dismiss();

    ASSERT_EQ(1, value);

    futF.dismiss();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_future_set_notifier_double_future_2)
{
    using namespace std::chrono_literals;

    std::mutex prot;
    std::condition_variable guard;
    int value = 0;

    wago::promise<int> promB = wago::promise<int>::create_with_dismiss_notifier([&prot, &guard, &value]() {
            std::unique_lock<std::mutex> lock(prot);
            ++value;
            guard.notify_all();
        });
    wago::future<int> futB(promB.get_future());

    wago::promise<int> promF = wago::promise<int>::create_with_dismiss_notifier([&futB]() { futB.dismiss(); });
    wago::future<int> futF(promF.get_future());

    ASSERT_EQ(0, value);

    std::thread th([&futF] { std::this_thread::sleep_for(20ms); futF.dismiss(); });

    ASSERT_EQ(0, value);

    {
        std::unique_lock<std::mutex> lock(prot);

        guard.wait(lock, [&value] { return 0 < value; });
    }

    th.join();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, void_type_empty_future)
{
    wago::future<void> f;

	ASSERT_FALSE(f.valid()) << "Empty future must be invalid.";
	ASSERT_FALSE(f.ready()) << "Empty future must not be ready.";
	ASSERT_FALSE(f.dismissed()) << "Empty future must not be dismissed.";

	ASSERT_FALSE(f.has_value()) << "Empty future must not have value.";
	ASSERT_FALSE(f.has_exception()) << "Empty future must not have exception (value).";


    try {
        f.get();
        FAIL() << "Empty future should throw on 'get'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'get' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'get' - unknown exception thrown instead.";
    }


    try {
        f.get_exception();
        FAIL() << "Empty future should throw on 'get_exception'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'get_exception' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'get_exception'"\
                  " - unknown exception thrown instead.";
    }


    try {
        f.wait();
        FAIL() << "Empty future should throw on 'wait'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'wait' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'wait' - unknown exception thrown instead.";
    }


    try {
        (void)f.wait_for(std::chrono::seconds(3));
        FAIL() << "Empty future should throw on 'wait_for'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'wait_for' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'wait_for' - unknown exception thrown instead.";
    }


    try {
        (void)f.wait_until(std::chrono::system_clock::now() + std::chrono::seconds(3));
        FAIL() << "Empty future should throw on 'wait_until'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'wait_until' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'wait_until'"\
                  " - unknown exception thrown instead.";
    }


    try {
        f.dismiss();
    }
    catch (...) {
        FAIL() << "Empty future should allow dismissal without throwing exceptions.";
    }


    try {
        f.set_notifier([]() {});
        FAIL() << "Empty future should throw on 'set_notifier'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'set_notifier' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'set_notifier'"\
                  " - unknown exception thrown instead.";
    }


    try {
        f.set_exception_notifier([](std::exception_ptr) {});
        FAIL() << "Empty future should throw on 'set_exception_notifier'.";
    }
    catch (const std::future_error& fe) {
        ASSERT_EQ(std::future_errc::future_already_retrieved, fe.code()) <<
            "Empty future should throw std::future_error on 'set_exception_notifier' with "\
            "std::future_errc::future_already_retrieved error code.";
    }
    catch (...) {
        FAIL() << "Empty future should throw std::future_error on 'set_exception_notifier'"\
                  " - unknown exception thrown instead.";
    }
}


TEST(wstd_future, void_type_move)
{
    wago::promise<void> pA;
    wago::future<void> fA;

    try {
        wago::promise<void> p1;
        wago::future<void> f1(p1.get_future());
        wago::promise<void> p2(std::move(p1));
        wago::future<void> f2(std::move(f1));

        pA = std::move(p2);
        fA = std::move(f2);
    }
    catch (...) {
        FAIL() << "Moving future or promise failed (1).";
    }

    try {
        wago::promise<void> pB(std::move(pA));
        wago::future<void> fB(std::move(fA));

        pB = wago::promise<void>();
        fB = wago::future<void>();
    }
    catch (...) {
        FAIL() << "Moving future or promise failed (2).";
    }
}


TEST(wstd_future, void_type_basic_promise)
{
    {
        wago::promise<void> p;
        wago::future<void> f = p.get_future();

        ASSERT_TRUE(f.valid()) << "Freshly constructed promise shall return valide future.";
        ASSERT_TRUE(p.execute()) << "Freshly constructed promise shall be in 'execute' state.";
    }

    {
        wago::promise<void> p;

        try {
            p.set_value();
        }
        catch (...) {
            FAIL() << "Freshly constructed promise shall allow to set value.";
        }
    }

    {
        wago::promise<void> p;

        try {
            p.set_exception(std::make_exception_ptr(std::runtime_error("an exception")));
        }
        catch (...) {
            FAIL() << "Freshly constructed promise shall allow to set exception.";
        }
    }
}


TEST(wstd_future, void_type_single_future_get)
{
    using namespace std::chrono_literals;

    wago::promise<void> p;
    wago::future<void> f = p.get_future();

    std::thread th([&p] { std::this_thread::sleep_for(10ms); p.set_value(); });

    ASSERT_TRUE(f.valid());
    ASSERT_FALSE(f.ready());
    ASSERT_FALSE(f.has_value());
    ASSERT_FALSE(f.has_exception());

    f.get();

    th.join();

    ASSERT_FALSE(f.valid());
    ASSERT_FALSE(f.ready());
    ASSERT_FALSE(f.has_value());
    ASSERT_FALSE(f.has_exception());
}


// This test check if future can return a valid result if the source promise is already gone.
// This checks if there is a bug present with deallocated memory usage. Unfortunately this bug
// does not necessarily cause failure of the unit tests as the memory is phisically still available.
// Results are visible only under valgrind.
TEST(wstd_future, void_type_promise_gone_single_future_get)
{
    using namespace std::chrono_literals;

    wago::future<void> f;

    {
        wago::promise<void> p;
        f = p.get_future();

        std::thread th([&p] { std::this_thread::sleep_for(10ms); p.set_value(); });

        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        th.join();
    }

    f.get();

    ASSERT_FALSE(f.valid());
    ASSERT_FALSE(f.ready());
    ASSERT_FALSE(f.has_value());
    ASSERT_FALSE(f.has_exception());
}



// This test check if future can return a valid result if the source promise is already gone.
// This checks if there is a bug present with deallocated memory usage. Unfortunately this bug
// does not necessarily cause failure of the unit tests as the memory is phisically still available.
// Results are visible only under valgrind.
TEST(wstd_future, void_type_promise_gone_single_future_exception)
{
    using namespace std::chrono_literals;

    wago::future<void> f;

    {
        wago::promise<void> p;
        f = p.get_future();

        std::thread th([&p]
        {
            std::this_thread::sleep_for(10ms);
            p.set_exception(std::make_exception_ptr(std::runtime_error("an exception")));
        });

        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        th.join();
    }

    const std::exception_ptr result = f.get_exception();

    ASSERT_FALSE(f.valid());
    ASSERT_FALSE(f.ready());
    ASSERT_FALSE(f.has_value());
    ASSERT_FALSE(f.has_exception());
    ASSERT_TRUE(nullptr != result);
}


TEST(wstd_future, void_type_future_notifier)
{
    using namespace std::chrono_literals;

    {
        wago::promise<void> p;
        wago::future<void> f = p.get_future();
        int result = 0;

        f.set_notifier([&result]() { result = 3; });

        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        p.set_value();

        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
        ASSERT_TRUE(3 == result);
    }
    {
        wago::promise<void> p;
        wago::future<void> f = p.get_future();
        int result = 0;

        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        p.set_value();
        f.set_notifier([&result]() { result = 3; });

        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
        ASSERT_TRUE(3 == result);
    }
    {
        wago::promise<void> p;
        wago::future<void> f = p.get_future();
        std::mutex guard;
        std::condition_variable cv_guard;
        int result = 0;

        f.set_notifier([&result, &guard, &cv_guard]()
            {
                std::unique_lock<std::mutex> lock(guard);
                result = 3;
                cv_guard.notify_all();
            });

        std::thread th([&p] { std::this_thread::sleep_for(10ms); p.set_value(); });

        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        std::unique_lock<std::mutex> lock(guard);
        const bool success = cv_guard.wait_for(lock, 50ms, [&result] { return 3 == result; });

        th.join();

        ASSERT_TRUE(success);
        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
        ASSERT_TRUE(3 == result);
    }
}


TEST(wstd_future, void_type_future_wait)
{
    using namespace std::chrono_literals;

    wago::promise<void> p;
    wago::future<void> f = p.get_future();

    std::thread th([&p] { std::this_thread::sleep_for(10ms); p.set_value(); });
    
    ASSERT_TRUE(f.valid());
    ASSERT_FALSE(f.ready());
    ASSERT_FALSE(f.has_value());
    ASSERT_FALSE(f.has_exception());

    f.wait();
    th.join();

    ASSERT_TRUE(f.valid());
    ASSERT_TRUE(f.ready());
    ASSERT_TRUE(f.has_value());
    ASSERT_FALSE(f.has_exception());

    f.get();

    ASSERT_FALSE(f.valid());
    ASSERT_FALSE(f.ready());
    ASSERT_FALSE(f.has_value());
    ASSERT_FALSE(f.has_exception());
}


TEST(wstd_future, void_type_future_wait_for)
{
    using namespace std::chrono_literals;
    
    {
        wago::promise<void> p;
        wago::future<void> f = p.get_future();
        
        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        std::thread th([&p] { std::this_thread::sleep_for(10ms); p.set_value(); });
        std::future_status fs = f.wait_for(30ms);

        ASSERT_TRUE(std::future_status::ready == fs);
        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_TRUE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        f.get();
        th.join();

        ASSERT_FALSE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
    }
    {
        wago::promise<void> p;
        wago::future<void> f = p.get_future();
        
        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        std::thread th([&p] { std::this_thread::sleep_for(30ms); p.set_value(); });
        std::future_status fs = f.wait_for(10ms);

        ASSERT_TRUE(std::future_status::timeout == fs);
        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        f.get();
        th.join();

        ASSERT_FALSE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
    }
}


TEST(wstd_future, void_type_future_wait_until)
{
    using namespace std::chrono_literals;
    
    {
        wago::promise<void> p;
        wago::future<void> f = p.get_future();
        
        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        std::thread th([&p] { std::this_thread::sleep_for(10ms); p.set_value(); });
        std::future_status fs = f.wait_until(std::chrono::system_clock::now() + 30ms);

        ASSERT_TRUE(std::future_status::ready == fs);
        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_TRUE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        f.get();
        th.join();

        ASSERT_FALSE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
    }
    {
        wago::promise<void> p;
        wago::future<void> f = p.get_future();
        
        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        std::thread th([&p] { std::this_thread::sleep_for(30ms); p.set_value(); });
        std::future_status fs = f.wait_until(std::chrono::system_clock::now() + 10ms);

        ASSERT_TRUE(std::future_status::timeout == fs);
        ASSERT_TRUE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        f.get();
        th.join();

        ASSERT_FALSE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
    }
}


TEST(wstd_future, void_type_wait_for_any)
{
    using namespace std::chrono_literals;

    constexpr std::size_t size = 100;
    constexpr std::size_t active = 33;

    std::array<wago::promise<void>, size> promises;
    std::array<wago::future<void>, size> futures;

    for (std::size_t i = 0; i < size; ++i)
        futures[i] = promises[i].get_future();

    std::thread th([&promises] { std::this_thread::sleep_for(10ms); promises[active].set_value(); });

    auto first = wago::wait_for_any(std::begin(futures), std::end(futures));

    th.join();

    ASSERT_TRUE(first != std::begin(futures));
    ASSERT_TRUE(first != std::end(futures));
}


TEST(wstd_future, void_type_wait_for_any_vector)
{
    using namespace std::chrono_literals;

    constexpr std::size_t size = 100;
    constexpr std::size_t active = 33;

    std::array<wago::promise<void>, size> promises;
    std::vector<wago::future<void>> futures;

    for (wago::promise<void>& p : promises)
        futures.push_back(p.get_future());

    ASSERT_TRUE(size == futures.size());

    std::thread th([&promises] { std::this_thread::sleep_for(10ms); promises[active].set_value(); });

    auto first = wago::wait_for_any(std::begin(futures), std::end(futures));

    th.join();

    ASSERT_TRUE(first != std::begin(futures));
    ASSERT_TRUE(first != std::end(futures));
}


TEST(wstd_future, void_type_wait_for_any_list)
{
    using namespace std::chrono_literals;

    constexpr std::size_t size = 100;
    constexpr std::size_t active = 33;

    std::array<wago::promise<void>, size> promises;
    std::list<wago::future<void>> futures;

    for (wago::promise<void>& p : promises)
        futures.push_back(p.get_future());

    ASSERT_TRUE(size == futures.size());

    std::thread th([&promises] { std::this_thread::sleep_for(10ms); promises[active].set_value(); });

    auto first = wago::wait_for_any(std::begin(futures), std::end(futures));

    th.join();

    ASSERT_TRUE(first != std::begin(futures));
    ASSERT_TRUE(first != std::end(futures));
}


TEST(wstd_future, void_type_wait_for_any_for)
{
    using namespace std::chrono_literals;

    {
        constexpr std::size_t size = 100;
        constexpr std::size_t active = 33;

        std::array<wago::promise<void>, size> promises;
        std::array<wago::future<void>, size> futures;

        for (std::size_t i = 0; i < size; ++i)
            futures[i] = promises[i].get_future();

        std::thread th([&promises] { std::this_thread::sleep_for(10ms); promises[active].set_value(); });

        auto first = wago::wait_for_any_for(std::begin(futures), std::end(futures), 50ms);

        th.join();

        ASSERT_TRUE(first != std::begin(futures));
        ASSERT_TRUE(first != std::end(futures));
    }
    {
        constexpr std::size_t size = 100;
        constexpr std::size_t active = 33;

        std::array<wago::promise<void>, size> promises;
        std::array<wago::future<void>, size> futures;

        for (std::size_t i = 0; i < size; ++i)
            futures[i] = promises[i].get_future();

        std::thread th([&promises] { std::this_thread::sleep_for(20ms); promises[active].set_value(); });

        auto first = wago::wait_for_any_for(std::begin(futures), std::end(futures), 10ms);


        ASSERT_TRUE(first != std::begin(futures));
        ASSERT_TRUE(first == std::end(futures));

        first = wago::wait_for_any(std::begin(futures), std::end(futures));
        th.join();

        ASSERT_TRUE(first != std::begin(futures));
        ASSERT_TRUE(first != std::end(futures));
    }
}


TEST(wstd_future, void_type_wait_for_any_until)
{
    using namespace std::chrono_literals;

    {
        constexpr std::size_t size = 100;
        constexpr std::size_t active = 33;

        std::array<wago::promise<void>, size> promises;
        std::array<wago::future<void>, size> futures;

        for (std::size_t i = 0; i < size; ++i)
            futures[i] = promises[i].get_future();

        std::thread th([&promises] { std::this_thread::sleep_for(10ms); promises[active].set_value(); });

        auto first = wago::wait_for_any_until(std::begin(futures), std::end(futures),
                                              std::chrono::system_clock::now() + 50ms);

        th.join();

        ASSERT_TRUE(first != std::begin(futures));
        ASSERT_TRUE(first != std::end(futures));
    }
    {
        constexpr std::size_t size = 100;
        constexpr std::size_t active = 33;

        std::array<wago::promise<void>, size> promises;
        std::array<wago::future<void>, size> futures;

        for (std::size_t i = 0; i < size; ++i)
            futures[i] = promises[i].get_future();

        std::thread th([&promises] { std::this_thread::sleep_for(20ms); promises[active].set_value(); });

        auto first = wago::wait_for_any_until(std::begin(futures), std::end(futures),
                                              std::chrono::system_clock::now() + 10ms);


        ASSERT_TRUE(first != std::begin(futures));
        ASSERT_TRUE(first == std::end(futures));

        first = wago::wait_for_any(std::begin(futures), std::end(futures));
        th.join();

        ASSERT_TRUE(first != std::begin(futures));
        ASSERT_TRUE(first != std::end(futures));
    }
}


TEST(wstd_future, void_type_resolved_future_value)
{
    using namespace std::chrono_literals;

    {
        wago::future<void> f = wago::resolved_future();

        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_TRUE(f.has_value());
        ASSERT_FALSE(f.has_exception());

        f.get();

        ASSERT_FALSE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
    }
    {
        wago::resolved_future().get();
    }
}


TEST(wstd_future, void_type_resolved_future_exception)
{
    using namespace std::chrono_literals;

    {
        wago::future<void> f = wago::resolved_future<void>(
                    std::make_exception_ptr(std::runtime_error("an exception")));

        ASSERT_TRUE(f.valid());
        ASSERT_TRUE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_TRUE(f.has_exception());

        const std::exception_ptr result = f.get_exception();

        ASSERT_FALSE(f.valid());
        ASSERT_FALSE(f.ready());
        ASSERT_FALSE(f.has_value());
        ASSERT_FALSE(f.has_exception());
        ASSERT_TRUE(nullptr != result);
    }
    {
        const std::exception_ptr result = wago::resolved_future<void>(
                    std::make_exception_ptr(std::runtime_error("an exception"))).get_exception();

        ASSERT_TRUE(nullptr != result);
    }
}


TEST(wstd_future, void_type_dismissal_notifier_promise_ctor)
{
    int value = 0;
    wago::promise<void> prom = wago::promise<void>::create_with_dismiss_notifier([&value]() { value = 1; });
    wago::future<void> fut(prom.get_future());

    ASSERT_EQ(0, value);

    fut.dismiss();

    ASSERT_EQ(1, value);

    fut.dismiss();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, void_type_dismissal_notifier_future_set_notifier_double_future)
{
    int value = 0;


    wago::promise<void> promB = wago::promise<void>::create_with_dismiss_notifier([&value]() { ++value; });
    wago::future<void> futB(promB.get_future());


    wago::promise<void> promF = wago::promise<void>::create_with_dismiss_notifier([&futB]() { futB.dismiss(); });
    wago::future<void> futF(promF.get_future());


    ASSERT_EQ(0, value);

    futF.dismiss();

    ASSERT_EQ(1, value);

    futF.dismiss();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, void_type_dismissal_notifier_future_set_notifier_double_future_2)
{
    using namespace std::chrono_literals;

    std::mutex prot;
    std::condition_variable guard;
    int value = 0;

    wago::promise<void> promB = wago::promise<void>::create_with_dismiss_notifier([&prot, &guard, &value]() {
            std::unique_lock<std::mutex> lock(prot);
            ++value;
            guard.notify_all();
        });
    wago::future<void> futB(promB.get_future());

    wago::promise<void> promF = wago::promise<void>::create_with_dismiss_notifier([&futB]() { futB.dismiss(); });
    wago::future<void> futF(promF.get_future());

    ASSERT_EQ(0, value);

    std::thread th([&futF] { std::this_thread::sleep_for(20ms); futF.dismiss(); });

    ASSERT_EQ(0, value);

    {
        std::unique_lock<std::mutex> lock(prot);

        guard.wait(lock, [&value] { return 0 < value; });
    }

    th.join();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_setter)
{
    wago::promise<int> prom;
    wago::future<int> fut(prom.get_future());

    int value = 0;

    prom.set_dismiss_notifier([&value] { value = 1; });

    ASSERT_EQ(0, value);

    fut.dismiss();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_setter_2)
{
    wago::promise<int> prom;
    wago::future<int> fut(prom.get_future());

    int value = 0;

    fut.dismiss();
    prom.set_dismiss_notifier([&value] { value = 1; });

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_setter_void)
{
    wago::promise<void> prom;
    wago::future<void> fut(prom.get_future());

    int value = 0;

    prom.set_dismiss_notifier([&value] { value = 1; });

    ASSERT_EQ(0, value);

    fut.dismiss();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_setter_void_2)
{
    wago::promise<void> prom;
    wago::future<void> fut(prom.get_future());

    int value = 0;

    fut.dismiss();
    prom.set_dismiss_notifier([&value] { value = 1; });

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_setter_thread)
{
    using namespace std::chrono_literals;

    wago::promise<int> p;
    wago::future<int> f = p.get_future();

    std::thread th([&f] { std::this_thread::sleep_for(10ms); f.dismiss(); });

    int value = 0;

    p.set_dismiss_notifier([&value] { value = 1; });

    ASSERT_EQ(0, value);

    th.join();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_setter_thread_2)
{
    using namespace std::chrono_literals;

    wago::promise<int> p;
    int value = 0;

    p.set_dismiss_notifier([&value] { value = 1; });

    wago::future<int> f = p.get_future();

    std::thread th([&f] { std::this_thread::sleep_for(10ms); f.dismiss(); });

    ASSERT_EQ(0, value);

    th.join();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_setter_thread_void)
{
    using namespace std::chrono_literals;

    wago::promise<void> p;
    wago::future<void> f = p.get_future();

    std::thread th([&f] { std::this_thread::sleep_for(10ms); f.dismiss(); });

    int value = 0;

    p.set_dismiss_notifier([&value] { value = 1; });

    ASSERT_EQ(0, value);

    th.join();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_setter_thread_void_2)
{
    using namespace std::chrono_literals;

    wago::promise<void> p;
    int value = 0;

    p.set_dismiss_notifier([&value] { value = 1; });

    wago::future<void> f = p.get_future();

    std::thread th([&f] { std::this_thread::sleep_for(10ms); f.dismiss(); });

    ASSERT_EQ(0, value);

    th.join();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_setter_double_dismiss)
{
    wago::promise<int> prom;
    wago::future<int> fut(prom.get_future());

    int value = 0;

    prom.set_dismiss_notifier([&value] { ++value; });

    ASSERT_EQ(0, value);

    fut.dismiss();
    fut.dismiss();
    fut.dismiss();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_setter_double_notifer_set)
{
    wago::promise<int> prom;
    wago::future<int> fut(prom.get_future());

    int value = 0;

    prom.set_dismiss_notifier([&value] { value = 5; });
    prom.set_dismiss_notifier([&value] { value = 6; });
    prom.set_dismiss_notifier([&value] { ++value; });

    ASSERT_EQ(0, value);

    fut.dismiss();
    fut.dismiss();
    fut.dismiss();

    ASSERT_EQ(1, value);

    prom.set_dismiss_notifier([&value] { value = 2; });

    ASSERT_EQ(2, value);

    prom.set_dismiss_notifier([&value] { ++value; });

    ASSERT_EQ(3, value);

    fut.dismiss();
    fut.dismiss();

    ASSERT_EQ(3, value);
}


TEST(wstd_future, dismissal_notifier_setter_double_dismiss_void)
{
    wago::promise<void> prom;
    wago::future<void> fut(prom.get_future());

    int value = 0;

    prom.set_dismiss_notifier([&value] { ++value; });

    ASSERT_EQ(0, value);

    fut.dismiss();
    fut.dismiss();
    fut.dismiss();

    ASSERT_EQ(1, value);
}


TEST(wstd_future, dismissal_notifier_setter_double_notifer_set_void)
{
    wago::promise<void> prom;
    wago::future<void> fut(prom.get_future());

    int value = 0;

    prom.set_dismiss_notifier([&value] { value = 5; });
    prom.set_dismiss_notifier([&value] { value = 6; });
    prom.set_dismiss_notifier([&value] { ++value; });

    ASSERT_EQ(0, value);

    fut.dismiss();
    fut.dismiss();
    fut.dismiss();

    ASSERT_EQ(1, value);

    prom.set_dismiss_notifier([&value] { value = 2; });

    ASSERT_EQ(2, value);

    prom.set_dismiss_notifier([&value] { ++value; });

    ASSERT_EQ(3, value);

    fut.dismiss();
    fut.dismiss();

    ASSERT_EQ(3, value);
}


TEST(wstd_future, dismiss_remove_before_dismiss)
{
    using namespace std::chrono_literals;

    std::atomic_bool kill_switch(true);
    std::thread th([&kill_switch] {
            std::this_thread::sleep_for(10ms);
            if (kill_switch)
            {
                std::thread kill_me([] { std::this_thread::sleep_for(10ms); exit(-1); });
                FAIL() << "Deletion of dismissied promise resulted in a deadlock. The test will be terminated.";
            }
        });

    std::unique_ptr<wago::promise<int>> prom = std::make_unique<wago::promise<int>>();
    wago::future<int> fut(prom->get_future());

    prom->set_dismiss_notifier([&prom] { prom.reset(); });

    fut.dismiss();

    kill_switch = false;
    th.join();
}


TEST(wstd_future, dismiss_remove_after_dismiss)
{
    using namespace std::chrono_literals;

    std::atomic_bool kill_switch(true);
    std::thread th([&kill_switch] {
            std::this_thread::sleep_for(10ms);
            if (kill_switch)
            {
                std::thread kill_me([] { std::this_thread::sleep_for(10ms); exit(-1); });
                FAIL() << "Deletion of dismissied promise resulted in a deadlock. The test will be terminated.";
            }
        });

    std::unique_ptr<wago::promise<int>> prom = std::make_unique<wago::promise<int>>();
    wago::future<int> fut(prom->get_future());

    fut.dismiss();

    prom->set_dismiss_notifier([&prom] { prom.reset(); });

    kill_switch = false;
    th.join();
}


TEST(wstd_future, dismiss_remove_before_dismiss_void)
{
    using namespace std::chrono_literals;

    std::atomic_bool kill_switch(true);
    std::thread th([&kill_switch] {
            std::this_thread::sleep_for(10ms);
            if (kill_switch)
            {
                std::thread kill_me([] { std::this_thread::sleep_for(10ms); exit(-1); });
                FAIL() << "Deletion of dismissied promise resulted in a deadlock. The test will be terminated.";
            }
        });

    std::unique_ptr<wago::promise<void>> prom = std::make_unique<wago::promise<void>>();
    wago::future<void> fut(prom->get_future());

    prom->set_dismiss_notifier([&prom] { prom.reset(); });

    fut.dismiss();

    kill_switch = false;
    th.join();
}


TEST(wstd_future, dismiss_remove_after_dismiss_void)
{
    using namespace std::chrono_literals;

    std::atomic_bool kill_switch(true);
    std::thread th([&kill_switch] {
            std::this_thread::sleep_for(10ms);
            if (kill_switch)
            {
                std::thread kill_me([] { std::this_thread::sleep_for(10ms); exit(-1); });
                FAIL() << "Deletion of dismissied promise resulted in a deadlock. The test will be terminated.";
            }
        });

    std::unique_ptr<wago::promise<void>> prom = std::make_unique<wago::promise<void>>();
    wago::future<void> fut(prom->get_future());

    fut.dismiss();

    prom->set_dismiss_notifier([&prom] { prom.reset(); });

    kill_switch = false;
    th.join();
}
