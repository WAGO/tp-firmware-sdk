//------------------------------------------------------------------------------
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
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
///  \brief    Test for trace functions.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <gtest/gtest.h>

#include "wc/trace.h"

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static wc_trace_channels channel_no_23          = wc_trace_channels::channel_23;
static wc_trace_channels active_channels;
static wc_trace_channels started_channels;

static wc_trace_channels marker_channel;
static char const *      marker_name;
static size_t            marker_name_length;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
void wc_trace_activate_channel(wc_trace_channels const channel)
{
    active_channels = static_cast<wc_trace_channels>(static_cast<uint32_t>(active_channels) | static_cast<uint32_t>(channel));
}

void wc_trace_deactivate_channel(wc_trace_channels const channel)
{
    active_channels = static_cast<wc_trace_channels>(static_cast<uint32_t>(active_channels) & static_cast<uint32_t>(~channel));
}

bool wc_trace_is_active_channel(wc_trace_channels const channel)
{
    return (active_channels & channel) > 0;
}

void wc_trace_start_channel(wc_trace_channels const channel)
{
    if(wc_trace_is_active_channel(channel))
    {
        started_channels = static_cast<wc_trace_channels>(static_cast<uint32_t>(started_channels) | static_cast<uint32_t>(channel));
    }
}

void wc_trace_stop_channel(wc_trace_channels const channel)
{
    started_channels = static_cast<wc_trace_channels>(static_cast<uint32_t>(started_channels) & static_cast<uint32_t>(~channel));
}

bool wc_trace_is_started_channel(wc_trace_channels const channel)
{
    return (started_channels & channel) > 0;
}

void wc_trace_set_marker(wc_trace_channels const channel,
                         char const *      const trace_point_name,
                         size_t            const name_length)
{
    marker_channel     = channel;
    marker_name        = trace_point_name;
    marker_name_length = name_length;
}

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class Trace_Fixture : public ::testing::Test
{
protected:
    Trace_Fixture() = default;
    ~Trace_Fixture() override = default;
    void SetUp() override
    {
        active_channels    = wc_trace_channels::invalid;
        started_channels   = wc_trace_channels::invalid;

        marker_channel     = wc_trace_channels::invalid;
        marker_name        = nullptr;
        marker_name_length = 0;
    }
};

TEST_F(Trace_Fixture, NoActiveOrStarted)
{
    EXPECT_FALSE(wc_trace_is_active_channel(wc_trace_channels::all));
    EXPECT_FALSE(wc_trace_is_started_channel(wc_trace_channels::all));
}

TEST_F(Trace_Fixture, ChannelNumber)
{
    EXPECT_EQ(12,
              wc_trace_get_channel_number(wc_trace_channels::channel_12));
    EXPECT_EQ(WC_TRACE_ALL_CHANNEL_NUMBER,
              wc_trace_get_channel_number(wc_trace_channels::all));
}

TEST_F(Trace_Fixture, ActivateAndDeactivateChannel)
{
    EXPECT_FALSE(wc_trace_is_active_channel(channel_no_23));

    wc_trace_activate_channel(channel_no_23);
    EXPECT_TRUE(wc_trace_is_active_channel(channel_no_23));

    wc_trace_stop_channel(channel_no_23);
    EXPECT_TRUE(wc_trace_is_active_channel(channel_no_23));

    wc_trace_deactivate_channel(channel_no_23);
    EXPECT_FALSE(wc_trace_is_active_channel(channel_no_23));
}

TEST_F(Trace_Fixture, StartAndStopChannel)
{
    EXPECT_FALSE(wc_trace_is_started_channel(channel_no_23));

    wc_trace_start_channel(channel_no_23);
    EXPECT_FALSE(wc_trace_is_started_channel(channel_no_23));

    wc_trace_activate_channel(channel_no_23);
    wc_trace_start_channel(channel_no_23);
    EXPECT_TRUE(wc_trace_is_started_channel(channel_no_23));

    wc_trace_deactivate_channel(channel_no_23);
    EXPECT_TRUE(wc_trace_is_started_channel(channel_no_23));

    wc_trace_stop_channel(channel_no_23);
    EXPECT_FALSE(wc_trace_is_started_channel(channel_no_23));
}

TEST_F(Trace_Fixture, SetMarker)
{
    static wc_trace_channels const test_marker_channel     = channel_no_23;
    static char              const test_marker_name[]      = "Some name";
    static size_t            const test_marker_name_length = sizeof(test_marker_name) - 1;

    wc_trace_set_marker(test_marker_channel, test_marker_name, test_marker_name_length);
    EXPECT_EQ(marker_channel,     test_marker_channel);
    EXPECT_EQ(marker_name,        test_marker_name);
    EXPECT_EQ(marker_name_length, test_marker_name_length);
}

TEST_F(Trace_Fixture, SetMarkerMacro)
{
    static wc_trace_channels const test_marker_channel     = channel_no_23;
    static char              const test_marker_name[]      = "Some name";
    static size_t            const test_marker_name_length = sizeof(test_marker_name) - 1;

    WC_TRACE_SET_MARKER(test_marker_channel, test_marker_name);
    EXPECT_EQ(marker_channel,     test_marker_channel);
    EXPECT_EQ(marker_name,        test_marker_name);
    EXPECT_EQ(marker_name_length, test_marker_name_length);
}

//---- End of source file ------------------------------------------------------
