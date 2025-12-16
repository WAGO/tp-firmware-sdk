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
///  \brief    Test kernel tracer.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wtrace/trace.hpp"
#include "mocks/mock_sal_ktrace.hpp"
#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
class tracer_management : public ::testing::Test
{
    mock_sal_ktrace ktrace;    

    void SetUp() override
    {
        ktrace.set_default_expectations();
        EXPECT_CALL(ktrace, find_debug_fs())
            .Times(::testing::AtMost(1))
            .WillOnce(::testing::Return("/some/directory"));
        EXPECT_CALL(ktrace, open(::testing::_, O_WRONLY))
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Return(13));
        EXPECT_CALL(ktrace, close(::testing::Eq(13)))
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::Return(0));
        EXPECT_CALL(ktrace, write(::testing::_, ::testing::_, ::testing::_))
            .Times(::testing::AnyNumber())
            .WillRepeatedly(::testing::ReturnArg<2>());
    }
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST_F(tracer_management, set_nullptr_to_remove_any_tracer)
{
    wago::wtrace::set_tracer(nullptr);
}

TEST_F(tracer_management, set_predefined_kernel_tracer)
{
    auto * tracer = wago::wtrace::get_kernel_tracer();
    ASSERT_NE(nullptr, tracer);
    wago::wtrace::set_tracer(tracer);
}

TEST_F(tracer_management, set_predefined_passive_kernel_tracer)
{
    auto * tracer = wago::wtrace::get_passive_kernel_tracer();
    ASSERT_NE(nullptr, tracer);
    wago::wtrace::set_tracer(tracer);
}

TEST_F(tracer_management, set_predefined_stdout_tracer)
{
    auto * tracer = wago::wtrace::get_stdout_tracer();
    ASSERT_NE(nullptr, tracer);
    wago::wtrace::set_tracer(tracer);
}

TEST_F(tracer_management, set_predefined_stderr_tracer)
{
    auto * tracer = wago::wtrace::get_stderr_tracer();
    ASSERT_NE(nullptr, tracer);
    wago::wtrace::set_tracer(tracer);
}

TEST_F(tracer_management, wc_trace_get_channel_number)
{
    unsigned const all = wago::wtrace::g_all_channel_number;
    EXPECT_EQ(all, wc_trace_get_channel_number(wc_trace_channels::all));
    EXPECT_EQ(  1, wc_trace_get_channel_number(wc_trace_channels::channel_1));
    EXPECT_EQ( 32, wc_trace_get_channel_number(wc_trace_channels::channel_32));
    EXPECT_EQ( 13, wc_trace_get_channel_number(wc_trace_channels::channel_3 + wc_trace_channels::channel_13));
}


//---- End of source file ------------------------------------------------------
