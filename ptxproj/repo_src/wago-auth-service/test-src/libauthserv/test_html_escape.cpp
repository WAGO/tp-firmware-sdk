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
///  \brief    Test html escaping
///
///  \author   FHa : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "utils/html_escape.hpp"

#include <gtest/gtest.h>

using wago::authserv::html_escape;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
TEST(html_escape, no_escapable_characters)
{
    std::string input = "html save string";
    EXPECT_EQ(html_escape(input), input);
}

TEST(html_escape, all_escape_characters)
{
    std::string input = "<>&\"'";
    std::string expected = "&lt;&gt;&amp;&quot;&apos;";
    EXPECT_EQ(html_escape(input), expected);
}

TEST(html_escape, insert_line_breaks)
{
    std::string input = "line break\n"
        "artifical line break\\n";
    std::string expected = "line break<br>"
        "artifical line break<br>";
    EXPECT_EQ(html_escape(input), expected);
}

TEST(html_escape, insert_line_breaks_2)
{
    std::string input = "\n\r\n\n\r \\n\\r\\n\\n\\r";
    std::string expected = "<br><br><br> <br><br><br>";
    EXPECT_EQ(html_escape(input), expected);
}

TEST(html_escape, line_breaks_and_angle_brackets)
{
    std::string input = "<\n><\r\n><\n\r>";
    std::string expected = "&lt;<br>&gt;&lt;<br>&gt;&lt;<br>&gt;";
    EXPECT_EQ(html_escape(input), expected);
}

//---- End of source file ------------------------------------------------------

