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
///  \brief    Implementation of a the APITest provider for frontend callbacks
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "frontend_callback_provider.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"

#include <wc/assertion.h>
#include <wc/log.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

frontend_callback_provider::frontend_callback_provider(wdx::device_selector device_selector,
                                                       wdx::parameter_id_t  own_parameter_id,
                                                       wdx::parameter_id_t  read_callback_id,
                                                       wdx::parameter_id_t  write_callback_id,
                                                       wdx::parameter_id_t  method_callback_id,
                                                       bool                 should_dismiss)
: device_selector_m(device_selector)
, own_parameter_id_m(own_parameter_id)
, read_callback_id_m(read_callback_id, 0, device_selector.get_selected_device())
, write_callback_id_m(write_callback_id, 0, device_selector.get_selected_device())
, method_callback_id_m(method_callback_id, 0, device_selector.get_selected_device())
, should_dismiss_m(should_dismiss)
, frontend_proxy_m("APITest Callback Frontend")
{
    WC_ASSERT((!device_selector.is_any_selector()) && (!device_selector.is_collection_selector()));
}

frontend_callback_provider::~frontend_callback_provider() noexcept = default;

wdx::parameter_selector_response frontend_callback_provider::get_provided_parameters()
{
    return wdx::parameter_selector_response({wdx::parameter_selector::all_with_definition(own_parameter_id_m, device_selector_m)});
}

future<std::vector<wdx::value_response>> frontend_callback_provider::get_parameter_values(std::vector<wdx::parameter_instance_id> parameter_ids)
{
    auto pending_responses          = std::make_shared<promise<std::vector<wdx::value_response>>>();
    auto responses                  = std::make_shared<std::vector<wdx::value_response>>();
    auto should_run_frontend_thread = std::make_shared<std::atomic<bool>>(false);

    for (auto const &id : parameter_ids) {
        responses->push_back(wdx::value_response());
        auto &single_response = responses->back();
        if(id.id != own_parameter_id_m)
        {
            // not this providers responsibility: ignoring
        }
        else
        {
            should_run_frontend_thread->store(true);
            do_frontend_get_call(pending_responses, responses, should_run_frontend_thread, single_response);
        }
    }
    if(should_run_frontend_thread->load())
    {
        std::thread frontend_worker([&driver=frontend_proxy_m, should_run_frontend_thread] {
            while(should_run_frontend_thread->load()) 
            {
                driver.run();
            }
        });
        frontend_worker.detach();

        return pending_responses->get_future();
    }
    else
    {
        return resolved_future(std::move(*responses));
    }
}

void frontend_callback_provider::do_frontend_get_call(std::shared_ptr<promise<std::vector<wdx::value_response>>>  pending_responses,
                                                  std::shared_ptr<std::vector<wdx::value_response>>           responses,
                                                  std::shared_ptr<std::atomic<bool>>                          should_run_frontend_thread,
                                                  wdx::value_response                                        &single_response,
                                                  bool                                                        second_try)
{
    auto pending_frontend_call = frontend_proxy_m.get_frontend().get_parameters({read_callback_id_m});
    pending_frontend_call.set_notifier([&driver=frontend_proxy_m, should_run_frontend_thread, pending_responses, responses, &single_response, expected_dismiss=should_dismiss_m] (auto frontend_response) {
        if(expected_dismiss)
        {
            uint16_t unexpected_success = 1042;
            single_response.set_domain_specific_error(unexpected_success, "Unexpected success");
        }
        else
        {
            if(frontend_response.at(0).is_success())
            {
                single_response.set_value(frontend_response.at(0).value);
            }
            else
            {
                auto core_status = static_cast<uint16_t>(frontend_response.at(0).status);
                single_response.set_domain_specific_error(core_status,
                                                          "Forwarded core status code: " +
                                                          std::to_string(core_status));
            }
        }
        should_run_frontend_thread->store(false);
        pending_responses->set_value(std::move(*responses));
        driver.stop();
    });
    pending_frontend_call.set_exception_notifier([this, &driver=frontend_proxy_m, should_run_frontend_thread, pending_responses, responses, &single_response, second_try, expected_dismiss=should_dismiss_m] (std::exception_ptr e_ptr) {
        try
        {
            if(e_ptr)
            {
                std::rethrow_exception(e_ptr);
            }
        }
        catch(std::future_error const &)
        {
            if(expected_dismiss)
            {
                // TODO: check if this is the correct future_error and not something else?
                uint16_t successfully_dismissed = 1337;
                single_response.set_domain_specific_error(successfully_dismissed, "The frontend call was successfully dismissed.");
            }
            else // unexpected future error / dismissal
            {
                uint16_t unexpectetly_dismissed = 1234;
                single_response.set_domain_specific_error(unexpectetly_dismissed, "The frontend call was dismissed unexpectedly.");
            }
        }
        catch(wdx::linuxos::com::exception &e) // connection error
        {
            // workaround for reconnect when the paramd has been restartet
            if(second_try)
            {
                single_response.set_error(wdx::status_codes::internal_error, std::string("The frontend call raised an unexpected error: ") + e.what());
            }
            else
            {
                while(driver.run_once());
                do_frontend_get_call(pending_responses, responses, should_run_frontend_thread, single_response, true);
                return;
            }
        }
        catch(...) // unexpected error
        {
            single_response.set_error(wdx::status_codes::internal_error, "The frontend call raised an unexpected error");
        }
        should_run_frontend_thread->store(false);
        pending_responses->set_value(std::move(*responses));
        driver.stop();
    });
    
    if(should_dismiss_m)
    {
        pending_frontend_call.dismiss();
    }
}

future<std::vector<wdx::set_parameter_response>> frontend_callback_provider::set_parameter_values(std::vector<wdx::value_request> value_requests)
{
    auto pending_responses          = std::make_shared<promise<std::vector<wdx::set_parameter_response>>>();
    auto responses                  = std::make_shared<std::vector<wdx::set_parameter_response>>();
    auto should_run_frontend_thread = std::make_shared<std::atomic<bool>>(false);

    for (auto const &req : value_requests) {
        responses->push_back(wdx::set_parameter_response());
        auto &single_response = responses->back();
        if(req.param_id.id != own_parameter_id_m)
        {
            // not this providers responsibility: ignoring
        }
        else
        {
            should_run_frontend_thread->store(true);
            do_frontend_set_call(req.value, pending_responses, responses, should_run_frontend_thread, single_response);
        }
    }
    if(should_run_frontend_thread->load())
    {
        std::thread frontend_worker([&driver=frontend_proxy_m, should_run_frontend_thread] {
            while(should_run_frontend_thread->load()) 
            {
                driver.run();
            }
        });
        frontend_worker.detach();

        return pending_responses->get_future();
    }
    else
    {
        return resolved_future(std::move(*responses));
    }
}

void frontend_callback_provider::do_frontend_set_call(std::shared_ptr<wdx::parameter_value>                              const  req_value,
                                                      std::shared_ptr<promise<std::vector<wdx::set_parameter_response>>>        pending_responses,
                                                      std::shared_ptr<std::vector<wdx::set_parameter_response>>                 responses,
                                                      std::shared_ptr<std::atomic<bool>>                                        should_run_frontend_thread,
                                                      wdx::set_parameter_response                                              &single_response,
                                                      bool                                                                      second_try)
{
    auto pending_frontend_call = frontend_proxy_m.get_frontend().set_parameter_values({ wdx::value_request(write_callback_id_m, req_value) });
    pending_frontend_call.set_notifier([&driver=frontend_proxy_m, should_run_frontend_thread, pending_responses, responses, &single_response, expected_dismiss=should_dismiss_m] (auto frontend_response) {
        if(expected_dismiss)
        {
            uint16_t unexpected_success = 1042;
            single_response.set_domain_specific_error(unexpected_success, "Unexpected success");
        }
        else
        {
            if(frontend_response.at(0).is_success())
            {
                single_response.set_success();
            }
            else
            {
                auto core_status = static_cast<uint16_t>(frontend_response.at(0).status);
                single_response.set_domain_specific_error(core_status,
                                                          "Forwarded core status code: " +
                                                          std::to_string(core_status));
            }
        }
        should_run_frontend_thread->store(false);
        pending_responses->set_value(std::move(*responses));
        driver.stop();
    });
    pending_frontend_call.set_exception_notifier([this, &driver=frontend_proxy_m, should_run_frontend_thread, pending_responses, responses, &single_response, req_value, second_try, expected_dismiss=should_dismiss_m] (std::exception_ptr e_ptr) {
        try
        {
            if(e_ptr)
            {
                std::rethrow_exception(e_ptr);
            }
        }
        catch(std::future_error const &)
        {
            if(expected_dismiss)
            {
                // TODO: check if this is the correct future_error and not something else?
                uint16_t successfully_dismissed = 1337;
                single_response.set_domain_specific_error(successfully_dismissed, "The frontend call was successfully dismissed.");
            }
            else // unexpected future error / dismissal
            {
                uint16_t unexpectetly_dismissed = 1234;
                single_response.set_domain_specific_error(unexpectetly_dismissed, "The frontend call was dismissed unexpectedly.");
            }
        }
        catch(wdx::linuxos::com::exception &e) // connection error
        {
            // workaround for reconnect when the paramd has been restartet
            if(second_try)
            {
                single_response.set_error(wdx::status_codes::internal_error, std::string("The frontend call raised an unexpected error: ") + e.what());
            }
            else
            {
                while(driver.run_once());
                do_frontend_set_call(req_value, pending_responses, responses, should_run_frontend_thread, single_response, true);
                return;
            }
        }
        catch(...) // unexpected error
        {
            single_response.set_error(wdx::status_codes::internal_error, "The frontend call raised an unexpected error");
        }
        should_run_frontend_thread->store(false);
        pending_responses->set_value(std::move(*responses));
        driver.stop();
    });

    if(should_dismiss_m)
    {
        pending_frontend_call.dismiss();
    }
}

future<wdx::method_invocation_response> frontend_callback_provider::invoke_method(wdx::parameter_instance_id method_id, std::vector<std::shared_ptr<wdx::parameter_value>>)
{
    auto pending_response           = std::make_shared<promise<wdx::method_invocation_response>>();
    auto response                   = std::make_shared<wdx::method_invocation_response>();
    auto should_run_frontend_thread = std::make_shared<std::atomic<bool>>(false);

    auto &single_response = *response;
    if(method_id.id != own_parameter_id_m)
    {
        // not this providers responsibility: ignoring
    }
    else
    {
        should_run_frontend_thread->store(true);
        do_frontend_method_call(pending_response, response, should_run_frontend_thread, single_response);
    }
    
    if(should_run_frontend_thread->load())
    {
        std::thread frontend_worker([&driver=frontend_proxy_m, should_run_frontend_thread] {
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
        return resolved_future(std::move(*response));
    }
}


void frontend_callback_provider::do_frontend_method_call(std::shared_ptr<promise<wdx::method_invocation_response>>  pending_response,
                                                         std::shared_ptr<wdx::method_invocation_response>           response,
                                                         std::shared_ptr<std::atomic<bool>>                         should_run_frontend_thread,
                                                         wdx::method_invocation_response                           &single_response,
                                                         bool                                                       second_try)
{
    auto pending_frontend_call = frontend_proxy_m.get_frontend().invoke_method(method_callback_id_m, {});
    pending_frontend_call.set_notifier([&driver=frontend_proxy_m, should_run_frontend_thread, pending_response, response, &single_response, expected_dismiss=should_dismiss_m] (auto frontend_response) {
    if(expected_dismiss)
    {
        uint16_t unexpected_success = 1042;
        single_response.set_domain_specific_error(unexpected_success, "Unexpected success");
    }
    else
    {
        if(frontend_response.is_success())
        {
            single_response.set_out_args({});
        }
        else
        {
            auto core_status = static_cast<uint16_t>(frontend_response.status);
            single_response.set_domain_specific_error(core_status,
                                                      "Forwarded core status code: " +
                                                      std::to_string(core_status));
        }
    }
    should_run_frontend_thread->store(false);
    pending_response->set_value(std::move(*response));
    driver.stop();
    });
    pending_frontend_call.set_exception_notifier([this, &driver=frontend_proxy_m, should_run_frontend_thread, pending_response, response, &single_response, second_try, expected_dismiss=should_dismiss_m] (std::exception_ptr e_ptr) {
        try
        {
            if(e_ptr)
            {
                std::rethrow_exception(e_ptr);
            }
        }
        catch(std::future_error const &)
        {
            if(expected_dismiss)
            {
                // TODO: check if this is the correct future_error and not something else?
                uint16_t successfully_dismissed = 1337;
                single_response.set_domain_specific_error(successfully_dismissed, "The frontend call was successfully dismissed.");
            }
            else // unexpected future error / dismissal
            {
                uint16_t unexpectetly_dismissed = 1234;
                single_response.set_domain_specific_error(unexpectetly_dismissed, "The frontend call was dismissed unexpectedly.");
            }
        }
        catch(wdx::linuxos::com::exception &e) // connection error
        {
            // workaround for reconnect when the paramd has been restartet
            if(second_try)
            {
                single_response.set_error(wdx::status_codes::internal_error, std::string("The frontend call raised an unexpected error: ") + e.what());
            }
            else
            {
                while(driver.run_once());
                do_frontend_method_call(pending_response, response, should_run_frontend_thread, single_response, true);
                return;
            }
        }
        catch(...) // unexpected error
        {
            single_response.set_error(wdx::status_codes::internal_error, "The frontend call raised an unexpected error");
        }
        should_run_frontend_thread->store(false);
        pending_response->set_value(std::move(*response));
        driver.stop();
    });

    if(should_dismiss_m)
    {
        pending_frontend_call.dismiss();
    }
}

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
