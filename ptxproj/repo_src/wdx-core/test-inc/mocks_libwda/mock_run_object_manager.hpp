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
///  \brief    Mock run object manager.
///
///  \author   Röh: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_LIBWDA_MOCK_RUN_OBJECT_MANAGER_HPP_
#define TEST_INC_MOCKS_LIBWDA_MOCK_RUN_OBJECT_MANAGER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/run_object_manager.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::wda::rest::run_object_manager_i;
using wago::wdx::wda::rest::method_run_object;
using ready_handler = std::function<void(std::string run_id)>;

using ::testing::Exactly;
using ::testing::AtLeast;
using ::testing::AtMost;
using ::testing::AnyNumber;
using ::testing::WithArgs;
using ::testing::Invoke;
using ::testing::Return;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_run_object_manager : public run_object_manager_i
{
private:
    std::string add_run(wago::wdx::parameter_instance_path     const &method_instance_path,
                        uint16_t                                      timeout_span,
                        wago::wdx::method_invocation_named_response &&response)
    {
       return add_run(method_instance_path, timeout_span, response);
    }
    std::string add_run(wago::wdx::parameter_instance_path                   const &method_instance_path,
                        uint16_t                                                    timeout_span,
                        wago::future<wago::wdx::method_invocation_named_response> &&invoke_future,
                        ready_handler                                               handler)
    {
        return add_run(method_instance_path, timeout_span, invoke_future, handler);
    }
public:
    MOCK_METHOD3(add_run, std::string(wago::wdx::parameter_instance_path    const &method_instance_path,
                                      uint16_t                                     timeout_span,
                                      wago::wdx::method_invocation_named_response &response));
    MOCK_METHOD4(add_run, std::string(wago::wdx::parameter_instance_path                  const &method_instance_path,
                                      uint16_t                                                   timeout_span,
                                      wago::future<wago::wdx::method_invocation_named_response> &invoke_future,
                                      ready_handler                                              handler));
    MOCK_METHOD2(get_run, std::shared_ptr<method_run_object>(wago::wdx::parameter_instance_path const &method_instance_path,
                                                             std::string                               run_id));
    MOCK_METHOD1(get_runs, std::vector<std::shared_ptr<method_run_object>>(wago::wdx::parameter_instance_path const &method_instance_path));
    MOCK_METHOD2(remove_run, void(wago::wdx::parameter_instance_path const &method_instance_path, std::string run_id));
    MOCK_METHOD0(clean_runs, void());
    MOCK_METHOD0(max_runs_reached, bool());

    void set_default_expectations()
    {
        EXPECT_CALL(*this, add_run(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, add_run(::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_run(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_runs(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remove_run(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, clean_runs())
            .Times(0);
        EXPECT_CALL(*this, max_runs_reached())
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_LIBWDA_MOCK_RUN_OBJECT_MANAGER_HPP_
//---- End of source file ------------------------------------------------------
