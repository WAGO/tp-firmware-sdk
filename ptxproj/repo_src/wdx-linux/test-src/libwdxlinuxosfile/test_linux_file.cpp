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
///  \brief    Test implementation of linux_file helper class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include "linux_file.hpp"
#include "mocks/mock_filemanagement.hpp"
#include <gtest/gtest.h>
#include <unistd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using ::testing::Exactly;
using ::testing::AtMost;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgReferee;

using wago::wdx::linuxos::file::linux_file;

constexpr char const * a_file_path = "/imaginary/path/to/a/test/file";

struct linux_file_fixture_params {
    char const * file_path;
    bool         no_empty_file_on_disk;
    bool         keep_file_open; // configurable only for read_access
    bool         writable;
    // writeable only
    uint64_t     file_capacity;
    mode_t       file_mode;
    // inherit gid and uid by owner
    char const * file_owner;
    // set uid and gid directly
    uid_t        file_owner_id;
    gid_t        file_group_id;

    static linux_file_fixture_params for_read_access(char const * file_path,
                                                     bool         no_empty_file_on_disk,
                                                     bool         keep_file_open)
    {
        linux_file_fixture_params params = {0};
        params = { file_path, no_empty_file_on_disk, keep_file_open, false, 0, 0, nullptr, 123, 456 };
        return params;
    }

    static linux_file_fixture_params for_write_access(char const * file_path,
                                                      bool         no_empty_file_on_disk,
                                                      uint64_t     file_capacity,
                                                      mode_t       file_mode,
                                                      char const * file_owner)
    {
        linux_file_fixture_params params = {0};
        params = { file_path, no_empty_file_on_disk, true, true, file_capacity, file_mode, file_owner, 123, 456 };
        return params;
    }

    static linux_file_fixture_params for_write_access(char const * file_path,
                                                      bool         no_empty_file_on_disk,
                                                      uint64_t     file_capacity,
                                                      mode_t       file_mode,
                                                      uid_t        file_owner_id,
                                                      gid_t        file_group_id)
    {
        linux_file_fixture_params params = {0};
        params = { file_path, no_empty_file_on_disk, true, true, file_capacity, file_mode, nullptr, file_owner_id, file_group_id };
        return params;
    }
};
class linux_file_fixture : public ::testing::TestWithParam<linux_file_fixture_params>
{
public:
    // mocks
    mock_filemanagement file_mgmt_mock;

    // test variables
    linux_file_fixture_params test_params              = GetParam();
    file_descriptor           test_temp_fd             = 42;
    std::string               test_temp_file_path      = std::string(test_params.file_path) + "-123456.tmp";
    std::string               test_temp_file_directory = std::string(test_params.file_path).substr(
        0, std::string(test_params.file_path).find_last_of('/')
    );
    
    // create a linux file to test dependent on test_params
    inline std::unique_ptr<linux_file> create_test_object()
    {
        if (!test_params.writable)
        {
            return std::make_unique<linux_file>(test_params.file_path,
                              test_params.no_empty_file_on_disk,
                              test_params.keep_file_open);
        }
        else if (test_params.file_owner != nullptr)
        {
            return std::make_unique<linux_file>(test_params.file_path,
                              test_params.no_empty_file_on_disk,
                              test_params.file_capacity,
                              test_params.file_mode,
                              test_params.file_owner);
        }
        else
        {
            return std::make_unique<linux_file>(test_params.file_path,
                              test_params.no_empty_file_on_disk,
                              test_params.file_capacity,
                              test_params.file_mode,
                              test_params.file_owner_id,
                              test_params.file_group_id);
        }
    }

    void SetUp() override
    {
        file_mgmt_mock.set_default_expectations();
        EXPECT_CALL(file_mgmt_mock, delete_old_temp_files(::testing::_, ::testing::_))
            .Times(AtMost(1));
    }

    bool should_create_temp_file()
    {
        return test_params.writable 
            && ((test_params.file_capacity > 0) || (!test_params.no_empty_file_on_disk));
    }

    void expect_temp_file_to_be_created()
    {
        uint64_t test_file_system_space = 1000;
        int      test_fs_space_result   = 0;
        EXPECT_CALL(file_mgmt_mock, get_file_system_space_impl(::testing::StrEq(test_temp_file_directory), ::testing::_))
            .Times(Exactly(1))
            .WillRepeatedly(DoAll(
                SetArgReferee<1>(test_file_system_space),
                Return(test_fs_space_result)));

        file_descriptor test_open_result = test_temp_fd;
        EXPECT_CALL(file_mgmt_mock, create_and_open_temp_file(::testing::StrEq(test_params.file_path), ::testing::_, ::testing::_))
            .Times(Exactly(1))
            .WillRepeatedly(DoAll(
                SetArgReferee<2>(test_temp_file_path),
                Return(test_open_result)));

        if (test_params.file_owner != nullptr)
        {
            EXPECT_CALL(file_mgmt_mock, get_user_info_impl(::testing::StrEq(test_params.file_owner), ::testing::_, ::testing::_))
                .Times(Exactly(1))
                .WillRepeatedly(DoAll(
                    SetArgReferee<1>(test_params.file_owner_id),
                    SetArgReferee<2>(test_params.file_group_id)));
        }

        if (test_params.file_mode != 0)
        {
            int test_fchmod_result = 0;
            EXPECT_CALL(file_mgmt_mock, fchmod_impl(test_open_result, test_params.file_mode))
                .Times(Exactly(1))
                .WillRepeatedly(Return(test_fchmod_result));
        }
        int test_fchown_result = 0;
        EXPECT_CALL(file_mgmt_mock, fchown_impl(test_open_result, test_params.file_owner_id, test_params.file_group_id))
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_fchown_result));
    }

    void expect_temp_file_to_be_removed(bool only_close_dont_unlink = false)
    {
        int test_close_result = 0;
        EXPECT_CALL(file_mgmt_mock, close_impl(test_temp_fd))
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_close_result));

        if (!only_close_dont_unlink)
        {
            int test_unlink_result = 0;
            EXPECT_CALL(file_mgmt_mock, unlink_impl(::testing::StrEq(test_temp_file_path)))
                .Times(Exactly(1))
                .WillRepeatedly(Return(test_unlink_result));
        }
    }
};

TEST_P(linux_file_fixture, construct_delete) {
    if (should_create_temp_file())
    {
        expect_temp_file_to_be_created();
        expect_temp_file_to_be_removed();
    }

    {
        auto test_linux_file = create_test_object();
    }
}

TEST_P(linux_file_fixture, get_file_size) {
    if (should_create_temp_file())
    {
        expect_temp_file_to_be_created();
        expect_temp_file_to_be_removed();
    }

    auto   test_linux_file  = create_test_object();
    size_t test_file_size   = 42;

    // mock preparation
    int test_stat_result = 0;
    EXPECT_CALL(file_mgmt_mock, get_file_size_impl(::testing::StrEq(test_params.file_path), ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(DoAll(
            SetArgReferee<1>(test_file_size),
            Return(test_stat_result)));

    // test
    {
        EXPECT_EQ(test_file_size, test_linux_file->get_file_size());
    }
}

TEST_P(linux_file_fixture, get_file_size_throws_when_stat_fails) {
    if (should_create_temp_file())
    {
        expect_temp_file_to_be_created();
        expect_temp_file_to_be_removed();
    }

    auto   test_linux_file  = create_test_object();
    size_t test_file_size   = 0;

    // mock preparation
    int test_stat_result = -1;
    EXPECT_CALL(file_mgmt_mock, get_file_size_impl(::testing::StrEq(test_params.file_path), ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(DoAll(
            SetArgReferee<1>(test_file_size),
            Return(test_stat_result)));

    // test
    {
        EXPECT_THROW(test_linux_file->get_file_size(), std::exception);
    }
}

TEST_P(linux_file_fixture, read) {
    if (should_create_temp_file())
    {
        expect_temp_file_to_be_created();
        expect_temp_file_to_be_removed();
    }

    std::vector<uint8_t> test_data        = { 0x01, 0x02, 0x03 };
    size_t               test_file_size   = test_data.size();
    size_t               test_offset      = 1;
    size_t               test_length      = 1;
    std::vector<uint8_t> test_data_chunk(test_data.begin() + static_cast<ssize_t>(test_offset),
                                         test_data.begin() + static_cast<ssize_t>(test_offset + test_length));

    // mock preparation
    int test_stat_result = 0;
    EXPECT_CALL(file_mgmt_mock, get_file_size_impl(::testing::StrEq(test_params.file_path), ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(DoAll(
            SetArgReferee<1>(test_file_size),
            Return(test_stat_result)));

    file_descriptor test_open_result = 1337;
    EXPECT_CALL(file_mgmt_mock, open_readonly_impl(::testing::StrEq(test_params.file_path)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(test_open_result));

    EXPECT_CALL(file_mgmt_mock, seek_to_offset_impl(test_open_result, test_offset))
        .Times(Exactly(1))
        .WillRepeatedly(Return(test_offset));

    EXPECT_CALL(file_mgmt_mock, read_impl(test_open_result, ::testing::SizeIs(test_length)))
        .Times(Exactly(1))
        .WillRepeatedly(DoAll(
            SetArgReferee<1>(test_data_chunk),
            Return(test_length)));

    int test_close_result = 0;
    EXPECT_CALL(file_mgmt_mock, close_impl(test_open_result))
        .Times(Exactly(1))
        .WillRepeatedly(Return(test_close_result));

    // test
    {
        auto test_linux_file  = create_test_object();
        EXPECT_EQ(test_data_chunk, test_linux_file->read(test_offset, test_length));

        // 'close' must not be called inside read if flag is set
        if (!test_params.keep_file_open)
        {
            ::testing::Mock::VerifyAndClearExpectations(&file_mgmt_mock);
        }
    }
}

TEST_P(linux_file_fixture, read_not_existing_file) {
    if (should_create_temp_file())
    {
        expect_temp_file_to_be_created();
        expect_temp_file_to_be_removed();
    }

    auto                 test_linux_file  = create_test_object();
    size_t               test_offset      = 0;
    size_t               test_length      = 1;
    std::vector<uint8_t> test_data_chunk; // empty

    // mock preparation
    file_descriptor test_open_result = -1;
    EXPECT_CALL(file_mgmt_mock, open_readonly_impl(::testing::StrEq(test_params.file_path)))
        .Times(Exactly(1))
        .WillRepeatedly(
            InvokeWithoutArgs([test_open_result]() { 
                errno = ENOENT; 
                return test_open_result;
            }));

    // test
    if (test_params.no_empty_file_on_disk)
    {
        EXPECT_EQ(test_data_chunk, test_linux_file->read(test_offset, test_length));
    }
    else
    {
        EXPECT_THROW(test_linux_file->read(test_offset, test_length), std::exception);
    }
}


TEST_P(linux_file_fixture, write) {
    if (should_create_temp_file())
    {
        expect_temp_file_to_be_created();
        expect_temp_file_to_be_removed();
    }

    auto                 test_linux_file  = create_test_object();
    std::vector<uint8_t> test_data        = { 0x01, 0x02, 0x03 };
    size_t               test_offset      = 0;
    size_t               test_length      = test_data.size();

    // mock preparation
    if (test_params.writable && (test_params.file_capacity >= test_data.size()))
    {
        EXPECT_CALL(file_mgmt_mock, seek_to_offset_impl(test_temp_fd, test_offset))
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_offset));

        EXPECT_CALL(file_mgmt_mock, write_impl(test_temp_fd, ::testing::ContainerEq(test_data)))
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_length));
    }

    // test
    {
        // not writable at all
        if (!test_params.writable)
        {
            EXPECT_THROW(test_linux_file->write(test_offset, test_data), std::exception);
        }
        // too low capacity
        else if (test_params.file_capacity < test_data.size())
        {
            EXPECT_THROW(test_linux_file->write(test_offset, test_data), std::exception);
        }
        // write should work otherwise
        else
        {
            EXPECT_NO_THROW(test_linux_file->write(test_offset, test_data));
        }
    }
}

TEST_P(linux_file_fixture, write_and_store) {
    if (should_create_temp_file())
    {
        expect_temp_file_to_be_created();
        // unlinking the file is NOT expected, as the file is expected to be
        // moved to its final destination instead.
        expect_temp_file_to_be_removed(true);
    }

    auto                 test_linux_file  = create_test_object();
    std::vector<uint8_t> test_data        = { 0x01, 0x02, 0x03 };
    size_t               test_offset      = 0;
    size_t               test_length      = test_data.size();

    // mock preparation
    if (test_params.writable && (test_params.file_capacity >= test_data.size()))
    {
        EXPECT_CALL(file_mgmt_mock, seek_to_offset_impl(test_temp_fd, test_offset))
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_offset));

        EXPECT_CALL(file_mgmt_mock, write_impl(test_temp_fd, ::testing::ContainerEq(test_data)))
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_length));
    }
    // .store()
    if (test_params.writable)
    {
        // remove original file if empty file written and configured that way
        if (test_params.no_empty_file_on_disk && (test_params.file_capacity == 0))
        {
            int test_unlink_old_result = 0;
            EXPECT_CALL(file_mgmt_mock, unlink_impl(::testing::StrEq(test_params.file_path)))
                .Times(Exactly(1))
                .WillRepeatedly(Return(test_unlink_old_result));
        }
        // sync and rename temp file otherwise
        else
        {
            int test_fsync_result = 0;
            EXPECT_CALL(file_mgmt_mock, fsync_impl(test_temp_fd))
                .Times(Exactly(1))
                .WillRepeatedly(Return(test_fsync_result));

            int test_rename_result = 0;
            EXPECT_CALL(file_mgmt_mock, rename_impl(::testing::StrEq(test_temp_file_path.c_str()), ::testing::StrEq(test_params.file_path)))
                .Times(Exactly(1))
                .WillRepeatedly(Return(test_rename_result));
        }

        // temp dir sync in any case
        int test_open_dir_result = 23;
        EXPECT_CALL(file_mgmt_mock, open_readonly_impl(::testing::StrEq(test_temp_file_directory)))
            .Times(Exactly(1))
            .WillRepeatedly(
                InvokeWithoutArgs([test_open_dir_result]() { 
                    errno = ENOENT; 
                    return test_open_dir_result;
                }));
        int test_fsync_dir_result = 0;
        EXPECT_CALL(file_mgmt_mock, fsync_impl(test_open_dir_result))
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_fsync_dir_result));
        int test_close_dir_result = 0;
        EXPECT_CALL(file_mgmt_mock, close_impl(test_open_dir_result))
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_close_dir_result));
    }

    // test
    {
        // not writable at all -> write and store both fail
        if (!test_params.writable)
        {
            EXPECT_THROW(test_linux_file->write(test_offset, test_data), std::exception);
            EXPECT_THROW(test_linux_file->store(), std::exception);
        }
        // too low capacity -> write fails, but store doesn't (it still will 'store' an empty file)
        else if (test_params.file_capacity < test_data.size())
        {
            EXPECT_THROW(test_linux_file->write(test_offset, test_data), std::exception);
            EXPECT_NO_THROW(test_linux_file->store());
        }
        // write and store should work otherwise
        else
        {
            EXPECT_NO_THROW(test_linux_file->write(test_offset, test_data));
            EXPECT_NO_THROW(test_linux_file->store());
            ::testing::Mock::VerifyAndClearExpectations(&file_mgmt_mock);
        }
    }
}

TEST_P(linux_file_fixture, get_write_file_size) {
    if (should_create_temp_file())
    {
        expect_temp_file_to_be_created();
        expect_temp_file_to_be_removed();
    }

    auto   test_linux_file  = create_test_object();
    size_t test_length      = 3;

    // mock preparation
    if (test_params.writable && ((test_params.file_capacity > 0) || !test_params.no_empty_file_on_disk))
    {
        EXPECT_CALL(file_mgmt_mock, get_file_size_impl(test_temp_file_path, ::testing::_))
            .Times(Exactly(1))
            .WillRepeatedly(DoAll(
                SetArgReferee<1>(test_length),
                Return(0)
            ));
    }

    // test
    {
        // file is readonly
        if (!test_params.writable)
        {
            EXPECT_THROW(test_linux_file->get_write_file_size(), std::logic_error);
        }
        // file has no capacity and no_empty_file_on_disk -> cannot get file size of '0' from disk -> must be '0'
        else if (test_params.file_capacity == 0 && test_params.no_empty_file_on_disk)
        {
            EXPECT_EQ(test_linux_file->get_write_file_size(), 0);
        }
        // okay
        else
        {
            EXPECT_EQ(test_linux_file->get_write_file_size(), test_length);
        }
    }
}

TEST_P(linux_file_fixture, read_write_file) {
    if (should_create_temp_file())
    {
        expect_temp_file_to_be_created();
        expect_temp_file_to_be_removed();
    }

    auto                 test_linux_file = create_test_object();
    std::vector<uint8_t> test_data       = { 0x1, 0x2, 0x3 };
    uint64_t             test_offset     = 0;
    size_t               test_length     = test_data.size();

    // mock preparation
    if (test_params.writable && ((test_params.file_capacity > 0) || !test_params.no_empty_file_on_disk))
    {
        EXPECT_CALL(file_mgmt_mock, get_file_size_impl(test_temp_file_path, ::testing::_))
            .Times(Exactly(1))
            .WillRepeatedly(DoAll(
                SetArgReferee<1>(test_length),
                Return(0)
            ));
        EXPECT_CALL(file_mgmt_mock, seek_to_offset_impl(test_temp_fd, static_cast<size_t>(test_offset)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(test_offset));

        EXPECT_CALL(file_mgmt_mock, read_impl(test_temp_fd, ::testing::SizeIs(test_length)))
            .Times(Exactly(1))
            .WillRepeatedly(DoAll(
                SetArgReferee<1>(test_data),
                Return(test_length)
            ));
    }

    // test
    {
        // file is readonly
        if (!test_params.writable)
        {
            EXPECT_THROW(test_linux_file->read_write_file(test_offset, test_length), std::logic_error);
        }
        // file has no capacity and no_empty_file_on_disk -> cannot get file size of '0' from disk -> must be '0'
        else if (test_params.file_capacity == 0 && test_params.no_empty_file_on_disk)
        {
            EXPECT_EQ(test_linux_file->read_write_file(test_offset, test_length).size(), 0);
        }
        // okay
        else
        {
            EXPECT_EQ(test_linux_file->read_write_file(test_offset, test_length), test_data);
        }
    }
}

TEST_P(linux_file_fixture, get_write_file_fd) {
    if (should_create_temp_file())
    {
        expect_temp_file_to_be_created();
        expect_temp_file_to_be_removed();
    }

    auto test_linux_file = create_test_object();

    // test
    {
        // file is readonly
        if (!test_params.writable)
        {
            EXPECT_THROW(test_linux_file->get_write_file_fd(), std::logic_error);
        }
        // fd is null if no file can exist
        else if ((test_params.file_capacity == 0) && test_params.no_empty_file_on_disk)
        {
            EXPECT_EQ(test_linux_file->get_write_file_fd(), nullptr);
        }
        // okay
        else
        {
            ASSERT_NE(test_linux_file->get_write_file_fd(), nullptr);
            EXPECT_EQ(*test_linux_file->get_write_file_fd(), test_temp_fd);
        }
    }
}

INSTANTIATE_TEST_CASE_P(linux_file_readonly, linux_file_fixture,
    ::testing::Values(
        linux_file_fixture_params::for_read_access( a_file_path, false, false),
        linux_file_fixture_params::for_read_access( a_file_path, true,  false),
        linux_file_fixture_params::for_read_access( a_file_path, false, true ),
        linux_file_fixture_params::for_read_access( a_file_path, true,  true )
    ));

INSTANTIATE_TEST_CASE_P(linux_file_writable_with_owner, linux_file_fixture,
    ::testing::Values(
        linux_file_fixture_params::for_write_access(a_file_path, false, 0,  0,   "me"),
        linux_file_fixture_params::for_write_access(a_file_path, true,  0,  0,   "me"),
        linux_file_fixture_params::for_write_access(a_file_path, false, 42, 0,   "me"),
        linux_file_fixture_params::for_write_access(a_file_path, true,  42, 0,   "me"),
        linux_file_fixture_params::for_write_access(a_file_path, false, 0,  123, "me"),
        linux_file_fixture_params::for_write_access(a_file_path, true,  0,  123, "me"),
        linux_file_fixture_params::for_write_access(a_file_path, false, 42, 123, "me"),
        linux_file_fixture_params::for_write_access(a_file_path, true,  42, 123, "me")
    ));

INSTANTIATE_TEST_CASE_P(linux_file_writable_with_uid_and_guid, linux_file_fixture,
    ::testing::Values(
        linux_file_fixture_params::for_write_access(a_file_path, false, 0,  0,   1, 2),
        linux_file_fixture_params::for_write_access(a_file_path, true,  0,  0,   1, 2),
        linux_file_fixture_params::for_write_access(a_file_path, false, 42, 0,   1, 2),
        linux_file_fixture_params::for_write_access(a_file_path, true,  42, 0,   1, 2),
        linux_file_fixture_params::for_write_access(a_file_path, false, 0,  123, 1, 2),
        linux_file_fixture_params::for_write_access(a_file_path, true,  0,  123, 1, 2),
        linux_file_fixture_params::for_write_access(a_file_path, false, 42, 123, 1, 2),
        linux_file_fixture_params::for_write_access(a_file_path, true,  42, 123, 1, 2)
    ));

//---- End of source file ------------------------------------------------------
