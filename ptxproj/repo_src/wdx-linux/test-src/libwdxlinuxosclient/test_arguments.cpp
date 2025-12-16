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
///  \brief    Test program argument evaluation for libwdxlinuxosclient.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "modules/client_args.hpp"
#include "wago/wdx/linuxos/client/client_option.hpp"

#include <gtest/gtest.h>

#include <functional>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::client::arguments;
using wago::wdx::linuxos::client::log_channel;
using wago::wdx::linuxos::client::client_option;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
#define EXPECT_ERROR_STR_CONTAINS(error_string, contained_string_1, contained_string_2) \
    EXPECT_TRUE(    ((error_string).find(contained_string_1) != std::string::npos) \
                 || ((error_string).find(contained_string_2) != std::string::npos)) << "Error string: " << error_string

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(arguments, construct_delete)
{
    char cmd_name[] = "testapp";
    char * argv[] = {
        cmd_name
    };
    int argc = sizeof(argv) / sizeof(char *);

    std::string service_name = "Test Service";

    arguments args(argc, argv, service_name);
}

TEST(arguments, get_help)
{
    char cmd_name[] = "testapp";
    char * argv[] = {
        cmd_name
    };
    int argc = sizeof(argv) / sizeof(char *);

    std::string service_name = "Test Service";

    EXPECT_NE("", arguments(argc, argv, service_name).get_help());
}

static void test_accept_argument(char                                   *long_name,
                                 char                                   *short_name,
                                 char                                   *value,
                                 std::function<void(arguments &&args)> &&test_cb)
{
    char cmd_name[] = "testapp";
    char * argv[] = {
        cmd_name,
        long_name,
        value
    };
    int argc = sizeof(argv) / sizeof(char *);
    if(value == nullptr)
    {
        --argc;
    }

    std::string service_name = "Test Service";

    test_cb(arguments(argc, argv, service_name));
    argv[1] = short_name;
    test_cb(arguments(argc, argv, service_name));
}

static void test_accept_argument_combined(char                                   *short_with_value,
                                          std::function<void(arguments &&args)> &&test_cb)
{
    char cmd_name[] = "testapp";
    char * argv[] = {
        cmd_name,
        short_with_value
    };
    int argc = sizeof(argv) / sizeof(char *);

    std::string service_name = "Test Service";

    test_cb(arguments(argc, argv, service_name));
}

TEST(arguments, help)
{
    char long_name[]  = "--help";
    char short_name[] = "-h";

    test_accept_argument(long_name, short_name, nullptr, [](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_TRUE(args.is_help_requested());
    });
}

TEST(arguments, log_channel)
{
    char long_name[]  = "--log-channel";
    char short_name[] = "-c";
    char value[]      = "stdout";

    test_accept_argument(long_name, short_name, value, [](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_EQ(log_channel::std_out, args.get_log_channel());
    });
}

TEST(arguments, log_channel_combined_value)
{
    char short_name_with_value[] = "-cstderr";

    test_accept_argument_combined(short_name_with_value, [](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_EQ(log_channel::std_err, args.get_log_channel());
    });
}

TEST(arguments, error_invalid_log_channel)
{
    char long_name[]  = "--log-channel";
    char short_name[] = "-c";
    char value[]      = "somewhere";

    test_accept_argument(long_name, short_name, value, [long_name, short_name](arguments &&args) {
        EXPECT_TRUE(args.has_parse_error());
        EXPECT_ERROR_STR_CONTAINS(args.get_parse_error(), long_name[2], short_name[1]);
    });
}

TEST(arguments, log_level)
{
    char long_name[]  = "--log-level";
    char short_name[] = "-l";
    char value[]      = "debug";

    test_accept_argument(long_name, short_name, value, [](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_EQ(log_level_t::debug, args.get_log_level());
    });
}

TEST(arguments, log_level_combined_value)
{
    char short_name_with_value[] = "-linfo";

    test_accept_argument_combined(short_name_with_value, [](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_EQ(log_level_t::info, args.get_log_level());
    });
}

TEST(arguments, error_invalid_log_level)
{
    char long_name[]  = "--log-level";
    char short_name[] = "-l";
    char value[]      = "massive";

    test_accept_argument(long_name, short_name, value, [long_name, short_name](arguments &&args) {
        EXPECT_TRUE(args.has_parse_error());
        EXPECT_ERROR_STR_CONTAINS(args.get_parse_error(), long_name[2], short_name[1]);
    });
}

#ifndef SYSTEMD_INTEGRATION

TEST(arguments, desired_user)
{
    char long_name[]  = "--user";
    char short_name[] = "-u";
    char value[]      = "bob";

    test_accept_argument(long_name, short_name, value, [value](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_EQ(value, args.get_desired_user());
    });
}

TEST(arguments, desired_user_combined_value)
{
    char short_name_with_value[] = "-ualice";

    test_accept_argument_combined(short_name_with_value, [short_name_with_value](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_EQ(&short_name_with_value[2], args.get_desired_user());
    });
}

TEST(arguments, error_desired_user_without_value)
{
    char  long_name[]  = "--user";
    char  short_name[] = "-u";
    char *value        = nullptr;

    test_accept_argument(long_name, short_name, value, [long_name, short_name](arguments &&args) {
        EXPECT_TRUE(args.has_parse_error());
        EXPECT_ERROR_STR_CONTAINS(args.get_parse_error(), long_name[2], short_name[1]);
    });
}

TEST(arguments, desired_group)
{
    char long_name[]  = "--group";
    char short_name[] = "-g";
    char value[]      = "human";

    test_accept_argument(long_name, short_name, value, [](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_EQ("human", args.get_desired_group());
    });
}

TEST(arguments, desired_group_combined_value)
{
    char short_name_with_value[] = "-ggerman";

    test_accept_argument_combined(short_name_with_value, [short_name_with_value](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_EQ(&short_name_with_value[2], args.get_desired_group());
    });
}
#endif // SYSTEMD_INTEGRATION

TEST(arguments, backend_socket)
{
    char long_name[]  = "--backend-socket";
    char short_name[] = "-b";
    char value[]      = "/my/socket";

    test_accept_argument(long_name, short_name, value, [](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_EQ("/my/socket", args.get_backend_socket());
    });
}

TEST(arguments, backend_socket_combined_value)
{
    char short_name_with_value[] = "-b/test/path/to/socket";

    test_accept_argument_combined(short_name_with_value, [short_name_with_value](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_EQ(&short_name_with_value[2], args.get_backend_socket());
    });
}

TEST(arguments, desired_worker_count)
{
    char long_name[]  = "--worker-count";
    char short_name[] = "-w";
    char value[]      = "2";

    test_accept_argument(long_name, short_name, value, [](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_EQ(2, args.get_desired_worker_count());
    });
}

TEST(arguments, desired_worker_count_combined_value)
{
    char short_name_with_value[] = "-w5";

    test_accept_argument_combined(short_name_with_value, [short_name_with_value](arguments &&args) {
        EXPECT_FALSE(args.has_parse_error());
        EXPECT_EQ(5, args.get_desired_worker_count());
    });
}

TEST(arguments, additional_argument_with_upper_case_letter_as_short_option)
{
    int                argc   = 0;
    char               self[] = "self";
    char              *argv[] = {self};
    std::string const  name   = "Test";
    std::string        value;
    client_option      invalid_option = {'Y',"LongOption","Help",value};

    arguments args(argc,argv,name,{invalid_option});
}

TEST(arguments, error_invalid_worker_count_argument)
{
    char long_name[]  = "--worker-count";
    char short_name[] = "-w";
    char value[]      = "111111111111111111111111111111111111111111111111111111111111111111111111";

    test_accept_argument(long_name, short_name, value, [long_name, short_name](arguments &&args) {
        EXPECT_TRUE(args.has_parse_error());
        EXPECT_ERROR_STR_CONTAINS(args.get_parse_error(), long_name[2], short_name[1]);
    });
}

TEST(arguments, error_invalid_worker_count)
{
    char long_name[]  = "--worker-count";
    char short_name[] = "-w";
    char value[]      = "50";

    test_accept_argument(long_name, short_name, value, [long_name, short_name](arguments &&args) {
        EXPECT_TRUE(args.has_parse_error());
        EXPECT_ERROR_STR_CONTAINS(args.get_parse_error(), long_name[2], short_name[1]);
    });
}

TEST(arguments, error_invalid_worker_count_argument_text)
{
    char long_name[]  = "--worker-count";
    char short_name[] = "-w";
    char value[]      = "some text";

    test_accept_argument(long_name, short_name, value, [long_name, short_name](arguments &&args) {
        EXPECT_TRUE(args.has_parse_error());
        EXPECT_ERROR_STR_CONTAINS(args.get_parse_error(), long_name[2], short_name[1]);
    });
}

TEST(arguments, error_unknown_option)
{
    char long_name[]  = "--some-unknown-option";
    char short_name[] = "-y";
    char value[]      = "88";

    test_accept_argument(long_name, short_name, value, [long_name, short_name](arguments &&args) {
        EXPECT_TRUE(args.has_parse_error());
        EXPECT_ERROR_STR_CONTAINS(args.get_parse_error(), long_name[2], short_name[1]);
    });
}

TEST(arguments, error_unknown_option_without_value)
{
    char  long_name[]  = "--some-other-unknown-option";
    char  short_name[] = "-z";
    char *value        = nullptr;

    test_accept_argument(long_name, short_name, value, [long_name, short_name](arguments &&args) {
        EXPECT_TRUE(args.has_parse_error());
        EXPECT_ERROR_STR_CONTAINS(args.get_parse_error(), long_name[2], short_name[1]);
    });
}

TEST(arguments, error_additional_argument_with_lower_case_letter_as_short_option)
{
    int                argc   = 0;
    char               self[] = "self";
    char              *argv[] = {self};
    std::string const  name   = "Test";
    std::string        value;
    client_option      invalid_option = {'y',"LongOption","Help",value};

    EXPECT_THROW(arguments args(argc,argv,name,{invalid_option}), std::exception);
}


//---- End of source file ------------------------------------------------------
