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
#include "kernel_tracer.hpp"

#include "mocks/mock_sal_ktrace.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using ::testing::Return;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class kernel_tracer_fixture : public ::testing::Test
{
public:
    mock_sal_ktrace ktrace;

    void SetUp() override
    {
        ktrace.set_default_expectations();
        EXPECT_CALL(ktrace, find_debug_fs())
            .Times(1)
            .WillOnce(Return("/some/directory"));
        EXPECT_CALL(ktrace, open(::testing::_, O_WRONLY))
            .Times(::testing::AnyNumber())
            .WillRepeatedly(Return(13));
        EXPECT_CALL(ktrace, close(::testing::Eq(13)))
            .Times(::testing::AnyNumber())
            .WillRepeatedly(Return(0));
    }
};

TEST_F(kernel_tracer_fixture, construct_delete_active)
{
    wago::wtrace::kernel_tracer tracer(false);    
}

TEST_F(kernel_tracer_fixture, construct_delete_passive)
{
    wago::wtrace::kernel_tracer tracer(true);
}

TEST_F(kernel_tracer_fixture, start_stop_when_active)
{
    wago::wtrace::kernel_tracer tracer(false);

    EXPECT_CALL(ktrace, write(::testing::_, ::testing::StrEq("1"), ::testing::Eq(1)))
        .Times(::testing::Exactly(1))
        .WillRepeatedly(::testing::ReturnArg<2>());

    tracer.start();

    EXPECT_CALL(ktrace, write(::testing::_, ::testing::StrEq("0"), ::testing::Eq(1)))
        .Times(::testing::Exactly(1))
        .WillRepeatedly(::testing::ReturnArg<2>());

    tracer.stop();
}

TEST_F(kernel_tracer_fixture, start_stop_when_passive)
{
    wago::wtrace::kernel_tracer tracer(true);
    tracer.start();
    tracer.stop();
}

//---- End of source file ------------------------------------------------------
