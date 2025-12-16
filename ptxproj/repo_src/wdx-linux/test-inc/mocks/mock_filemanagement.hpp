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
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_FILEMANAGEMENT_HPP_
#define TEST_INC_MOCKS_MOCK_FILEMANAGEMENT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction_file.hpp"

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
using wago::wdx::linuxos::file::sal::filemanagement;
using wago::wdx::linuxos::file::sal::file_descriptor;

GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_filemanagement : public wago::wdx::linuxos::file::sal::filemanagement
{
private:
    filemanagement *saved_instance;

public:

    static void set_instance(filemanagement *new_instance)
    {
        instance = new_instance;
    }

    mock_filemanagement()
    {
        saved_instance = &get_instance();
        set_instance(this);
    }
    ~mock_filemanagement()
    {
        set_instance(saved_instance);
    }

    file_descriptor open_readonly(std::string const &file_path) noexcept override
    {
        return open_readonly_impl(file_path);
    }
    MOCK_METHOD1(open_readonly_impl, file_descriptor(std::string const &file_path));
    MOCK_METHOD3(create_and_open_temp_file, file_descriptor(std::string const &file_path_prefix,
                                                            std::string const &file_suffix,
                                                            std::string       &result_file_path));
    MOCK_METHOD2(delete_old_temp_files, void(std::string const &file_path_prefix,
                                             std::string const &file_suffix));
    int fchmod(file_descriptor fd,
               mode_t          mode) noexcept override
    {
        return fchmod_impl(fd, mode);
    }
    MOCK_METHOD2(fchmod_impl, file_descriptor(file_descriptor fd,
                                              mode_t          mode));
    int fchown(file_descriptor fd,
               uid_t           owner,
               gid_t           group) noexcept override
    {
        return fchown_impl(fd, owner, group);
    }
    MOCK_METHOD3(fchown_impl, file_descriptor(file_descriptor fd,
                                              uid_t           owner,
                                              gid_t           group));
    off64_t seek_to_offset(file_descriptor fd,
                           size_t          offset) noexcept override
    {
        return seek_to_offset_impl(fd, offset);
    }
    MOCK_METHOD2(seek_to_offset_impl, off64_t(file_descriptor fd,
                                              size_t          offset));
    ssize_t read(file_descriptor       fd,
                 std::vector<uint8_t> &buffer) noexcept override
    {
        return read_impl(fd, buffer);
    }
    MOCK_METHOD2(read_impl, ssize_t(file_descriptor       fd,
                                    std::vector<uint8_t> &buffer));
    ssize_t write(file_descriptor             fd,
                  std::vector<uint8_t> const &data) noexcept override
    {
        return write_impl(fd, data);
    }
    MOCK_METHOD2(write_impl, ssize_t(file_descriptor             fd,
                                     std::vector<uint8_t> const &data));
    int close(file_descriptor fd) noexcept override
    {
        return close_impl(fd);
    }
    MOCK_METHOD1(close_impl, int(file_descriptor fd));
    int fsync(file_descriptor fd) noexcept override
    {
        return fsync_impl(fd);
    }
    MOCK_METHOD1(fsync_impl, int(file_descriptor fd));
    int get_file_size(std::string const &file_path,
                      uint64_t          &file_size) noexcept override
    {
        return get_file_size_impl(file_path, file_size);
    }
    MOCK_METHOD2(get_file_size_impl, int(std::string const &file_path,
                                         uint64_t          &file_size));
    int get_file_permissions(std::string const &file_path,
                             mode_t            &mode,
                             uid_t             &uid,
                             gid_t             &gid) noexcept override
    {
        return get_file_permissions_impl(file_path, mode, uid, gid);
    }
    MOCK_METHOD4(get_file_permissions_impl, int(std::string const &file_path,
                                                mode_t            &mode,
                                                uid_t             &uid,
                                                gid_t             &gid));
    int get_file_system_space(std::string const &directory_path,
                              uint64_t          &free_size) noexcept override
    {
        return get_file_system_space_impl(directory_path, free_size);
    }
    MOCK_METHOD2(get_file_system_space_impl, int(std::string const &directory_path,
                                                 uint64_t          &free_size));
    int rename(std::string const &old_path,
               std::string const &new_path) noexcept override
    {
        return rename_impl(old_path, new_path);
    }
    MOCK_METHOD2(rename_impl, int(std::string const &old_path,
                                  std::string const &new_path));
    int unlink(std::string const &file_path) noexcept override
    {
        return unlink_impl(file_path);
    }
    MOCK_METHOD1(unlink_impl, int(std::string const &file_path));
    void get_user_info(std::string const &name,
                       uid_t             &uid,
                       gid_t             &gid) override
    {
        get_user_info_impl(name, uid, gid);
    }
    MOCK_METHOD3(get_user_info_impl, void(std::string const &name,
                                          uid_t             &uid,
                                          gid_t             &gid));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, open_readonly_impl(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, create_and_open_temp_file(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, delete_old_temp_files(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, fchmod_impl(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, fchown_impl(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, seek_to_offset_impl(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, read_impl(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, write_impl(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, close_impl(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, fsync_impl(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_file_size_impl(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_file_permissions_impl(::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_file_system_space_impl(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, rename_impl(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, unlink_impl(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_user_info_impl(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_FILEMANAGEMENT_HPP_
//---- End of source file ------------------------------------------------------
