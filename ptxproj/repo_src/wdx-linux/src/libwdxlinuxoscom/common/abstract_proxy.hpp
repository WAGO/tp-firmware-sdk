//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Abstract proxy class template intended to represent an interface
///            on the other side of an IPC connection 
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_ABSTRACT_PROXY_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_ABSTRACT_PROXY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/driver_i.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"
#include "coder.hpp"
#include "managed_object.hpp"
#include "method_id.hpp"
#include "sender_i.hpp"
#include "ipc_status.hpp"
#include "definitions.hpp"
#include "type_helpers.hpp"
#include "debug_log.hpp"

#include <wc/trace.h>
#include <wc/structuring.h>
#include <wc/log.h>
#include <wago/future.hpp>

#include <memory>
#include <map>
#include <functional>
#include <type_traits>
#include <sstream>
#include <mutex>
#include <cstdint>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

/// An abstract proxy template class implements an interface by translating calls
/// into IPC messages.
///
/// For an interface which shall be used via an IPC connection there should be
/// a derived class of this abstract base class. The only porpuse of the derived 
/// class is then to provide an implementation for each interface method by 
/// redirecting those method calls to the protected "call" method defined by the 
/// class template.
template<class Interface>
class abstract_proxy : public Interface, public managed_object {
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(abstract_proxy)

public:
    using interface = Interface;

private:
    using open_call = promise<data_istream*>;

    std::atomic<uint32_t>         next_call_id_m;
    std::map<uint32_t, open_call> open_calls_m;
    std::mutex                    open_calls_mutex_m;

public:
    /// Create a proxy which uses the given sender and driver interfaces
    /// \param id       The id shared with the stub which lives on the other side of an IPC connection.
    /// \param sender   Sender interface to send data.
    /// \param driver   Driver interface to trigger data receive in case of synchronous calls.
    abstract_proxy(managed_object_id        id,
                   std::string       const &connection_name,
                   sender_i                &sender,
                   driver_i                &driver);

    ~abstract_proxy() noexcept override;

    void handle_message(data_istream &received) override;

protected:

    /// Use this method to make an exception protected (only wdx::linuxos::com::exception is thrown) remote call.
    /// See description of "call_unsafe" for further details.
    template<typename ReturnType,
             typename ... ArgumentTypes>
    ReturnType call(method_id_type method_id, ArgumentTypes &... args);

    /// Use this method to make a remote call. The other side of an IPC connection
    /// is expected to handle the call and send a response, which is then received,
    /// decoded and provided as the return value here.
    /// \param method_id The method id which corresponds to the original interface
    ///                  method which has been called
    /// \param args      The arguments as provided to the original method
    /// \return The decoded return value. If the type is a future, the actual result 
    ///         may be pending until received asynchronously.
    template<typename ReturnType,
             typename ... ArgumentTypes>
    ReturnType call_unsafe(method_id_type method_id, ArgumentTypes &... args);

private:

    void dismiss_call(uint32_t call_id);

    template<typename ReturnType,
             std::enable_if_t<utils::is_instance<ReturnType, future>::value, int> = 0>
    std::shared_ptr<promise<typename ReturnType::value_type>> prepare_receive(method_id_type method_id, uint32_t const call_id)
    {
        using value_type = typename ReturnType::value_type;
        return prepare_receive<value_type>(method_id, call_id);
    }

    template<typename ReturnType,
             std::enable_if_t<!utils::is_instance<ReturnType, future>::value, int> = 0>
    std::shared_ptr<promise<ReturnType>> prepare_receive(method_id_type method_id, uint32_t const call_id)
    {
        auto value_promise = std::make_shared<promise<ReturnType>>();
        // store the promise to receive an answer and what to do with it
        wait_for_response(call_id, [value_promise, cnx_name = get_connection_name(), obj_id = get_id(), method_id, call_id](data_istream * is) {
            if (value_promise->execute())
            {
                try
                {
                    WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com proxy: decode message");
                    ipc_status status;
                    coder::decode(*is, status);
                    if (status != ipc_status::success)
                    {
                        std::string error_message;
                        coder::decode(*is, error_message);
                        throw wdx::linuxos::com::remote_exception(error_message);
                    }

                    std::unique_ptr<ReturnType> value;
                    coder::decode(*is, value);
                    WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com proxy: message decoding done, async call done, set value");
                    debug_log_interface_call<Interface>("proxy response", cnx_name, obj_id, call_id, method_id);
                    value_promise->set_value(std::move(*value));
                }
                catch(std::exception const &e)
                {
                    debug_log_interface_call<Interface>("proxy response failed", cnx_name, obj_id, call_id, method_id);
                    WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com proxy: async call failed (exception caught), set exception");
                    value_promise->set_exception(std::make_exception_ptr(e));
                }
            }
            else
            {
                debug_log_interface_call<Interface>("proxy response dismissed", cnx_name, obj_id, call_id, method_id);
                WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com proxy: async call dismissed, cancel");
            }
        },
        [value_promise, cnx_name = get_connection_name(), obj_id = get_id(), method_id, call_id](std::exception_ptr ex) {
            debug_log_interface_call<Interface>("proxy response failed", cnx_name, obj_id, call_id, method_id);
            WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com proxy: async call failed, set exception");
            value_promise->set_exception(ex);
        });
        return value_promise;
    }

    /// A helper method to receive a typed response which can be either a future
    /// or a value.
    /// First: Return type is a future.
    template<typename ReturnType,
             std::enable_if_t<utils::is_instance<ReturnType, future>::value, int> = 0>
    ReturnType receive_and_decode(ReturnType result_future, method_id_type method_id)
    {
        WC_UNUSED_DATA(method_id);
        WC_DEBUG_LOG(("[" + get_connection_name() + " Proxy " + std::to_string(get_id()) + "] " + WC_ARRAY_TO_PTR(__func__) + "(" + std::to_string(method_id) + ") {interface=" + typeid(Interface).name() + "}").c_str());
        return result_future;
    }

    /// Second: Return type is a value.
    template<typename ReturnType,
             std::enable_if_t<!utils::is_instance<ReturnType, future>::value, int> = 0>
    ReturnType receive_and_decode(future<ReturnType> result_future, method_id_type method_id)
    {
        // TODO: Where do timeout and attempts come from?
        //       Potentially we wont need this loop here and may
        //       just provide the timeout to the managers receive_next
        //       method call.
        size_t   attempts = 300;
        uint32_t timeout  = 100;
        // receive until our future resolved
        wc_log(log_level_t::debug, "Start loop for sync proxy call");
        while(!result_future.ready() && attempts > 0) 
        {
            // manager MIGHT work on our future or any other...
            // should the manager work an everything he
            // received so far instead?
            if (!get_driver().run_once(timeout))
            {
                attempts--; // skip this attempt if some other future has been
                            // fulfilled instead of ours.
            }
        }
        wc_log(log_level_t::debug, "Finished loop for sync proxy call");

        if(!result_future.ready())
        {
            // TODO: May we throw an "synchronous_call_timeout" exception?
            throw wdx::linuxos::com::exception("timeout while waiting on response");
        }
        WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com proxy: sync call done");
        WC_UNUSED_DATA(method_id);
        WC_DEBUG_LOG(("[" + get_connection_name() + " Proxy " + std::to_string(get_id()) + "] " + WC_ARRAY_TO_PTR(__func__) + "(" + std::to_string(method_id) + ") {interface=" + typeid(Interface).name() + "}").c_str());
        return result_future.get();
    }

    /// a helper function to enqueue an open call for a call which returned a future to be 
    /// resolved later. 
    void wait_for_response(uint32_t                                  call_id,
                           std::function<void(data_istream *)>     &&handler,
                           std::function<void(std::exception_ptr)> &&exception_handler);

    /// Utility to generate a call ID for calls to this proxy
    /// The call Id will be required to map a response to the correct
    /// future/promise object.
    inline uint32_t get_next_call_id()
    {
        return next_call_id_m++;
    }
};

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
template<class I>
abstract_proxy<I>::abstract_proxy(managed_object_id         id,
                                  std::string        const &connection_name,
                                  sender_i                 &sender,
                                  driver_i                 &driver)
: managed_object(id, connection_name, sender, driver)
, next_call_id_m(0)
{}

template<class I>
abstract_proxy<I>::~abstract_proxy() noexcept
{
    wc_log(log_level_t::debug, (string("Destroyed proxy with ") + std::to_string(open_calls_m.size()) + " open calls").c_str()); // parasoft-suppress CERT_CPP-DCL57-a CERT_CPP-ERR50-b "bad_alloc exception is not thrown on Linux"

    try
    {
        // Reject all pending promises (i.e. responses we're still waiting for)
        std::lock_guard<std::mutex> open_calls_lock(open_calls_mutex_m);
        for (auto &open : open_calls_m)
        {
            char const message[] = "IPC object has been cleaned up before the response has been received.";
            open.second.set_exception(std::make_exception_ptr(wdx::linuxos::com::exception(WC_ARRAY_TO_PTR(message))));
        }
    }
    catch(...)
    {
        bool const destructor_should_not_throw = true;
        WC_ASSERT(!destructor_should_not_throw);
    }
}

template<class I>
void abstract_proxy<I>::handle_message(data_istream &received)
{
    WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com proxy: handle incoming message");
    uint32_t call_id;
    coder::decode(received, call_id);
    WC_DEBUG_LOG(("[" + get_connection_name() + " Proxy " + std::to_string(get_id()) + "] " + WC_ARRAY_TO_PTR(__func__) + "{callid=" + std::to_string(call_id) + " interface=" + typeid(I).name() + "}").c_str());
    {
        try
        {
            open_call open;
            {
                std::lock_guard<std::mutex> open_calls_lock(open_calls_mutex_m);
                open = std::move(open_calls_m.at(call_id));
                open_calls_m.erase(call_id);
            }
            debug_log_call_count<I>("Open call erased (responded)", get_connection_name(), get_id(), call_id, open_calls_m.size());
            if (open.execute()) // False when future is dismissed
            {
                open.set_value(&received);
            }
        }
        catch (std::out_of_range const &e)
        {
            // The message is not of interest for us as we don't expected a response for this call id.
            // Normally this may only happen in case a future is dismissed (no answer expected, but possible).
            wc_log(log_level_t::info, (std::string(SERVICE_LOG_PREFIX "Failed to find open call ID " + std::to_string(call_id) + " (maybe dismissed): ") + e.what()).c_str());
        }
        catch (std::exception const &e)
        {
            wc_log(log_level_t::error, (std::string(SERVICE_LOG_PREFIX "An unexpected error occurred: ") + e.what()).c_str());
            WC_FAIL("An unexpected error occurred");
        }
    }
}

template<class I>
template<typename ReturnType,
         typename ... ArgumentTypes>
ReturnType abstract_proxy<I>::call(method_id_type method_id, ArgumentTypes &... args)
{
    std::string error_message = "Proxy call exception: ";
    try
    {
        return call_unsafe<ReturnType>(method_id, args...);
    }
    catch(wdx::linuxos::com::exception &ce)
    {
        error_message += ce.what();
    }
    catch(std::exception &e)
    {
        error_message += e.what();
        WC_FAIL(error_message.c_str());
    }
    catch(...)
    {
        error_message += "Exception without exception type!";
        WC_FAIL(error_message.c_str());
    }

    wc_log(log_level_t::error, (SERVICE_LOG_PREFIX + error_message).c_str());
    throw wdx::linuxos::com::exception(error_message);
}

template<class I>
template<typename ReturnType,
         typename ... ArgumentTypes>
ReturnType abstract_proxy<I>::call_unsafe(method_id_type method_id, ArgumentTypes &... args)
{
    WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com proxy: handle call, encode message");
    uint32_t call_id = get_next_call_id();

    WC_DEBUG_LOG(("[" + get_connection_name() + " Proxy " + std::to_string(get_id()) + "] " + WC_ARRAY_TO_PTR(__func__) + "(" + std::to_string(method_id) + ") {callid=" + std::to_string(call_id) + " interface=" + typeid(I).name() + "}").c_str());
    debug_log_interface_call<I>("proxy request", get_connection_name(), get_id(), call_id, method_id, args...);

    // encode and send
    std::vector<uint8_t> message;
    data_stream message_stream(message);
    coder::encode(message_stream, method_id, call_id, args...);
    auto promise = prepare_receive<ReturnType>(method_id, call_id);
    WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com proxy: encoding done, send request message");
    WC_ASSERT(message.size() <= sender_i::max_send_data);
    get_sender().send(*this, std::move(message), [this, promise, call_id](std::string error_message){
        if(!error_message.empty())
        {
            open_call call_promise;
            std::string error = SERVICE_LOG_PREFIX "Proxy failed to send request message: " + error_message;
            wc_log(log_level_t::error, error.c_str());
            {
                std::lock_guard<std::mutex> open_calls_lock(open_calls_mutex_m);
                call_promise = std::move(open_calls_m.at(call_id));
                open_calls_m.erase(call_id);
            }
            debug_log_call_count<I>("Open call erased (error)", get_connection_name(), get_id(), call_id, open_calls_m.size());
            call_promise.set_exception(std::make_exception_ptr(wdx::linuxos::com::exception(error)));
        }
        WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com proxy: message sent");
    });
    promise->set_dismiss_notifier([this, call_id](){
        dismiss_call(call_id);
    });
    // receive and decode using a overloaded helper function
    // to handle futures and non-futures differently
    return receive_and_decode<ReturnType>(std::move(promise->get_future()), method_id);
}

template<class I>
void abstract_proxy<I>::wait_for_response(uint32_t                                  call_id,
                                          std::function<void(data_istream *)>     &&handler,
                                          std::function<void(std::exception_ptr)> &&exception_handler)
{
    std::lock_guard<std::mutex> open_calls_lock(open_calls_mutex_m);
    // TODO: What happens to open calls, never receive an answer? Are this one destroyed when the connection is closed?
    open_calls_m.emplace(
        call_id,
        open_call(std::move(handler), std::move(exception_handler))
    );
    debug_log_call_count<I>("Open call added", get_connection_name(), get_id(), call_id, open_calls_m.size());
}

template <class I>
void abstract_proxy<I>::dismiss_call(uint32_t call_id)
{
    WC_DEBUG_LOG(("[" + get_connection_name() + " Proxy " + std::to_string(get_id()) + "] dismiss for call id " + std::to_string(call_id) + " {interface=" + typeid(I).name() + "}").c_str());
    // Avoid any work caused by processing an answer
    {
        std::lock_guard<std::mutex> open_calls_lock(open_calls_mutex_m);
        open_calls_m.at(call_id).get_future().dismiss();
        open_calls_m.erase(call_id);
    }
    debug_log_call_count<I>("Open call erased (dismiss)", get_connection_name(), get_id(), call_id, open_calls_m.size());

    // Try to dismiss remote by sending a dismiss message
    std::vector<uint8_t> dismiss_message;
    data_stream dismiss_message_stream(dismiss_message);
    coder::encode(dismiss_message_stream, dismiss_call_id, call_id);
    WC_ASSERT(dismiss_message.size() <= sender_i::max_send_data);
    get_sender().send(*this, std::move(dismiss_message), [](std::string error_message){
        if(!error_message.empty())
        {
            wc_log(log_level_t::error, (SERVICE_LOG_PREFIX "Proxy failed to send dismiss message: " + error_message).c_str());
        }
    });

    wc_log(log_level_t::info, (SERVICE_LOG_PREFIX "Dismissed open call ID " + std::to_string(call_id) + " from list of expected answers").c_str());
}

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_ABSTRACT_PROXY_HPP_
//---- End of source file ------------------------------------------------------
