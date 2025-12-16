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
///  \brief    Test program argument evaluation.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/optparsing/get_opt_parser.hpp"

#include <gtest/gtest.h>

#include <functional>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::optparsing::get_opt_parser;
using wago::optparsing::arg_option;
using wago::optparsing::arg_custom_func;

class get_opt_parser_fixture : public testing::Test
{
public:
    std::string const name;
    std::string const call_string;

    get_opt_parser parser;

    get_opt_parser_fixture()
    : name("Test name")
    , call_string("executable")
    , parser(name, call_string)
    { }

    ~get_opt_parser_fixture() override = default;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
#define EXPECT_ERROR_STR_CONTAINS(error_string, contained_string) \
    EXPECT_TRUE((error_string).find(contained_string) != std::string::npos) << "Error string: " << error_string

#define EXPECT_STR_CONTAINS(test_string, contained_string) \
    EXPECT_TRUE((test_string).find(contained_string) != std::string::npos) << "Tested string: " << test_string

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST_F(get_opt_parser_fixture, construct_delete)
{
    // Nothing to do, everything is done in fixture setup
}

TEST_F(get_opt_parser_fixture, add_simple_flag)
{
    bool flag_a;
    parser.add_flag('a', "long-a", flag_a, "Flag a");
}

TEST_F(get_opt_parser_fixture, add_counted_flag)
{
    size_t count_a;
    parser.add_flag('a', "long-a", count_a, "Flag a");
}

TEST_F(get_opt_parser_fixture, add_int64_argument)
{
    int64_t arg_a;
    parser.add_argument('a', "long-a", arg_a, "Argument a");
}

TEST_F(get_opt_parser_fixture, add_uint64_argument)
{
    uint64_t arg_a;
    parser.add_argument('a', "long-a", arg_a, "Argument a");
}

TEST_F(get_opt_parser_fixture, add_string_argument)
{
    std::string arg_a;
    parser.add_argument('a', "long-a", arg_a, "Argument a");
}

TEST_F(get_opt_parser_fixture, add_converter_func_argument)
{
    arg_custom_func converter = [](arg_option const, std::string const &){ /* Never called in this test */ EXPECT_TRUE(false); };
    parser.add_argument('a', "long-a",
                        converter,
                        "Argument a");
}

TEST_F(get_opt_parser_fixture, add_converter_func_argument_with_option_explanation)
{
    arg_custom_func converter = [](arg_option const, std::string const &){ /* Never called in this test */ EXPECT_TRUE(false); };
    parser.add_argument('a', "long-a",
                        converter,
                        "Argument a",
                        "Possible values: MyA | MyB");
}

TEST_F(get_opt_parser_fixture, add_non_char_short_option)
{
    bool flag_a;
    parser.add_flag(3, "long-a", flag_a, "Flag a");
}

TEST_F(get_opt_parser_fixture, add_empty_long_option)
{
    bool flag_a;
    parser.add_flag('a', "", flag_a, "Flag a");
}

TEST_F(get_opt_parser_fixture, basic_test_help_auto)
{
    std::string const help_title            = "The HELP title";
    std::string const help_additional_info  = "Some additional Help information:\nShow next line!";
    std::string const example_call_1        = "thistool -a 42";
    std::string const example_description_1 = "Calls this tool with parameter 'a' set to 42";
    std::string const example_call_2        = "thistool --start fast";
    std::string const example_description_2 = "Starts this tool with a fast startup option";

    parser.set_help_title(help_title);
    parser.set_help_additional_info(help_additional_info);
    parser.add_help_example(example_call_1, example_description_1);
    parser.add_help_example(example_call_2, example_description_2);

    char const short_option_a[] = "-a";
    char const long_option_a[]  = "--long-parameter-a";
    char const option_help_a[]  = "Help text for flag a";
    bool       flag_a;
    parser.add_flag(short_option_a[1], &(long_option_a[2]), flag_a, option_help_a);

    char const long_option_b[]  = "--long-parameter-b";
    char const option_help_b[]  = "Help text for flag b";
    bool       flag_b;
    parser.add_flag(2, &(long_option_b[2]), flag_b, option_help_b);

    char const short_option_c[] = "-c";
    char const option_help_c[]  = "Help text for flag c";
    bool       flag_c;
    parser.add_flag(short_option_c[1], "", flag_c, option_help_c);

    char const short_option_d[] = "-d";
    char const option_help_d[]  = "Help text for flag d";
    bool       flag_d;
    parser.add_flag(short_option_d[1], nullptr, flag_d, option_help_d); // Second time empty long option

    char const short_option_e[] = "-e";
    char const long_option_e[]  = "--long-parameter-e";
    bool       flag_e;
    parser.add_flag(short_option_e[1], &(long_option_e[2]), flag_e, nullptr); // Second time empty long option

    std::string const help_text = parser.generate_help_text();
    EXPECT_STR_CONTAINS(help_text, help_title);
    EXPECT_STR_CONTAINS(help_text, help_additional_info);
    EXPECT_STR_CONTAINS(help_text, example_call_1);
    EXPECT_STR_CONTAINS(help_text, example_description_1);
    EXPECT_STR_CONTAINS(help_text, example_call_2);
    EXPECT_STR_CONTAINS(help_text, example_description_2);
    EXPECT_STR_CONTAINS(help_text, short_option_a);
    EXPECT_STR_CONTAINS(help_text, long_option_a);
    EXPECT_STR_CONTAINS(help_text, option_help_a);
    EXPECT_STR_CONTAINS(help_text, long_option_b);
    EXPECT_STR_CONTAINS(help_text, option_help_b);
    EXPECT_STR_CONTAINS(help_text, short_option_c);
    EXPECT_STR_CONTAINS(help_text, option_help_c);
    EXPECT_STR_CONTAINS(help_text, short_option_d);
    EXPECT_STR_CONTAINS(help_text, option_help_d);
    EXPECT_STR_CONTAINS(help_text, short_option_e);
    EXPECT_STR_CONTAINS(help_text, long_option_e);
}

TEST_F(get_opt_parser_fixture, basic_test_help_requested)
{
    char       short_option_a[] = "-a";
    char const long_option_a[]  = "--long-parameter-a";
    char const option_help_a[]  = "Help text for flag a";
    bool       flag_a;
    parser.add_flag(short_option_a[1], &(long_option_a[2]), flag_a, option_help_a);

    char       short_option_b[] = "-b";
    char const long_option_b[]  = "--long-parameter-b";
    char const option_help_b[]  = "Help text for flag b";
    bool       flag_b;
    parser.add_flag(short_option_b[1], &(long_option_b[2]), flag_a, option_help_b);



    char       tool[]           = "thistool";
    char       short_option_h[] = "-h";

    char * test_call_1[]  = { tool, short_option_h };
    int    test_call_1_argc = 2;

    parser.parse(test_call_1_argc, test_call_1);
    EXPECT_TRUE(parser.is_help_requested());
}

TEST_F(get_opt_parser_fixture, basic_test_help_not_requested)
{
    char       short_option_a[] = "-a";
    char const long_option_a[]  = "--long-parameter-a";
    char const option_help_a[]  = "Help text for flag a";
    bool       flag_a;
    parser.add_flag(short_option_a[1], &(long_option_a[2]), flag_a, option_help_a);

    char       short_option_b[] = "-b";
    char const long_option_b[]  = "--long-parameter-b";
    char const option_help_b[]  = "Help text for flag b";
    bool       flag_b;
    parser.add_flag(short_option_b[1], &(long_option_b[2]), flag_a, option_help_b);


    char   tool[]           = "thistool";
    char * test_call_1[]  = { tool, short_option_a };
    int    test_call_1_argc = 2;

    parser.parse(test_call_1_argc, test_call_1);
    EXPECT_FALSE(parser.is_help_requested());
}

TEST_F(get_opt_parser_fixture, basic_test_help_requested_inbetween)
{
    char       short_option_a[] = "-a";
    char const long_option_a[]  = "--long-parameter-a";
    char const option_help_a[]  = "Help text for flag a";
    bool       flag_a;
    parser.add_flag(short_option_a[1], &(long_option_a[2]), flag_a, option_help_a);

    char       short_option_b[] = "-b";
    char const long_option_b[]  = "--long-parameter-b";
    char const option_help_b[]  = "Help text for flag b";
    bool       flag_b;
    parser.add_flag(short_option_b[1], &(long_option_b[2]), flag_a, option_help_b);


    char   tool[]           = "thistool";
    char   short_option_h[] = "-h";
    char * test_call_1[]  = { tool, short_option_a, short_option_h, short_option_b };
    int    test_call_1_argc = 4;

    parser.parse(test_call_1_argc, test_call_1);
    EXPECT_TRUE(parser.is_help_requested());
}

TEST_F(get_opt_parser_fixture, basic_parsing)
{
    char       tool[]           = "thistool";
    char       short_option_a[] = "-a";
    char       long_option_a[]  = "--long-parameter-a";
    char const option_help_a[]  = "Help text for flag a";
    bool       flag_a;
    parser.add_flag(short_option_a[1], &(long_option_a[2]), flag_a, option_help_a);

    char * test_call_1[]  = { tool, short_option_a };
    int  test_call_1_argc = 2;
    parser.parse(test_call_1_argc, test_call_1);
    EXPECT_TRUE(flag_a);

    char * test_call_2[]  = { tool };
    int  test_call_2_argc = 1;
    parser.parse(test_call_2_argc, test_call_2);
    EXPECT_FALSE(flag_a);
}

TEST_F(get_opt_parser_fixture, parsing_only_short_opt)
{
    char       tool[]           = "thistool";
    char       short_option_a[] = "-a";
    char const option_help_a[]  = "Help text for flag a";
    bool       flag_a;
    parser.add_flag(short_option_a[1], "", flag_a, option_help_a);

    char * test_call_1[]  = { tool, short_option_a };
    int  test_call_1_argc = 2;
    parser.parse(test_call_1_argc, test_call_1);
    EXPECT_TRUE(flag_a);

    char * test_call_2[]  = { tool };
    int  test_call_2_argc = 1;
    parser.parse(test_call_2_argc, test_call_2);
    EXPECT_FALSE(flag_a);
}

TEST_F(get_opt_parser_fixture, parsing_only_long_opt)
{
    char       tool[]          = "thistool";
    char       long_option_a[] = "--long-parameter-a";
    char const option_help_a[] = "Help text for flag a";
    bool       flag_a;
    parser.add_flag(326548, &(long_option_a[2]), flag_a, option_help_a);

    char * test_call_1[]  = { tool, long_option_a };
    int  test_call_1_argc = 2;
    parser.parse(test_call_1_argc, test_call_1);
    EXPECT_TRUE(flag_a);

    char * test_call_2[]  = { tool };
    int  test_call_2_argc = 1;
    parser.parse(test_call_2_argc, test_call_2);
    EXPECT_FALSE(flag_a);
}



TEST_F(get_opt_parser_fixture, error_add_reserved_zero_short_option)
{
    bool exception_thrown = false;

    bool count_z;
    try
    {
        parser.add_flag(0, "long-zero", count_z, "Flag zero");
    }
    catch(std::exception &e)
    {
        exception_thrown = true;
        std::string const error_string = e.what();
        EXPECT_ERROR_STR_CONTAINS(error_string, std::to_string(0));
        EXPECT_ERROR_STR_CONTAINS(error_string, "short option");
    }
    EXPECT_TRUE(exception_thrown);
}

TEST_F(get_opt_parser_fixture, error_add_reserved_questionmark_short_option)
{
    bool exception_thrown = false;

    bool count_q;
    try
    {
        parser.add_flag('?', "long-questionmark", count_q, "Flag ?");
    }
    catch(std::exception &e)
    {
        exception_thrown = true;
        std::string const error_string = e.what();
        EXPECT_ERROR_STR_CONTAINS(error_string, std::to_string(static_cast<unsigned>('?')));
        EXPECT_ERROR_STR_CONTAINS(error_string, "short option");
    }
    EXPECT_TRUE(exception_thrown);
}

TEST_F(get_opt_parser_fixture, error_add_space_short_option)
{
    bool exception_thrown = false;

    size_t count_w;
    try
    {
        parser.add_flag(' ', "long-space", count_w, "Flag w");
    }
    catch(std::exception &e)
    {
        exception_thrown = true;
        std::string const error_string = e.what();
        EXPECT_ERROR_STR_CONTAINS(error_string, std::to_string(static_cast<unsigned>(' ')));
        EXPECT_ERROR_STR_CONTAINS(error_string, "short option");
    }
    EXPECT_TRUE(exception_thrown);
}

TEST_F(get_opt_parser_fixture, error_add_tab_short_option)
{
    bool exception_thrown = false;

    size_t count_w;
    try
    {
        parser.add_flag('	', "long-tab", count_w, "Flag w");
    }
    catch(std::exception &e)
    {
        exception_thrown = true;
        std::string const error_string = e.what();
        EXPECT_ERROR_STR_CONTAINS(error_string, std::to_string(static_cast<unsigned>('	')));
        EXPECT_ERROR_STR_CONTAINS(error_string, "short option");
    }
    EXPECT_TRUE(exception_thrown);
}

TEST_F(get_opt_parser_fixture, error_add_space_long_option)
{
    bool exception_thrown = false;

    size_t count_w;
    try
    {
        parser.add_flag('w', "long with-space", count_w, "Flag w");
    }
    catch(std::exception &e)
    {
        exception_thrown = true;
        std::string const error_string = e.what();
        EXPECT_ERROR_STR_CONTAINS(error_string, "long with-space");
        EXPECT_ERROR_STR_CONTAINS(error_string, "long option");
    }
    EXPECT_TRUE(exception_thrown);
}

TEST_F(get_opt_parser_fixture, error_add_tab_long_option)
{
    bool exception_thrown = false;

    size_t count_w;
    try
    {
        parser.add_flag('w', "long	with-tab", count_w, "Flag w");
    }
    catch(std::exception &e)
    {
        exception_thrown = true;
        std::string const error_string = e.what();
        EXPECT_ERROR_STR_CONTAINS(error_string, "long	with-tab");
        EXPECT_ERROR_STR_CONTAINS(error_string, "long option");
    }
    EXPECT_TRUE(exception_thrown);
}

TEST_F(get_opt_parser_fixture, error_add_short_option_twice)
{
    bool exception_thrown = false;

    size_t count_x;
    parser.add_flag('x', "long-x", count_x, "Flag x");
    int64_t arg_x;
    try
    {
        parser.add_argument('x', "long-x-second", arg_x, "Argument x");
    }
    catch(std::exception &e)
    {
        exception_thrown = true;
        std::string const error_string = e.what();
        EXPECT_ERROR_STR_CONTAINS(error_string, "'x'");
        EXPECT_ERROR_STR_CONTAINS(error_string, "short option");
    }
    EXPECT_TRUE(exception_thrown);
}

TEST_F(get_opt_parser_fixture, error_add_long_option_twice)
{
    bool exception_thrown = false;

    size_t count_x;
    parser.add_flag('x', "doubled-long-opt", count_x, "Flag x");
    int64_t arg_x;
    try
    {
        parser.add_argument('y', "doubled-long-opt", arg_x, "Argument y");
    }
    catch(std::exception &e)
    {
        exception_thrown = true;
        std::string const error_string = e.what();
        EXPECT_ERROR_STR_CONTAINS(error_string, "doubled-long-opt");
        EXPECT_ERROR_STR_CONTAINS(error_string, "long option");
    }
    EXPECT_TRUE(exception_thrown);
}

TEST_F(get_opt_parser_fixture, error_add_dead_option)
{
    bool exception_thrown = false;

    size_t count_x;
    try
    {
        parser.add_flag(1234, "", count_x, "Flag x");
    }
    catch(std::exception &e)
    {
        exception_thrown = true;
        std::string const error_string = e.what();
        EXPECT_ERROR_STR_CONTAINS(error_string, "option");
        EXPECT_ERROR_STR_CONTAINS(error_string, "unusable");
        EXPECT_ERROR_STR_CONTAINS(error_string, "dead");
        EXPECT_ERROR_STR_CONTAINS(error_string, "empty");
        EXPECT_ERROR_STR_CONTAINS(error_string, "range [a-zA-Z]");
        EXPECT_ERROR_STR_CONTAINS(error_string, "1234");
    }
    EXPECT_TRUE(exception_thrown);
}


//---- End of source file ------------------------------------------------------
