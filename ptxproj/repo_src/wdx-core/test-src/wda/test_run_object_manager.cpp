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
///  \brief    Test run object manager.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/run_object_manager.hpp"
#include "http/http_exception.hpp"

#include <wago/wdx/test/wda_check.hpp>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::wda::rest::run_object_manager;
using wago::wdx::wda::http::http_exception;
using wago::wdx::wda::http::http_status_code;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(run_object_manager, construct_delete)
{
    run_object_manager run_manager;
}

TEST(run_object_manager, add_by_response)
{
    wago::wdx::method_invocation_named_response response(wago::wdx::status_codes::success);
    uint16_t const timeout = 900;
    wago::wdx::parameter_instance_path instance_id("param", "0-0");

    run_object_manager run_manager;
    auto run_id     = run_manager.add_run(instance_id, timeout, std::move(response));
    auto run_object = run_manager.get_run(instance_id, run_id);
    ASSERT_NE(nullptr, run_object->invocation_response.get());
    EXPECT_WDA_SUCCESS(*(run_object->invocation_response));
    EXPECT_GE(static_cast<uint32_t>(timeout), run_object->get_timeout_left());
}

TEST(run_object_manager, add_by_future)
{
    wago::promise<wago::wdx::method_invocation_named_response> test_promise;
    wago::wdx::method_invocation_named_response response(wago::wdx::status_codes::success);
    uint16_t const timeout = 900;
    wago::wdx::parameter_instance_path instance_id("param", "0-0");

    // Prepare handler
    std::string handler_call_id;
    run_object_manager::ready_handler handler = [&handler_call_id](std::string run_id){
        handler_call_id = run_id;
    };

    // Prepare Manager and test add
    run_object_manager run_manager;
    auto run_id     = run_manager.add_run(instance_id, timeout, test_promise.get_future(), handler);
    auto run_object = run_manager.get_run(instance_id, run_id);
    EXPECT_EQ(nullptr, run_object->invocation_response.get());

    // Test solving future
    test_promise.set_value(std::move(response));
    EXPECT_EQ(run_id, handler_call_id);
    EXPECT_NE(nullptr, run_object->invocation_response.get());
    EXPECT_WDA_SUCCESS(*(run_object->invocation_response));
    EXPECT_GE(static_cast<uint32_t>(timeout), run_object->get_timeout_left());
}

TEST(run_object_manager, remove_and_not_found)
{
    wago::wdx::method_invocation_named_response response(wago::wdx::status_codes::success);
    wago::wdx::parameter_instance_path instance_id("param", "0-0");

    // Add and remove
    run_object_manager run_manager;
    auto run_id = run_manager.add_run(instance_id, 900, std::move(response));
                  run_manager.remove_run(instance_id, run_id);

    // Check for not found exception
    bool exception_thrown = false;
    try
    {
        run_manager.get_run(instance_id, run_id);
    }
    catch(http_exception &e)
    {
        exception_thrown = true;
        EXPECT_STRNE("", e.what());
        EXPECT_EQ(http_status_code::not_found, e.get_http_status_code());
    }
    EXPECT_TRUE(exception_thrown);
}

TEST(run_object_manager, not_found)
{
    // Add and remove
    run_object_manager run_manager;
    wago::wdx::parameter_instance_path instance_id("param", "0-0");

    // Check for not found exception
    bool exception_thrown = false;
    try
    {
        run_manager.get_run(instance_id, "probablynotexisting");
    }
    catch(http_exception &e)
    {
        exception_thrown = true;
        EXPECT_STRNE("", e.what());
        EXPECT_EQ(http_status_code::not_found, e.get_http_status_code());
    }
    EXPECT_TRUE(exception_thrown);
}

TEST(run_object_manager, cleanup)
{
    wago::wdx::method_invocation_named_response response1(wago::wdx::status_codes::success);
    wago::wdx::parameter_instance_path instance_id1("param", "0-0");
    wago::wdx::method_invocation_named_response response2(wago::wdx::status_codes::success);
    wago::wdx::parameter_instance_path instance_id2("param2", "0-0");

    // Add, get and clean
    run_object_manager run_manager;
    auto run_id1 = run_manager.add_run(instance_id1,   0, std::move(response1));
    auto run_id2 = run_manager.add_run(instance_id2, 900, std::move(response2));
                   run_manager.get_run(instance_id1, run_id1);
                   run_manager.get_run(instance_id2, run_id2);
                   run_manager.clean_runs();

    // Check for not found exception
    bool exception_thrown = false;
    try
    {
        run_manager.get_run(instance_id1, run_id1);
    }
    catch(http_exception &e)
    {
        exception_thrown = true;
        EXPECT_STRNE("", e.what());
        EXPECT_EQ(http_status_code::not_found, e.get_http_status_code());
    }
    EXPECT_TRUE(exception_thrown);
    run_manager.get_run(instance_id2, run_id2);
}

TEST(run_object_manager, add_max)
{
    wago::wdx::method_invocation_named_response response(wago::wdx::status_codes::success);
    uint16_t const timeout = 900;
    wago::wdx::parameter_instance_path instance_id("param", "0-0");

    run_object_manager run_manager;
    for(size_t i = 0; i < run_object_manager::max_runs; i++)
    {
        ASSERT_FALSE(run_manager.max_runs_reached());
        auto response_to_add = response;
        auto run_id          = run_manager.add_run(instance_id, timeout, std::move(response_to_add));
    }
    EXPECT_TRUE(run_manager.max_runs_reached());
}


//---- End of source file ------------------------------------------------------
