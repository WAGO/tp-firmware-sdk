//------------------------------------------------------------------------------
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
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
///  \brief    Implementation of a the APITest parameter provider for file parameters
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_parameter_provider.hpp"
#include "generic_file_provider.hpp"

#include <wago/wdx/linuxos/com/join_future_responses.hpp>
#include <wago/wdx/parameter_service_backend_i.hpp>
#include <wago/wdx/linuxos/file/file_parameter_handler.hpp>
#include <wago/wdx/linuxos/file/exception.hpp>
#include <wc/assertion.h>
#include <wc/log.h>

#include <exception>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

using wdx::parameter_selector;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
file_parameter_provider::file_parameter_provider(wdx::device_selector            const &device_selector,
                                                 wdx::parameter_id_t             const &writable_file_parameter,
                                                 bool                                   fail_on_write_parameter,
                                                 wdx::parameter_service_backend_i      &backend)
: device_selector_m(device_selector)
, file_parameter_id_m(writable_file_parameter)
, fail_on_write_m(fail_on_write_parameter)
, backend_m(backend)
{
    handler_m = std::make_unique<file::file_parameter_handler>(file_parameter_id_m, backend_m, [](bool readonly){
        if(readonly)
        {
            // Initial provider for read access
            std::string const initial_content = "Initial file content";
            return std::make_unique<generic_file_provider>(std::vector<uint8_t>(initial_content.data(), initial_content.data() + initial_content.size()));
        }
        else
        {
            return std::make_unique<generic_file_provider>(std::vector<uint8_t>(), true);
        }
    });
}

file_parameter_provider::~file_parameter_provider() noexcept = default;

wdx::parameter_selector_response file_parameter_provider::get_provided_parameters()
{
    std::vector<wdx::parameter_selector> selectors;
    selectors.push_back(wdx::parameter_selector::all_with_definition(file_parameter_id_m, device_selector_m));

    return wdx::parameter_selector_response(selectors);
}

future<std::vector<wdx::value_response>> file_parameter_provider::get_parameter_values(std::vector<wdx::parameter_instance_id> parameter_ids)
{
    std::vector<future<wdx::value_response>> single_futures;
    for(size_t i = 0; i < parameter_ids.size(); ++i)
    {
        WC_ASSERT(parameter_ids.at(i).id == file_parameter_id_m);
        if (parameter_ids.at(i).id != file_parameter_id_m)
        {
            single_futures.push_back(resolved_future(wdx::value_response()));
            continue;
        }

        try
        {
            auto joinable = std::make_shared<promise<wdx::value_response>>();
            {
                auto future_id = handler_m->get_file_id();
                future_id.set_notifier([joinable](auto file_id_value){
                    joinable->set_value(wdx::value_response(file_id_value));
                });
                future_id.set_exception_notifier([joinable](std::exception_ptr e_ptr){
                    joinable->set_value(file::create_error_response_from_ptr<wdx::value_response>(e_ptr, "Failed to get file ID for read: "));
                });
            }
            single_futures.push_back(joinable->get_future());
        }
        catch(std::exception const &e)
        {
            std::string const error_message = "Failed to get file ID for read: " + std::string(e.what());
            WC_FAIL(error_message.c_str());
            single_futures.push_back(resolved_future(wdx::value_response(wdx::status_codes::internal_error, error_message)));
        }
    }

    WC_ASSERT(single_futures.size() == parameter_ids.size());
    return com::join_future_responses(std::move(single_futures));
}

future<wdx::file_id_response> file_parameter_provider::create_parameter_upload_id(wdx::parameter_id_t context)
{
    WC_ASSERT_RETURN((context == file_parameter_id_m), resolved_future(wdx::file_id_response()));

    auto response_promise = std::make_shared<promise<wdx::file_id_response>>();
    try
    {
        auto future_id = handler_m->create_file_id_for_write(context);
        future_id.set_notifier([response_promise](auto file_id_value){
            response_promise->set_value(wdx::file_id_response(file_id_value->get_file_id()));
        });
        future_id.set_exception_notifier(file::create_exception_handler(response_promise, "Failed to create file ID for write: "));
    }
    catch(std::exception const &e)
    {
        std::string const error_message = "Failed to create file ID for write: " + std::string(e.what());
        WC_FAIL(error_message.c_str());
        return resolved_future(wdx::file_id_response(wdx::status_codes::internal_error, error_message));
    }

    return response_promise->get_future();
}

future<wdx::response> file_parameter_provider::remove_parameter_upload_id(wdx::file_id        id,
                                                                          wdx::parameter_id_t context)
{
    WC_ASSERT_RETURN((context == file_parameter_id_m), resolved_future(wdx::response()));

    try
    {
        handler_m->remove_file_id_for_write(id);
        return resolved_future(wdx::response(wdx::status_codes::success));
    }
    catch(file::exception const &e)
    {
        return resolved_future(wdx::response(e.get_status_code(), e.what()));
    }
}

future<std::vector<wdx::set_parameter_response>> file_parameter_provider::set_parameter_values(std::vector<wdx::value_request> value_requests)
{
    std::vector<future<wdx::set_parameter_response>> single_futures;
    for(size_t i = 0; i < value_requests.size(); ++i)
    {
        WC_ASSERT(value_requests.at(i).param_id.id == file_parameter_id_m);
        if (value_requests.at(i).param_id.id != file_parameter_id_m)
        {
            single_futures.push_back(resolved_future(wdx::set_parameter_response()));
            continue;
        }

        try
        {
            auto validator = [fail_on_write = fail_on_write_m](std::basic_istream<uint8_t>&){
                return !fail_on_write;
            };

            auto joinable = std::make_shared<promise<wdx::set_parameter_response>>();
            {
                auto future_result = handler_m->set_file_id(value_requests.at(i).value, validator);
                future_result.set_notifier([joinable](){
                    joinable->set_value(wdx::set_parameter_response(wdx::status_codes::success));
                });
                future_result.set_exception_notifier([joinable](std::exception_ptr e_ptr){
                    joinable->set_value(file::create_error_response_from_ptr<wdx::set_parameter_response>(e_ptr, "Failed to write file ID value: "));
                });
            }
            single_futures.push_back(joinable->get_future());
        }
        catch(std::exception const &e)
        {
            std::string const error_message = "Failed to write file ID value: " + std::string(e.what());
            WC_FAIL(error_message.c_str());
            single_futures.push_back(resolved_future(wdx::set_parameter_response(wdx::status_codes::internal_error, error_message)));
        }
    }

    WC_ASSERT(single_futures.size() == value_requests.size());
    return com::join_future_responses(std::move(single_futures));
}

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
