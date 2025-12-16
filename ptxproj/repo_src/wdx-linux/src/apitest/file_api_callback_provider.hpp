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
///  \brief    APITest provider to test parameter service file API callbacks
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_APITEST_FILE_API_CALLBACK_PROVIDER_HPP_
#define SRC_APITEST_FILE_API_CALLBACK_PROVIDER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/base_parameter_provider.hpp>
#include <wago/wdx/linuxos/com/exception.hpp>

#define PARAMCOM_INTERNAL_USE
#include <wago/wdx/linuxos/com/parameter_service_file_api_proxy.hpp>
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
class file_api_callback_provider final : public wdx::base_parameter_provider
{
WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(file_api_callback_provider)
private:
    wdx::device_selector device_selector_m;
    wdx::parameter_id_t  read_file_method_id_m;
    wdx::parameter_id_t  write_file_method_id_m;
    wdx::parameter_id_t  get_file_info_method_id_m;
    wdx::parameter_id_t  create_file_method_id_m;

    // the file api proxy. must be hold by this class to guarantee
    // exclusive control over the underlying IPC communication!
    com::parameter_service_file_api_proxy file_api_proxy_m;

public:
    file_api_callback_provider(wdx::device_selector device_selector,
                               wdx::parameter_id_t  read_file_method_id,
                               wdx::parameter_id_t  write_file_method_id,
                               wdx::parameter_id_t  get_file_info_method_id,
                               wdx::parameter_id_t  create_file_method_id);
    ~file_api_callback_provider() noexcept override;

    wdx::parameter_selector_response get_provided_parameters() override;
    future<wdx::method_invocation_response> invoke_method(wdx::parameter_instance_id method_id, std::vector<std::shared_ptr<wdx::parameter_value>> in_args) override;

private:

    void do_read_file_call(wdx::file_id read_id, uint64_t read_offset, size_t read_length, 
                           std::shared_ptr<promise<wdx::method_invocation_response>> pending_response,
                           std::shared_ptr<std::atomic<bool>>                        should_run_frontend_thread,
                           bool                                                      second_try = false);
    
    void do_write_file_call(wdx::file_id write_id, uint64_t write_offset, std::vector<uint8_t> write_data_as_vector, 
                            std::shared_ptr<promise<wdx::method_invocation_response>> pending_response,
                            std::shared_ptr<std::atomic<bool>>                        should_run_frontend_thread,
                            bool                                                      second_try = false);
    

    void do_get_file_info(wdx::file_id                                              info_id,
                          std::shared_ptr<promise<wdx::method_invocation_response>> pending_response,
                          std::shared_ptr<std::atomic<bool>>                        should_run_frontend_thread,
                          bool                                                      second_try = false);

    
    void do_create_file(wdx::file_id create_id, uint64_t create_capacity,
                        std::shared_ptr<promise<wdx::method_invocation_response>> pending_response,
                        std::shared_ptr<std::atomic<bool>>                        should_run_frontend_thread,
                        bool                                                      second_try = false);

private:
    template<class CoreResponse>
    auto get_notifier_for_file_api_call(std::shared_ptr<wago::promise<CoreResponse>> pending_response, std::shared_ptr<std::atomic<bool>> should_run_thread)
    {
        return [this, &driver=file_api_proxy_m, should_run_thread, pending_response] (auto core_response) {
            should_run_thread->store(false);
            if(core_response.is_success())
            {
                pending_response->set_value(this->get_method_response_for_file_api_response(core_response));
            }
            else
            {
                auto core_status = static_cast<uint16_t>(core_response.status);
                pending_response->set_value(CoreResponse(core_status,
                                                         "Forwarded core status code: " +
                                                         std::to_string(core_status)));
            }
            
            driver.stop();
        };
    }

    template<class CoreResponse>
    auto get_exception_notifier_for_file_api_call(std::shared_ptr<wago::promise<CoreResponse>> pending_response, std::shared_ptr<std::atomic<bool>> should_run_thread,
                                                  bool second_try, std::function<void()> retry_method)
    {
        return [&driver=file_api_proxy_m, should_run_thread, pending_response, second_try, retry_method] (std::exception_ptr e_ptr) {
            should_run_thread->store(false);
            try 
            {
                if (e_ptr)
                {
                    std::rethrow_exception(e_ptr);
                }
            }
            catch(wdx::linuxos::com::exception &ex) // connection error
            {
                if (second_try)
                {
                    pending_response->set_value(wdx::method_invocation_response(
                        wdx::status_codes::could_not_invoke_method, (std::string("File frontend exception: ") + ex.what())
                    ));
                }
                else
                {
                    while(driver.run_once());
                    retry_method();
                    return;
                }
            }
            catch (std::exception const &ex)
            {
                pending_response->set_value(wdx::method_invocation_response(
                    wdx::status_codes::could_not_invoke_method, (std::string("File frontend exception: ") + ex.what())
                ));
            }
            driver.stop();
        };
    }

    template<class CoreResponse>
    wdx::method_invocation_response get_method_response_for_file_api_response(CoreResponse const &core_response);
};


} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_APITEST_FILE_API_CALLBACK_PROVIDER_HPP_

//---- End of source file ------------------------------------------------------
