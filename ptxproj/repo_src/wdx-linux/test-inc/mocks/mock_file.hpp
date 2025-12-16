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
///  \brief    Mock for filemanagement interface (system abstraction).
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_FILE_HPP_
#define TEST_INC_MOCKS_MOCK_FILE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_i.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
using wago::wdx::linuxos::file::file_i;
using wago::wdx::linuxos::file::file_descriptor_ptr;

GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_file : public file_i
{
public:
    MOCK_METHOD0(get_file_size, uint64_t());
    MOCK_METHOD2(read, std::vector<uint8_t>(uint64_t offset,
                                            size_t   length));
    MOCK_METHOD2(write, void(uint64_t offset,
                             std::vector<uint8_t> const &data));
    MOCK_METHOD0(get_write_file_size, uint64_t());
    MOCK_METHOD2(read_write_file, std::vector<uint8_t>(uint64_t offset,
                                                       size_t   length));
    MOCK_METHOD0(get_write_file_fd, file_descriptor_ptr());
    MOCK_METHOD0(store, void());

    void set_default_expectations()
    {
        EXPECT_CALL(*this, get_file_size())
            .Times(0);
        EXPECT_CALL(*this, read(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, write(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_write_file_size())
            .Times(0);
        EXPECT_CALL(*this, read_write_file(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_write_file_fd())
            .Times(0);
        EXPECT_CALL(*this, store())
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_FILE_HPP_
//---- End of source file ------------------------------------------------------
