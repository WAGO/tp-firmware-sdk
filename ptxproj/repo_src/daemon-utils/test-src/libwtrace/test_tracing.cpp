//------------------------------------------------------------------------------
// Copyright (c) 2021-2024 WAGO GmbH & Co. KG
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
#include "mocks/mock_tracer.hpp"
#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class tracing : public ::testing::Test
{
public:
    mock_tracer tracer_mock;

    void SetUp() override
    {
        tracer_mock.set_default_expectations();
        wago::wtrace::set_tracer(&tracer_mock);
    }

    void TearDown() override
    {
        EXPECT_CALL(tracer_mock, stop())
            .Times(1);
        wago::wtrace::set_tracer(nullptr);

        wc_trace_stop_channel(wc_trace_channels::all);
        wc_trace_deactivate_channel(wc_trace_channels::all);
    }
};

TEST_F(tracing, activate_then_deactivate_channel)
{
    wc_trace_channels test_channel = wc_trace_channels::channel_10;
    wc_trace_activate_channel(test_channel);
    wc_trace_deactivate_channel(test_channel);
}

TEST_F(tracing, start_then_stop_channel)
{
    wc_trace_channels test_channel = wc_trace_channels::channel_10;
    wc_trace_start_channel(test_channel);
    wc_trace_stop_channel(test_channel);
}

TEST_F(tracing, activate_and_start_then_deactivate_and_stop_channel)
{
    wc_trace_channels test_channel = wc_trace_channels::channel_10;
    EXPECT_CALL(tracer_mock, start())
        .Times(1);
    wc_trace_activate_channel(test_channel);
    wc_trace_start_channel(test_channel);

    EXPECT_CALL(tracer_mock, stop())
            .Times(1);
    wc_trace_stop_channel(wc_trace_channels::all);
    wc_trace_deactivate_channel(wc_trace_channels::all);
}

TEST_F(tracing, trace_on_active_and_started_channel)
{
    wc_trace_channels test_channel = wc_trace_channels::channel_10;
    EXPECT_CALL(tracer_mock, start())
        .Times(1);
    wc_trace_activate_channel(test_channel);
    wc_trace_start_channel(test_channel);

    char const test_marker[] = "Test Marker!";
    EXPECT_CALL(tracer_mock, trace(::testing::Eq(test_channel), ::testing::Eq(test_marker), ::testing::Eq(::strlen(test_marker))))
        .Times(1);
    WC_TRACE_SET_MARKER(test_channel, test_marker);
}

TEST_F(tracing, trace_on_started_but_inactive_channel)
{
    wc_trace_channels test_channel       = wc_trace_channels::channel_10;
    wc_trace_channels test_channel_other = wc_trace_channels::channel_11;
    wc_trace_activate_channel(test_channel_other);
    wc_trace_start_channel(test_channel);

    char const test_marker[] = "Test Marker!";
    WC_TRACE_SET_MARKER(test_channel, test_marker);
}

TEST_F(tracing, trace_on_active_but_stopped_channel)
{
    wc_trace_channels test_channel       = wc_trace_channels::channel_10;
    wc_trace_channels test_channel_other = wc_trace_channels::channel_11;
    wc_trace_activate_channel(test_channel);
    wc_trace_start_channel(test_channel_other);

    char const test_marker[] = "Test Marker!";
    WC_TRACE_SET_MARKER(test_channel, test_marker);
}




//---- End of source file ------------------------------------------------------
