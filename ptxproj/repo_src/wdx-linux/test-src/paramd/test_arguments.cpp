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
///  \brief    Test program argument evaluation for paramd.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "modules/paramd_args.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::paramd::arguments;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(arguments, construct_delete_parse)
{
    ::testing::internal::CaptureStderr();

    char   arg0[] = "self";
    char * argv[] = { arg0 };
    int argc = sizeof(argv) / sizeof(char *);

    arguments args(argc, argv);

    // Check output
    std::string stderr_output = ::testing::internal::GetCapturedStderr();
    EXPECT_TRUE(stderr_output.find("base path for REST-API given, using default") != std::string::npos) <<
        "stderr: \n" << stderr_output;
    EXPECT_TRUE(stderr_output.find("base path for file-API given, using default") != std::string::npos) <<
        "stderr: \n" << stderr_output;

    // Check defaults
    EXPECT_FALSE(args.get_help().empty());
    EXPECT_FALSE(args.get_rest_api_base_path().empty());
    EXPECT_FALSE(args.get_file_api_base_path().empty());
    EXPECT_FALSE(args.get_rest_api_socket().empty());
    EXPECT_FALSE(args.get_file_api_socket().empty());
    EXPECT_FALSE(args.get_backend_socket().empty());
}

TEST(arguments, no_warnings_on_help)
{
    ::testing::internal::CaptureStderr();

    char   arg0[] = "self";
    char   arg1[] = "-h";
    char * argv[] = { arg0, arg1 };
    int argc = sizeof(argv) / sizeof(char *);

    arguments args(argc, argv);

    // Check output
    std::string stderr_output = ::testing::internal::GetCapturedStderr();
    EXPECT_TRUE(stderr_output.empty()) << "stderr: \n" << stderr_output;
}

//---- End of source file ------------------------------------------------------
