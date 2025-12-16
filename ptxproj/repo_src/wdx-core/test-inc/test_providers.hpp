//------------------------------------------------------------------------------
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
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
///  \brief    WDA test providers for google test/mock.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_TEST_PROVIDERS_HPP_
#define TEST_INC_TEST_PROVIDERS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "posix_sync.hpp"
#include "model_provider_i.hpp"
#include "device_description_provider_i.hpp"
#include "device_extension_provider_i.hpp"
#include <wago/wdx/base_parameter_provider.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>
#include <memory>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace {

class test_model_provider : public wago::wdx::model_provider_i
{
private:
    wago::wdx::wdm_content_t const model_m;

public:
    test_model_provider(wago::wdx::wdm_content_t const model)
    : model_m(model)
    {}
    ~test_model_provider() override = default;

    wago::future<wago::wdx::wdm_response> get_model_information() override
    {
        return wago::resolved_future(wago::wdx::wdm_response(std::move(wago::wdx::wdm_content_t(model_m))));
    }
};


class test_description_provider : public wago::wdx::device_description_provider_i
{
private:
    wago::wdx::device_selector const selector_m;
    wago::wdx::wdd_content_t   const description_m;

public:
    test_description_provider(wago::wdx::device_selector const selector,
                              wago::wdx::wdd_content_t   const description)
    : selector_m(selector), description_m(description)
    {}
    ~test_description_provider() override = default;

    wago::wdx::device_selector_response get_provided_devices() override
    {
        return wago::wdx::device_selector_response({ selector_m });
    }
    wago::future<wago::wdx::wdd_response> get_device_information(std::string const, std::string const) override
    {
        return wago::resolved_future(wago::wdx::wdd_response::from_pure_wdd(std::move(wago::wdx::wdd_content_t(description_m))));
    }
};


class test_extension_provider : public wago::wdx::device_extension_provider_i
{
private:
    wago::wdx::device_selector const selector_m;
    std::string                const feature_id_m;

public:
    test_extension_provider(wago::wdx::device_selector const  selector,
                            std::string                const &feature_id)
    : selector_m(selector), feature_id_m(feature_id)
    {}
    ~test_extension_provider() override = default;

    wago::wdx::device_extension_response get_device_extensions() override
    {
        return wago::wdx::device_extension_response({ selector_m }, { feature_id_m });
    }
};


class test_parameter_provider : public wago::wdx::base_parameter_provider
{
private:
    using promise_type = wago::promise<std::vector<wago::wdx::value_response>>;
    wago::wdx::parameter_selector               const selector_m;
    std::shared_ptr<wago::wdx::parameter_value> const value_m;
    bool                                        const sync_m;
    bool                                        const allow_call_in_parallel_m;
    std::unique_ptr<promise_type>                     current_promise_m;
    std::vector<wago::wdx::value_response>            current_prepared_responses_m;
    posix_mutex                                       mutex_m;
    std::string                                       error_message_m;

public:
    test_parameter_provider(wago::wdx::parameter_selector               const selector,
                            std::shared_ptr<wago::wdx::parameter_value> const value,
                            bool                                        const sync = true,
                            bool                                        const allow_call_in_parallel = true)
    : selector_m(selector), value_m(value), sync_m(sync), allow_call_in_parallel_m(allow_call_in_parallel)
    {}
    ~test_parameter_provider() override = default;

    wago::wdx::parameter_selector_response get_provided_parameters() override
    {
        return wago::wdx::parameter_selector_response({ selector_m });
    }
    wago::future<std::vector<wago::wdx::value_response>> get_parameter_values(std::vector<wago::wdx::parameter_instance_id> parameter_ids) override
    {
        std::lock_guard<posix_mutex> lock(mutex_m);
        bool sync = sync_m;

        // Check core promise (-> called in parallel!)
        if(current_promise_m.get() != nullptr)
        {
            if(allow_call_in_parallel_m)
            {
                // Answer in sync way, because promise is already in use
                sync = true;
            }
            else
            {
                auto responses = std::vector<wago::wdx::value_response>(parameter_ids.size());
                error_message_m = "Provider should not called while another work is in progress";
                for(auto response : current_prepared_responses_m)
                {
                    response.set_error(wago::wdx::status_codes::internal_error, error_message_m);
                }
                return wago::resolved_future(std::move(responses));
            }
        }

        // Prepare responses
        std::vector<wago::wdx::value_response> responses;
        for(auto id : parameter_ids)
        {
            if(id.id == selector_m.get_selected_id())
            {
                responses.push_back(wago::wdx::value_response(value_m));
            }
            else
            {
                responses.push_back(wago::wdx::value_response(wago::wdx::status_codes::parameter_not_provided));
            }
        }

        if(sync)
        {
            // Return results in a sync way
            return wago::resolved_future<std::vector<wago::wdx::value_response>>(std::move(responses));
        }
        else
        {
            // Return results in an async way
            current_prepared_responses_m = std::move(responses);
            current_promise_m = std::make_unique<promise_type>();

            return current_promise_m->get_future();
        }
    }
    void set_response()
    {
        std::lock_guard<posix_mutex> lock(mutex_m);

        // Check for an outstanding promise
        if(current_promise_m.get() != nullptr)
        {
            current_promise_m->set_value(std::move(current_prepared_responses_m));
            current_promise_m.reset();
        }
    }
    bool has_error()
    {
        return !error_message_m.empty();
    }
    std::string get_error_message()
    {
        return error_message_m;
    }
};

}


#endif // TEST_INC_TEST_PROVIDERS_HPP_

//---- End of source file ------------------------------------------------------
