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
///  \brief    Test string utils.
///
///  \author   Röh: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "utils/string_util.hpp"

TEST(test_string_util, convert_to_int_ok)  
{
    std::string test_string = "-42";
    int result;
    bool conv_ok = wago::convert_to_int(test_string, result);
    EXPECT_TRUE(conv_ok);
    EXPECT_EQ(std::to_string(result), test_string);
}

TEST(test_string_util, convert_to_int_not_ok)  
{
    std::string test_string = "two";
    int result;
    bool conv_ok = wago::convert_to_int(test_string, result);
    EXPECT_FALSE(conv_ok);
}

TEST(test_string_util, convert_to_uint32_ok)  
{
    std::string test_string = "4294967295";
    uint32_t result;
    bool conv_ok = wago::convert_to_uint32(test_string, result);
    EXPECT_TRUE(conv_ok);
    EXPECT_EQ(std::to_string(result), test_string);
}

TEST(test_string_util, convert_to_uint32_string_added)  
{
    std::string test_string = "4294967295 test";
    uint32_t result;
    bool conv_ok = wago::convert_to_uint32(test_string, result);
    EXPECT_FALSE(conv_ok);
}

TEST(test_string_util, convert_to_uint32_negative_value)  
{
    std::string test_string = "-1";
    uint32_t result;
    bool conv_ok = wago::convert_to_uint32(test_string, result);
    EXPECT_FALSE(conv_ok);
}

TEST(test_string_util, convert_to_uint32_value_to_large)  
{
    std::string test_string = "4294967296";
    uint32_t result;
    bool conv_ok = wago::convert_to_uint32(test_string, result);
    EXPECT_FALSE(conv_ok);
}

TEST(test_string_util, convert_to_uint64_ok)  
{
    std::string test_string = "18446744073709551615";
    uint64_t result;
    bool conv_ok = wago::convert_to_uint64(test_string, result);
    EXPECT_TRUE(conv_ok);
    EXPECT_EQ(std::to_string(result), test_string);
}

TEST(test_string_util, convert_to_uint64_string_added)  
{
    std::string test_string = "18446744073709551615 test";
    uint64_t result;
    bool conv_ok = wago::convert_to_uint64(test_string, result);
    EXPECT_FALSE(conv_ok);
}

TEST(test_string_util, convert_to_uint64_negative_value)  
{
    std::string test_string = "-1";
    uint64_t result;
    bool conv_ok = wago::convert_to_uint64(test_string, result);
    EXPECT_FALSE(conv_ok);
}

TEST(test_string_util, convert_to_uint64_value_to_large)  
{
    std::string test_string = "18446744073709551616";
    uint64_t result;
    bool conv_ok = wago::convert_to_uint64(test_string, result);
    EXPECT_FALSE(conv_ok);
}

TEST(test_string_util, convert_to_int64_ok)  
{
    std::string test_string = "9223372036854775807";
    int64_t result;
    bool conv_ok = wago::convert_to_int64(test_string, result);
    EXPECT_TRUE(conv_ok);
    EXPECT_EQ(std::to_string(result), test_string);
}

TEST(test_string_util, convert_to_int64_string_added)  
{
    std::string test_string = "9223372036854775807 test";
    int64_t result;
    bool conv_ok = wago::convert_to_int64(test_string, result);
    EXPECT_FALSE(conv_ok);
}

TEST(test_string_util, convert_to_int64_negative_value)  
{
    std::string test_string = "-9223372036854775807";
    int64_t result;
    bool conv_ok = wago::convert_to_int64(test_string, result);
    EXPECT_TRUE(conv_ok);
    EXPECT_EQ(std::to_string(result), test_string);
}

TEST(test_string_util, convert_to_int64_value_to_large)  
{
    std::string test_string = "9223372036854775808";
    int64_t result;
    bool conv_ok = wago::convert_to_int64(test_string, result);
    EXPECT_FALSE(conv_ok);
}

TEST(test_string_util, tolower_test)
{
    std::string test_string = "WAGO";
    wago::to_lower(test_string);
    EXPECT_EQ(test_string, "wago");
}

TEST(test_string_util, tolower_copy_test)
{
    std::string test_string = "WAGO";
    std::string lower = wago::to_lower_copy(test_string);
    EXPECT_EQ(lower, "wago");
    EXPECT_EQ(test_string, "WAGO");
}

TEST(test_string_util, ltrim_test)
{
    std::string test_string = " \n\rWAGO \n\r";
    wago::ltrim(test_string);
    EXPECT_EQ(test_string, "WAGO \n\r");
}

TEST(test_string_util, rtrim_test)
{
    std::string test_string = " \n\rWAGO\r\n ";
    wago::rtrim(test_string);
    EXPECT_EQ(test_string, " \n\rWAGO");
}

TEST(test_string_util, trim_test)
{
    std::string test_string = " \n\rWAGO\r\n ";
    wago::trim(test_string);
    EXPECT_EQ(test_string, "WAGO");
}

TEST(test_string_util, split_string_ok)
{
    std::string test_string = "foo bar";
    std::vector<std::string> results = wago::split_string(test_string, ' ');
    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0], "foo");
    EXPECT_EQ(results[1], "bar");
}

TEST(test_string_util, split_string_empty_char)
{
    std::string test_string = "foo bar";
    std::vector<std::string> results = wago::split_string(test_string, '\0');
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], "foo bar");
}

TEST(test_string_util, replace)
{
    std::string test_string = "foo";
    std::string replacement_string = "bar";
    std::string replacement = replace(test_string, test_string, replacement_string);
    EXPECT_EQ(replacement, replacement_string);
}

TEST(test_string_util, random_alpha)
{
    size_t string_length = 10;
    std::string test_string = random_alpha(string_length);
    bool is_al_num = true;
    for(int i = 0; i < test_string.size() && is_al_num; i++)
    {
        is_al_num = !(std::isalnum(test_string[i]) == 0);
    }
    EXPECT_EQ(test_string.size(), string_length);
    EXPECT_TRUE(is_al_num);
}

TEST(test_string_util, starts_with_ok)
{
    std::string test_string = "WAGO";
    EXPECT_TRUE(starts_with(test_string, "WA"));
}

TEST(test_string_util, starts_with_nok)
{
    std::string test_string = "/WAGO";
    EXPECT_FALSE(starts_with(test_string, "WA"));
}
