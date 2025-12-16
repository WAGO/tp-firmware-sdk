//------------------------------------------------------------------------------
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
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
///  \brief    Test config file abstraction class.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/file/config_file.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// test definitions
//------------------------------------------------------------------------------
TEST(config_file, construct_delete_readonly_default_with_empty_file_on_disk)
{
    std::string const file_path     = "/test/file/path.txt";
    bool        const no_empty_file = false;

    wago::wdx::linuxos::file::config_file some_config_file(file_path);
    EXPECT_EQ(file_path,     some_config_file.m_file_path);
    EXPECT_EQ(no_empty_file, some_config_file.m_no_empty_file_on_disk);
}

TEST(config_file, construct_delete_readonly)
{
    std::string const file_path     = "/test/file/path.txt";
    bool        const no_empty_file = true;

    wago::wdx::linuxos::file::config_file some_config_file(file_path, no_empty_file);
    EXPECT_EQ(file_path,     some_config_file.m_file_path);
    EXPECT_EQ(no_empty_file, some_config_file.m_no_empty_file_on_disk);
}

TEST(config_file, construct_delete_writable)
{
    std::string const file_path     = "/test/file/path.txt";
    bool        const no_empty_file = false;
    mode_t      const file_mode     = 0123;
    std::string const file_owner    = "testOwner";

    wago::wdx::linuxos::file::config_file some_config_file(file_path, no_empty_file, file_mode, file_owner);
    EXPECT_EQ(file_path,     some_config_file.m_file_path);
    EXPECT_EQ(no_empty_file, some_config_file.m_no_empty_file_on_disk);
    EXPECT_EQ(file_mode,     some_config_file.m_file_mode);
    EXPECT_EQ(file_owner,    some_config_file.m_file_owner);
}

// Because load and store operation implementation are trivial, no mocked tests are defined


//---- End of source file ------------------------------------------------------
