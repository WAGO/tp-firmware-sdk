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
///  \brief    Mock for filesystem interface (system abstraction).
///
///  \author   FHa: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_FILESYSTEM_HPP_
#define TEST_INC_MOCKS_MOCK_FILESYSTEM_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

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
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_filesystem : public wago::authserv::sal::filesystem
{
private:
    filesystem *saved_instance;

public:
    static void set_instance(filesystem *new_instance)
    {
        instance = new_instance;
    }

    mock_filesystem()
    {
        saved_instance = &get_instance();
        set_instance(this);
    }
    ~mock_filesystem()
    {
        set_instance(saved_instance);
    }

    MOCK_METHOD(int, stat, (char        const *file,
                            struct stat       *buf), (const, noexcept));
    MOCK_METHOD(bool, is_regular_file, (std::string const &file_path), (const, noexcept));
    MOCK_METHOD(std::vector<std::string>, list_directory, (std::string const &file_path), (const));
    MOCK_METHOD(int, poll, (pollfd *fds,
                            nfds_t  nfds,
                            int     timeout), (const, noexcept));
    MOCK_METHOD(int, chown, (char const *file,
                             uid_t       owner,
                             gid_t       group), (const, noexcept));
    MOCK_METHOD(int, chown, (char const *file,
                             char const *owner,
                             char const *group), (const, noexcept));
    MOCK_METHOD(int, fcntl, (int fd,
                             int cmd,
                             int first_arg), (const, noexcept));
    MOCK_METHOD(int, close, (int fd), (const, noexcept));
    MOCK_METHOD(std::unique_ptr<std::iostream>, open_stream, (std::string             const &file_path,
                                                              std::ios_base::openmode        mode));
    MOCK_METHOD(int, rename, (char const *old_name,
                              char const *new_name), (const, noexcept));

    MOCK_METHOD(int, unlink, (char const *filename), (const, noexcept));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, stat(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, is_regular_file(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, list_directory(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, poll(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, chown(::testing::_, ::testing::A<uid_t>(), ::testing::A<gid_t>()))
            .Times(0);
        EXPECT_CALL(*this, chown(::testing::_, ::testing::A<char const *>(), ::testing::A<char const *>()))
            .Times(0);
        EXPECT_CALL(*this, fcntl(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, close(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, open_stream(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, rename(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, unlink(::testing::_))
            .Times(0);
    }

    void expect_directory_list(std::string dir_path, std::vector<std::string> file_list)
    {
        EXPECT_CALL(*this, list_directory(dir_path))
            .Times(testing::AtLeast(1))
            .WillRepeatedly(
                testing::Return(file_list)
            );
    }

    void expect_file_read(std::string file_path, std::string file_content) {
        EXPECT_CALL(*this, is_regular_file(file_path))
            .WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*this, open_stream(file_path, std::ios_base::in))
            .Times(testing::AtLeast(1))
            .WillRepeatedly(testing::WithoutArgs(testing::Invoke([file_content](){
                return std::move(std::make_unique<std::stringstream>(file_content));
        })));
    }

    void expect_file_write(std::string file_path) {
        EXPECT_CALL(*this, is_regular_file(file_path))
            .WillRepeatedly(testing::Return(true));
        EXPECT_CALL(*this, open_stream(file_path, std::ios_base::out))
            .Times(testing::AtLeast(1))
            .WillRepeatedly(testing::WithoutArgs(testing::Invoke([]() {
                return std::move(std::make_unique<std::stringstream>());
            })));
    }
};

GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_FILESYSTEM_HPP_
//---- End of source file ------------------------------------------------------
