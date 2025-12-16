//------------------------------------------------------------------------------
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
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
///  \brief    Implementation of a the APITest provider for file API callbacks
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_api_callback_provider.hpp"

#include <wc/assertion.h>
#include <wc/log.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

template<>
wdx::method_invocation_response file_api_callback_provider::get_method_response_for_file_api_response(wdx::file_read_response const &core_response);
template<>
wdx::method_invocation_response file_api_callback_provider::get_method_response_for_file_api_response(wdx::response const &core_response);
template<>
wdx::method_invocation_response file_api_callback_provider::get_method_response_for_file_api_response(wdx::file_info_response const &core_response);

file_api_callback_provider::file_api_callback_provider(wdx::device_selector device_selector,
                                                       wdx::parameter_id_t  read_file_method_id,
                                                       wdx::parameter_id_t  write_file_method_id,
                                                       wdx::parameter_id_t  get_file_info_method_id,
                                                       wdx::parameter_id_t  create_file_method_id)
: device_selector_m(device_selector)
, read_file_method_id_m(read_file_method_id)
, write_file_method_id_m(write_file_method_id)
, get_file_info_method_id_m(get_file_info_method_id)
, create_file_method_id_m(create_file_method_id)
, file_api_proxy_m("APITest Callback File API")
{
    WC_ASSERT((!device_selector.is_any_selector()) && (!device_selector.is_collection_selector()));
}

file_api_callback_provider::~file_api_callback_provider() noexcept = default;

wdx::parameter_selector_response file_api_callback_provider::get_provided_parameters()
{
    return wdx::parameter_selector_response({
        wdx::parameter_selector::all_with_definition(read_file_method_id_m, device_selector_m),
        wdx::parameter_selector::all_with_definition(write_file_method_id_m, device_selector_m),
        wdx::parameter_selector::all_with_definition(get_file_info_method_id_m, device_selector_m),
        wdx::parameter_selector::all_with_definition(create_file_method_id_m, device_selector_m)
    });
}

future<wdx::method_invocation_response> file_api_callback_provider::invoke_method(wdx::parameter_instance_id method_id, std::vector<std::shared_ptr<wdx::parameter_value>> in_args)
{
    auto pending_response           = std::make_shared<promise<wdx::method_invocation_response>>();
    auto should_run_frontend_thread = std::make_shared<std::atomic<bool>>(false);

    if (method_id.id == read_file_method_id_m)
    {
        // get args
        auto read_id     = in_args.at(0)->get_file_id();
        auto read_offset = in_args.at(1)->get_uint64();
        auto read_length = in_args.at(2)->get_uint32();

        // make file api call
        should_run_frontend_thread->store(true);
        do_read_file_call(read_id, read_offset, read_length, pending_response, should_run_frontend_thread);
    }
    else if (method_id.id == write_file_method_id_m)
    {
        // get args
        auto write_id     = in_args.at(0)->get_file_id();
        auto write_offset = in_args.at(1)->get_uint64();
        auto write_data   = in_args.at(2)->get_items();

        // fixme: this is really unhandy... why can't we get the uint8 vector from the in args directly?
        std::vector<uint8_t> write_data_as_vector;
        for (auto const &one_byte : write_data)
        {
            write_data_as_vector.push_back(one_byte.get_uint8());
        }

        // make file api call
        should_run_frontend_thread->store(true);
        do_write_file_call(write_id, write_offset, write_data_as_vector, pending_response, should_run_frontend_thread);
    }
    else if (method_id.id == get_file_info_method_id_m)
    {
        // get args
        auto info_id = in_args.at(0)->get_file_id();

        // make file api call
        should_run_frontend_thread->store(true);
        do_get_file_info(info_id, pending_response, should_run_frontend_thread);
    }
    else if (method_id.id == create_file_method_id_m)
    {
        // get args
        auto create_id       = in_args.at(0)->get_file_id();
        auto create_capacity = in_args.at(1)->get_uint64();

        // make file api call
        should_run_frontend_thread->store(true);     
        do_create_file(create_id, create_capacity, pending_response, should_run_frontend_thread);
    }

    if (should_run_frontend_thread->load())
    {
        std::thread frontend_worker([&driver=file_api_proxy_m, should_run_frontend_thread] {
            while(should_run_frontend_thread->load()) 
            {
                driver.run();
            }
        });
        frontend_worker.detach();

        return pending_response->get_future();
    }
    else
    {
        return resolved_future(wdx::method_invocation_response());
    }
}

void file_api_callback_provider::do_read_file_call(wdx::file_id read_id, uint64_t read_offset, size_t read_length, 
                                                   std::shared_ptr<promise<wdx::method_invocation_response>> pending_response,
                                                   std::shared_ptr<std::atomic<bool>> should_run_frontend_thread,
                                                   bool second_try)
{
    auto pending_frontend_call = file_api_proxy_m.get_file_api().file_read(read_id, read_offset, read_length);
    pending_frontend_call.set_notifier(get_notifier_for_file_api_call(pending_response, should_run_frontend_thread));
    pending_frontend_call.set_exception_notifier(get_exception_notifier_for_file_api_call(pending_response, should_run_frontend_thread, second_try, [=](){
        do_read_file_call(read_id, read_offset, read_length, pending_response, should_run_frontend_thread, true);
    }));
}

void file_api_callback_provider::do_write_file_call(wdx::file_id write_id, uint64_t write_offset, std::vector<uint8_t> write_data_as_vector, 
                                                    std::shared_ptr<promise<wdx::method_invocation_response>> pending_response,
                                                    std::shared_ptr<std::atomic<bool>> should_run_frontend_thread,
                                                    bool second_try)
{
    auto pending_frontend_call = file_api_proxy_m.get_file_api().file_write(write_id, write_offset, write_data_as_vector);
    pending_frontend_call.set_notifier(get_notifier_for_file_api_call(pending_response, should_run_frontend_thread));
    pending_frontend_call.set_exception_notifier(get_exception_notifier_for_file_api_call(pending_response, should_run_frontend_thread, second_try, [=](){
        do_write_file_call(write_id, write_offset, write_data_as_vector, pending_response, should_run_frontend_thread, true);
    }));
}

void file_api_callback_provider::do_get_file_info(wdx::file_id info_id,
                                                  std::shared_ptr<promise<wdx::method_invocation_response>> pending_response,
                                                  std::shared_ptr<std::atomic<bool>> should_run_frontend_thread,
                                                  bool second_try)
{
    auto pending_frontend_call = file_api_proxy_m.get_file_api().file_get_info(info_id);
    pending_frontend_call.set_notifier(get_notifier_for_file_api_call(pending_response, should_run_frontend_thread));
    pending_frontend_call.set_exception_notifier(get_exception_notifier_for_file_api_call(pending_response, should_run_frontend_thread, second_try, [=](){
        do_get_file_info(info_id, pending_response, should_run_frontend_thread, true);
    }));
}

void file_api_callback_provider::do_create_file(wdx::file_id create_id, uint64_t create_capacity,
                                                std::shared_ptr<promise<wdx::method_invocation_response>> pending_response,
                                                std::shared_ptr<std::atomic<bool>> should_run_frontend_thread,
                                                bool second_try)
{
    auto pending_frontend_call = file_api_proxy_m.get_file_api().file_create(create_id, create_capacity);
    pending_frontend_call.set_notifier(get_notifier_for_file_api_call(pending_response, should_run_frontend_thread));
    pending_frontend_call.set_exception_notifier(get_exception_notifier_for_file_api_call(pending_response, should_run_frontend_thread, second_try, [=](){
        do_create_file(create_id, create_capacity, pending_response, should_run_frontend_thread, true);
    }));
}

template<>
wdx::method_invocation_response file_api_callback_provider::get_method_response_for_file_api_response(wdx::file_read_response const &core_response)
{
    if (!core_response.has_error())
    {
        return wdx::method_invocation_response({
            wdx::parameter_value::create_uint8_array(core_response.data)
        });
    }
    else
    {
        return wdx::method_invocation_response(
            wdx::status_codes::could_not_invoke_method, ("File frontend error: " + wdx::to_string(core_response.status))
        );
    }
}

template<>
wdx::method_invocation_response file_api_callback_provider::get_method_response_for_file_api_response(wdx::response const &core_response)
{
    if (!core_response.has_error())
    {
        return wdx::method_invocation_response(wdx::status_codes::success);
    }
    else
    {
        return wdx::method_invocation_response(
            wdx::status_codes::could_not_invoke_method, ("File frontend error: " + wdx::to_string(core_response.status))
        );
    }
}

template<>
wdx::method_invocation_response file_api_callback_provider::get_method_response_for_file_api_response(wdx::file_info_response const &core_response)
{
    if (!core_response.has_error())
    {
        return wdx::method_invocation_response({
            wdx::parameter_value::create_uint64(core_response.file_size)
        });
    }
    else
    {
        return wdx::method_invocation_response(
            wdx::status_codes::could_not_invoke_method, ("File frontend error: " + wdx::to_string(core_response.status))
        );
    }
}

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
