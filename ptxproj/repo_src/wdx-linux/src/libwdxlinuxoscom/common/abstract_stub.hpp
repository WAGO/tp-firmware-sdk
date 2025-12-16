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
///  \brief    A class template to be used to implement stubs for interfaces 
///            easily
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_ABSTRACT_STUB_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_ABSTRACT_STUB_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/exception.hpp"
#include "managed_object.hpp"
#include "coder.hpp"
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

#include <sstream>
#include <type_traits>
#include <exception>
#include <memory>
#include <mutex>
#include <cstdint>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class driver_i;

/// An abstract stub provides the main IPC I/O implementation for the redirection
/// of generic IPC messages to the actual object which is implementing the interface. 
/// 
/// If a stub implementation is required for an interface, you should derive from 
/// the template class for that interface and only implement the handle_call method 
/// by yourself, which handles the redirecion of incoming calls.
template<class Interface>
class abstract_stub : public managed_object {
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(abstract_stub)

public:
    using interface = Interface;

protected:
    interface              &target_m;
    wc_trace_channels const  channel_m;

private:
    struct base_container
    {
        WC_INTERFACE_CLASS(base_container)

    public:
        virtual void dismiss() = 0;
        virtual bool is_dismissed() = 0;
    };

    template<class Type>
    struct specific_container : base_container
    {
        WC_INTERFACE_IMPL_CLASS(specific_container)

    public:
        Type value;

        specific_container(Type &&value_) // Conversion constructor: NOLINT(google-explicit-constructor)
        : value(std::move(value_))
        { }

        void dismiss() override
        {
            value.dismiss();
        }

        bool is_dismissed() override
        {
            return value.dismissed();
        }

        Type & get_future()
        {
            return value;
        }
    };

    std::map<uint32_t, std::shared_ptr<base_container>> open_target_calls_m;
    std::mutex                                          open_target_calls_mutex_m;

public:
    /// Create a Stub for the given interface
    /// \param id       The id shared with the proxy which lives on the other side of an IPC connection.
    /// \param sender   Sender interface to send data.
    /// \param receiver Receiver interface to trigger data receive in case of synchronous calls.
    /// \param target   An object which fulfills the to be stubbed interface.
    ///                 Calls will be redirected to this interface.
    /// \param channel  Trace channel to start/stop for incoming IPC calls.
    abstract_stub(managed_object_id        id,
                  std::string       const &connection_name,
                  sender_i                &sender,
                  driver_i                &driver,
                  interface               &target,
                  wc_trace_channels  const  channel = wc_trace_channels::invalid);

    ~abstract_stub() noexcept override = default;

    void handle_message(data_istream &message) override;

    interface &target() const
    {
        return target_m;
    }

protected:
    template<typename ReturnType,
             typename ... ArgumentTypes>
    using interface_method = ReturnType (interface::*)(ArgumentTypes...);

    /// Call to call a method of the stubbed interface on the target object hold by 
    /// this stub. The result of that call will be transmitted back then.
    /// \param method       The pointer-to-member to the interface member method
    /// \param method_id    Method ID (interface member method) for this call
    /// \param method_args  A stream holding the still encoded method arguments
    template<typename ReturnType,
             typename ... ArgumentTypes>
    void call(interface_method<ReturnType, ArgumentTypes...>  method,
              method_id_type                                  method_id,
              data_istream                                   &method_args);

private:
    /// helper method to send a response wither directly or by awaiting a future first


    /// Dispatches calls to the dedicated interface method. This method is to be 
    /// implemented by a derived class.
    virtual void handle_call(method_id_type  method_id, 
                             data_istream   &method_in_args) = 0;

    void handle_dismiss_call(data_istream &message);

    /// Returns true, if a call has been cleaned up
    bool clean_open_call(uint32_t call_id)
    {
        std::lock_guard<std::mutex> lock(open_target_calls_mutex_m);
        bool exists = open_target_calls_m.count(call_id) > 0;
        if (exists)
        {
            // remove future from list
            WC_ASSERT(open_target_calls_m.at(call_id)->is_dismissed() == false);
            open_target_calls_m.erase(call_id);
        }
        return exists;
    }

    /// Encode and send a value provided by a future.
    template<typename T,
         std::enable_if_t<utils::is_instance<T, future>::value, int> = 0>
    void encode_and_send_return_value(method_id_type method_id, uint32_t call_id, ipc_status status, T &&value)
    {
        // ipc errors are only handled by T != wago::future
        WC_ASSERT(status == ipc_status::success);

        // add future to list
        auto open_call = std::make_shared<specific_container<T>>(std::move(value));
        {
            std::lock_guard<std::mutex> lock(open_target_calls_mutex_m);
            open_target_calls_m.emplace(call_id, open_call);
        }
        auto &open_future = open_call->get_future();
        open_future.set_notifier([this, method_id, call_id, status](auto &&actual_value) {
            if (clean_open_call(call_id))
            {
                // value can now be sent directly
                encode_and_send_return_value(method_id, call_id, status, std::move(actual_value));
            }
        });
        open_future.set_exception_notifier([this, method_id, call_id] (std::exception_ptr e_ptr) {
            try 
            {
                if(e_ptr)
                {
                    std::rethrow_exception(e_ptr);
                }
            }
            catch (std::exception const &e)
            {
                std::string error_message = std::string(SERVICE_LOG_PREFIX "Exception notifier called: ") + e.what();
                wc_log(log_level_t::error, error_message.c_str());
                
                if (clean_open_call(call_id)) {
                    // Handle a throwing provider or IPC implementation (unexpected!)
                    encode_and_send_return_value(method_id, call_id, ipc_status::unexpected_exception, std::move(error_message));
                }
            }  
        });

    }

    /// Encode and send a value directly.
    template<typename T,
            std::enable_if_t<!utils::is_instance<T, future>::value, int> = 0>
    void encode_and_send_return_value(method_id_type method_id, uint32_t call_id, ipc_status status, T &&value)
    {
        WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com stub: got interface result, encode return message");
        std::vector<uint8_t> message;
        data_stream message_stream(message);
        coder::encode(message_stream, call_id, status, value);
        WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com stub: encoding done, send return message");
        if(channel_m != wc_trace_channels::invalid) { wc_trace_stop_channel(channel_m); }
        WC_DEBUG_LOG(("[" + get_connection_name() + " Stub " + std::to_string(get_id()) + "] " + WC_ARRAY_TO_PTR(__func__) + "() {callid=" + std::to_string(call_id) + " interface=" + typeid(Interface).name() + "}").c_str());
        debug_log_interface_call<Interface>("stub response", get_connection_name(), get_id(), call_id, method_id);
        WC_ASSERT(message.size() <= sender_i::max_send_data);
        get_sender().send(*this, std::move(message), [this, call_id](std::string error_message){
            if(!error_message.empty())
            {
                std::string error = SERVICE_LOG_PREFIX "Stub failed to send return message: " + error_message;
                wc_log(log_level_t::error, error.c_str());
                // will bubble up to driver_i::run/run_once call.
                throw wdx::linuxos::com::exception(error);
            }
            WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com stub: return message sent");
            WC_DEBUG_LOG(("[" + get_connection_name() + " Stub " + std::to_string(get_id()) + "] " + "send_handler() {callid=" + std::to_string(call_id) + " interface=" + typeid(Interface).name() + "}").c_str());
        });
    }
};

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

template<class I>
abstract_stub<I>::abstract_stub(managed_object_id                  id,
                                std::string                 const &connection_name,
                                sender_i                          &sender,
                                driver_i                          &driver,
                                abstract_stub<I>::interface       &target,
                                wc_trace_channels            const  channel)
: managed_object(id, connection_name, sender, driver)
, target_m(target)
, channel_m(channel)
{}

template <class I>
void abstract_stub<I>::handle_message(data_istream &message)
{
    WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com stub: handle incoming message");
    method_id_type method_id;
    coder::decode(message, method_id);
    WC_DEBUG_LOG(("[" + get_connection_name() + " Stub " + std::to_string(get_id()) + "] " + WC_ARRAY_TO_PTR(__func__) + "(" + std::to_string(method_id) + ") {interface=" + typeid(I).name() + "}").c_str());
    if (method_id == dismiss_call_id)
    {
        if(channel_m != wc_trace_channels::invalid)  { wc_trace_stop_channel(channel_m); }
        handle_dismiss_call(message);
    }
    else
    {
        if(channel_m != wc_trace_channels::invalid) { wc_trace_start_channel(channel_m); }
        handle_call(method_id, message);
    }
}

template <class I>
template <typename ReturnType,
          typename ... ArgumentTypes>
void abstract_stub<I>::call(interface_method<ReturnType, ArgumentTypes...>  method,
                            method_id_type                                  method_id,
                            data_istream                                   &method_args)
{
    uint32_t call_id;
    coder::decode(method_args, call_id);

    // decode arguments
    try
    {
        WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com stub: decode arguments");
        std::tuple<ArgumentTypes...> set_of_args;
        utils::call_helper([&method_args](ArgumentTypes & ... args){
            coder::decode(method_args, args...);
        }, set_of_args);
        WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com stub: decoding done");

        WC_DEBUG_LOG(("[" + get_connection_name() + " Stub " + std::to_string(get_id()) + "] " + WC_ARRAY_TO_PTR(__func__) + "() {callid=" + std::to_string(call_id) + " interface=" + typeid(I).name() + "}").c_str());
        debug_log_interface_call<I>("stub request", get_connection_name(), get_id(), call_id, method_id);

        // call the interface method with the decoded arguments
        WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com stub: call interface");
        ReturnType ret_value = utils::call_helper([this, &method](ArgumentTypes & ... args){
            return (target_m.*method)(args...);
        }, set_of_args);

        // either encode the return value directly or wait for a future to resolve
        // before doing so.
        encode_and_send_return_value(method_id, call_id, ipc_status::success, std::move(ret_value));
    }
    catch(std::exception &e)
    {
        std::string error_message = std::string(SERVICE_LOG_PREFIX "Exception caught in handle call: ") + e.what();
        wc_log(log_level_t::error, error_message.c_str());
        WC_FAIL(error_message.c_str());

        // Handle a throwing interface implementation (e.g. provider) or IPC implementation (unexpected!)
        encode_and_send_return_value(method_id, call_id, ipc_status::unexpected_exception, std::move(error_message));
    }
}

template <class I>
void abstract_stub<I>::handle_dismiss_call(data_istream &message)
{
    WC_TRACE_SET_MARKER(wc_trace_channels::all, "Com stub: call dismissed");
    uint32_t call_id;
    coder::decode(message, call_id);
    {
        std::lock_guard<std::mutex> lock(open_target_calls_mutex_m);
        if (open_target_calls_m.count(call_id) > 0)
        {
            open_target_calls_m.at(call_id)->dismiss();
            open_target_calls_m.erase(call_id);
        }
    }
}

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_ABSTRACT_STUB_HPP_
//---- End of source file ------------------------------------------------------
