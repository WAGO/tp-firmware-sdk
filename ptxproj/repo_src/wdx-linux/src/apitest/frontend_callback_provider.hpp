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
///  \brief    APITest provider to test parameter service frontend callbacks
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_APITEST_FRONTEND_CALLBACK_PROVIDER_HPP_
#define SRC_APITEST_FRONTEND_CALLBACK_PROVIDER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/base_parameter_provider.hpp>

#define PARAMCOM_INTERNAL_USE
#include <wago/wdx/linuxos/com/parameter_service_frontend_proxy.hpp>
#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
class frontend_callback_provider final : public wdx::base_parameter_provider
{
WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(frontend_callback_provider)
private:
    wdx::device_selector device_selector_m;
    wdx::parameter_id_t  own_parameter_id_m;

    // callback ids
    wdx::parameter_instance_id  read_callback_id_m;  
    wdx::parameter_instance_id  write_callback_id_m; 
    wdx::parameter_instance_id  method_callback_id_m;

    // should we dismiss frontend calls?
    bool should_dismiss_m;

    // the frontend proxy. must be hold by this class to guarantee
    // exclusive control over the underlying IPC communication!
    com::parameter_service_frontend_proxy frontend_proxy_m;

public:
    /// \param device_selector      Device(s) which the parameter/method is proovided for
    /// \param own_parameter_id     The parameter/method to be provided
    /// \param read_callback_id     Parameter to delegate read calls to. 
    ///                             The value type MUST match the own value type
    /// \param write_callback_id    Parameter to delegate write calls to. 
    ///                             The value type MUST match the own value type.
    /// \param method_callback_id   Method to delegate method invokation calls to. 
    ///                             The in and out argument types MUST match the own types. 
    ///                             At most ONE out arg is supported.
    ///                             No support for in args!
    /// \param mode                 Set if the delegate calles should be dismissed. 
    ///                             Attention: This will cause this provider to respond with 
    ///                             an error.
    frontend_callback_provider(wdx::device_selector device_selector,
                               wdx::parameter_id_t  own_parameter_id,
                               wdx::parameter_id_t  read_callback_id,
                               wdx::parameter_id_t  write_callback_id,
                               wdx::parameter_id_t  method_callback_id,
                               bool                 should_dismiss = false);
    ~frontend_callback_provider() noexcept override;

    wdx::parameter_selector_response get_provided_parameters() override;
    future<std::vector<wdx::value_response>> get_parameter_values(std::vector<wdx::parameter_instance_id> parameter_ids) override;
    future<std::vector<wdx::set_parameter_response>> set_parameter_values(std::vector<wdx::value_request> value_requests) override;
    future<wdx::method_invocation_response> invoke_method(wdx::parameter_instance_id method_id, std::vector<std::shared_ptr<wdx::parameter_value>> in_args) override;

private:
    void do_frontend_get_call(std::shared_ptr<promise<std::vector<wdx::value_response>>>  pending_responses,
                              std::shared_ptr<std::vector<wdx::value_response>>           responses,
                              std::shared_ptr<std::atomic<bool>>                          should_run_frontend_thread,
                              wdx::value_response                                        &single_response,
                              bool                                                        second_try = false);
    void do_frontend_set_call(std::shared_ptr<wdx::parameter_value>                              const  req_value,
                              std::shared_ptr<promise<std::vector<wdx::set_parameter_response>>>        pending_responses,
                              std::shared_ptr<std::vector<wdx::set_parameter_response>>                 responses,
                              std::shared_ptr<std::atomic<bool>>                                        should_run_frontend_thread,
                              wdx::set_parameter_response                                              &single_response,
                              bool                                                                      second_try = false);

    void do_frontend_method_call(std::shared_ptr<promise<wdx::method_invocation_response>>  pending_response,
                                 std::shared_ptr<wdx::method_invocation_response>           response,
                                 std::shared_ptr<std::atomic<bool>>                         should_run_frontend_thread,
                                 wdx::method_invocation_response                           &single_response,
                                 bool                                                       second_try = false);
};

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_APITEST_FRONTEND_CALLBACK_PROVIDER_HPP_

//---- End of source file ------------------------------------------------------
