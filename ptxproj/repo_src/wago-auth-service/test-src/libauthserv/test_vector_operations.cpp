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
///  \brief    Test vector operations
///
///  \author   FHa : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "utils/vector_operations.hpp"

#include <gtest/gtest.h>

using wago::authserv::is_subset;
using wago::authserv::split_string;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

TEST(vector_operations, valid_subset)
{
    std::vector<std::string> superset = {"This", "is", "a", "superset", "containing", "a", "subset", "."};
    std::vector<std::string> subset = {"a", "subset"};

    EXPECT_TRUE(is_subset(subset, superset));
}
TEST(vector_operations, invalid_subset)
{
    std::vector<std::string> superset = {"This", "is", "a", "set", "without", "a", "subset", "."};
    std::vector<std::string> subset = {"not", "a", "subset"};

    EXPECT_FALSE(is_subset(subset, superset));
}

TEST(vector_operations, split_string)
{
    std::string input = "This is a string, which can be split by space or comma.";
    std::vector<std::string> expected = {"This", "is", "a", "string,", "which", "can", "be", "split", "by", "space", "or", "comma."};
    EXPECT_EQ(split_string(input, ' '), expected);
}
TEST(vector_operations, split_string_2)
{
    std::string input = "This is a string, which can be split by space or comma.";
    std::vector<std::string> expected = {"This is a string", " which can be split by space or comma."};
    EXPECT_EQ(split_string(input, ','), expected);
}
//---- End of source file ------------------------------------------------------

