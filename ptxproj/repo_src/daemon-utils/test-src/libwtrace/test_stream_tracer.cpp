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
///  \brief    Test stream tracer.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "stream_tracer.hpp"
#include "wago/wtrace/trace.hpp"

#include <sstream>
#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(stream_tracer, construct_delete)
{
    std::stringstream out_stream;
    wago::wtrace::stream_tracer tracer(out_stream);
}

TEST(stream_tracer, write_output)
{
    wc_trace_channels const channel_1     = wc_trace_channels::channel_16;
    wc_trace_channels const channel_2     = wc_trace_channels::channel_25;
    char              const prefix_1[]    = "CHN 16";
    char              const prefix_2[]    = "CHN 25";
    char              const test_text_1[] = "test output A";
    char              const test_text_2[] = "test output B";
    char              const test_text_3[] = "test output C";

    std::stringstream out_stream;
    wago::wtrace::stream_tracer tracer(out_stream);
    tracer.trace(channel_2, test_text_1, sizeof(test_text_1) - 1);
    EXPECT_EQ(0, out_stream.rdbuf()->in_avail());
    tracer.start();
    tracer.trace(channel_1, test_text_2, sizeof(test_text_2) - 1);
    tracer.stop();
    tracer.trace(channel_2, test_text_3, sizeof(test_text_3) - 1);
    EXPECT_LT(0, out_stream.rdbuf()->in_avail());
    EXPECT_NE(std::string::npos, out_stream.rdbuf()->str().find(prefix_1));
    EXPECT_NE(std::string::npos, out_stream.rdbuf()->str().find(test_text_2));
    EXPECT_EQ(std::string::npos, out_stream.rdbuf()->str().find(prefix_2));
    EXPECT_EQ(std::string::npos, out_stream.rdbuf()->str().find(test_text_1));
    EXPECT_EQ(std::string::npos, out_stream.rdbuf()->str().find(test_text_3));
}


//---- End of source file ------------------------------------------------------
