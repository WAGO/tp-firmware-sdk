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
///  \brief    Test regular file provider.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/file/regular_file_provider.hpp"

#include "file_factory_override.hpp"

#include "mocks/mock_file.hpp"
#include "mocks/mock_filemanagement.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using ::testing::Exactly;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgReferee;

using wago::wdx::linuxos::file::regular_file_provider;

constexpr char const * a_file_path = "/imaginary/path/to/a/test/file";

struct regular_file_provider_fixture_params
{
    bool created_for_readonly;
    bool created_for_replace;
    bool created_for_new_file;

    // common 
    char const * file_path;
    // readonly
    bool         keep_file_open;
    // readonly and new
    bool         no_empty_file_on_disk;
    // replace and new
    uint64_t     file_size_limit;
    // new only
    mode_t       file_mode;
    char const * file_owner;

    static regular_file_provider_fixture_params for_readonly(char const * file_path,
                                                             bool         keep_file_open, 
                                                             bool         no_empty_file_on_disk)
    {
        regular_file_provider_fixture_params params = {0};
        params = { true, false, false,     // created_for_...
                 file_path,              // file_path
                 keep_file_open,         // keep_file_open
                 no_empty_file_on_disk,  // no_empty_file_on_disk
                 0ull,                   // file_size_limit
                 0, nullptr };           // file_mode & _owner
        return params;
    }

    static regular_file_provider_fixture_params for_replace(char const * file_path,
                                                            uint64_t     file_size_limit)
    {
        regular_file_provider_fixture_params params = {0};
        params = { false, true, false,    // created_for_...
                 file_path,             // file_path
                 false,                 // keep_file_open
                 false,                 // no_empty_file_on_disk
                 file_size_limit,       // file_size_limit
                 0, nullptr };          // file_mode & _owner
        return params;
    }

    static regular_file_provider_fixture_params for_new_file(char const * file_path,
                                                             bool         no_empty_file_on_disk,
                                                             uint64_t     file_size_limit,
                                                             mode_t       file_mode,
                                                             char const * file_owner)
    {
        regular_file_provider_fixture_params params = {0};
        params = { false, false, true,      // created_for_...
                 file_path,               // file_path
                 false,                   // keep_file_open
                 no_empty_file_on_disk,   // no_empty_file_on_disk
                 file_size_limit,         // file_size_limit
                 file_mode, file_owner }; // file_mode & _owner
        return params;
    }
};

class regular_file_provider_fixture : public ::testing::TestWithParam<regular_file_provider_fixture_params>
{
private:
    file_factory_override file_factory;

public:
    mock_filemanagement  file_mgmt_mock;
    mock_file           *file_mock = nullptr;

    regular_file_provider_fixture_params   test_params = GetParam();
    std::unique_ptr<regular_file_provider> test_file_provider;

    void SetUp() override
    {
        file_mgmt_mock.set_default_expectations();
        file_mock = file_factory.get_next_to_be_created_mock();
        file_mock->set_default_expectations();

        if (test_params.created_for_readonly)
        {
            test_file_provider = std::make_unique<regular_file_provider>(test_params.file_path,
                                                                         test_params.no_empty_file_on_disk,
                                                                         test_params.keep_file_open);
        }
        else if (test_params.created_for_replace)
        {
            test_file_provider = std::make_unique<regular_file_provider>(test_params.file_path,
                                                                         test_params.file_size_limit);
        }
        else if (test_params.created_for_new_file)
        {
            test_file_provider = std::make_unique<regular_file_provider>(test_params.file_path,
                                                                         test_params.file_size_limit,
                                                                         test_params.no_empty_file_on_disk,
                                                                         test_params.file_mode,
                                                                         test_params.file_owner);
        }
    }

    bool test_create(uint64_t test_capacity, bool get_file_permissions_fails = false)
    {
        // for replace mode only
        mode_t   test_mode = 123;
        uid_t    test_uid  = 456;
        gid_t    test_gid  = 789;

        if (test_capacity <= test_params.file_size_limit)
        {
            // retrieve file permissions from existing file when replacing
            if(test_params.created_for_replace)
            {
                EXPECT_CALL(file_mgmt_mock, get_file_permissions_impl(::testing::StrEq(test_params.file_path), ::testing::_, ::testing::_, ::testing::_))
                    .Times(Exactly(1))
                    .WillRepeatedly(DoAll(
                        SetArgReferee<1>(test_mode),
                        SetArgReferee<2>(test_uid),
                        SetArgReferee<3>(test_gid),
                        Return(get_file_permissions_fails ? -1 : 0)));
            }
            else
            {
                // cannot fail as it is not called
                get_file_permissions_fails = false;
            }
        }

        bool success = false;
        {
            auto provider_response = test_file_provider->create(test_capacity).get();
            EXPECT_TRUE(provider_response.is_determined());

            success = provider_response.is_success();

            
            // error for readonly files
            if (test_params.created_for_readonly)
            {
                EXPECT_TRUE(provider_response.has_error());
                EXPECT_EQ(provider_response.status, wago::wdx::status_codes::logic_error);
            }
            // error for invalid capacities
            else if (test_capacity > test_params.file_size_limit)
            {
                EXPECT_TRUE(provider_response.has_error());
                EXPECT_EQ(provider_response.status, wago::wdx::status_codes::file_size_exceeded);
            }
            // replace requires get_file_permissions to work
            else if (test_params.created_for_replace && get_file_permissions_fails)
            {
                EXPECT_TRUE(provider_response.has_error());
                EXPECT_EQ(provider_response.status, wago::wdx::status_codes::file_not_accessible);
            }
            // success otherwise
            else
            {
                EXPECT_TRUE(provider_response.is_success());
                EXPECT_EQ(provider_response.status, wago::wdx::status_codes::success);
            }
        }
        return success;
    }

    bool test_write(bool create_was_successful, uint64_t test_offset, std::vector<uint8_t> test_data, bool write_fails = false)
    {
        bool success = false;
        if (create_was_successful)
        {
            EXPECT_CALL(*file_mock, write(test_offset, ::testing::ContainerEq(test_data)))
                .Times(1)
                .WillRepeatedly(InvokeWithoutArgs([write_fails] {
                    if (write_fails)
                    {
                        throw std::exception();
                    }
                }));
        }

        {
            auto provider_response = test_file_provider->write(test_offset, test_data).get();
            EXPECT_TRUE(provider_response.is_determined());

            success = provider_response.is_success();

            // write must be successful if created has been successful and write did work as well
            if (create_was_successful && !write_fails)
            {
                EXPECT_TRUE(provider_response.is_success());
            }
            else
            {
                EXPECT_TRUE(provider_response.has_error());
            }
        }

        return success;
    }

    void test_get_file_info(bool create_was_successful, bool get_file_size_fails = false)
    {
        uint64_t test_file_size = 100;
        // get_file_size for readonly files
        if (test_params.created_for_readonly)
        {
            EXPECT_CALL(*file_mock, get_file_size())
                .Times(Exactly(1))
                .WillRepeatedly(InvokeWithoutArgs([get_file_size_fails, test_file_size] {
                    if (get_file_size_fails)
                    {
                        throw std::exception();
                    }
                    return test_file_size;
                }));
        }
        // get_write_file_size for writable files
        else if (!test_params.created_for_readonly && create_was_successful)
        {
            EXPECT_CALL(*file_mock, get_write_file_size())
                .Times(Exactly(1))
                .WillRepeatedly(InvokeWithoutArgs([get_file_size_fails, test_file_size] {
                    if (get_file_size_fails)
                    {
                        throw std::exception();
                    }
                    return test_file_size;
                }));
        }
        {
            auto provider_response = test_file_provider->get_file_info().get();
            EXPECT_TRUE(provider_response.is_determined());

            if (test_params.created_for_readonly || create_was_successful)
            {
                if (get_file_size_fails)
                {
                    EXPECT_TRUE(provider_response.has_error());
                    EXPECT_EQ(provider_response.status, wago::wdx::status_codes::file_not_accessible);
                }
                else
                {
                    EXPECT_TRUE(provider_response.is_success());
                    EXPECT_EQ(provider_response.file_size, test_file_size);
                }
            }
            else
            {
                EXPECT_TRUE(provider_response.is_success());
                EXPECT_EQ(provider_response.file_size, 0);
            }
        }
    }

    void test_is_complete(bool create_was_successful, bool write_was_successful)
    {
        // NO calls to mocks are expected to determine completion
        {
            if (test_params.created_for_readonly)
            {
                auto result = test_file_provider->is_complete().get();
                EXPECT_EQ(result, true);
            }
            else if (!create_was_successful)
            {
                EXPECT_THROW(test_file_provider->is_complete().get(), std::exception);
            }
            else
            {
                auto provider_response = test_file_provider->is_complete().get();

                if (write_was_successful)
                {
                    EXPECT_EQ(provider_response, true);
                }
                else
                {
                    EXPECT_EQ(provider_response, false);
                }
            }
        }
    }

    enum class validator_result {
        valid, fail, throws
    };

    void test_validate(bool create_was_successful, bool write_was_successful, validator_result test_validator_result, bool get_write_file_fd_fails = false, bool seek_to_offset_fails = false)
    {
        class my_exception : public std::exception
        {
            using std::exception::exception;
        };

        bool validator_called       = false;
        auto test_validator         = [&validator_called, test_validator_result](std::basic_istream<uint8_t>&)
        { 
            validator_called = true;
            if (test_validator_result == validator_result::throws)
            {
                throw my_exception();
            }
            return (test_validator_result == validator_result::valid);
        };

        if (!test_params.created_for_readonly && create_was_successful)
        {
            int test_write_fd = 123;
            EXPECT_CALL(*file_mock, get_write_file_fd())
                .Times(::testing::AtMost(1)) // 0, if capacity == 0 && no_empty_file_on_disk == true
                .WillRepeatedly(InvokeWithoutArgs([get_write_file_fd_fails, test_write_fd] {
                    if (get_write_file_fd_fails)
                    {
                        throw std::exception();
                    }
                    return std::make_shared<int>(test_write_fd);
                }));
            EXPECT_CALL(file_mgmt_mock, seek_to_offset_impl(test_write_fd, 0))
                .Times(::testing::AtMost(1)) // 0, if (capacity == 0 && no_empty_file_on_disk == true) || get_write_file_fd_fails
                .WillRepeatedly(InvokeWithoutArgs([seek_to_offset_fails] {
                    return seek_to_offset_fails ? -1 : 0;
                }));
        }

        {
            auto validation_result = test_file_provider->validate(test_validator);
            validation_result.wait();

            if (test_params.created_for_readonly)
            {
                EXPECT_FALSE(validation_result.has_exception());
                EXPECT_FALSE(validator_called);
            }
            else if (!create_was_successful)
            {
                EXPECT_TRUE(validation_result.has_exception());
                EXPECT_FALSE(validator_called);
                EXPECT_THROW(validation_result.get(), std::logic_error);
            } 
            else if (!write_was_successful)
            {
                EXPECT_TRUE(validation_result.has_exception());
                EXPECT_FALSE(validator_called);
                EXPECT_THROW(validation_result.get(), std::logic_error);
            }
            else if (get_write_file_fd_fails)
            {
                EXPECT_TRUE(validation_result.has_exception());
                EXPECT_FALSE(validator_called);
                EXPECT_THROW(validation_result.get(), std::exception);
            }
            else if (seek_to_offset_fails)
            {
                EXPECT_TRUE(validation_result.has_exception());
                EXPECT_FALSE(validator_called);
                EXPECT_THROW(validation_result.get(), std::exception);
            }
            else
            {
                if (test_validator_result == validator_result::valid)
                {
                    EXPECT_FALSE(validation_result.has_exception());
                    EXPECT_TRUE(validator_called);
                    EXPECT_NO_THROW(validation_result.get());
                }
                else if (test_validator_result == validator_result::fail)
                {
                    EXPECT_TRUE(validation_result.has_exception());
                    EXPECT_TRUE(validator_called);
                    EXPECT_THROW(validation_result.get(), std::runtime_error);
                }
                else if (test_validator_result == validator_result::throws)
                {
                    EXPECT_TRUE(validation_result.has_exception());
                    EXPECT_TRUE(validator_called);
                    EXPECT_THROW(validation_result.get(), my_exception);
                }
            }
        }
    }

    void test_finish(bool create_was_successful)
    {
        if (!test_params.created_for_readonly && create_was_successful)
        {
            EXPECT_CALL(*file_mock, store())
                .Times(Exactly(1));
        }

        {
            if (test_params.created_for_readonly || !create_was_successful)
            {
                auto finish_result = test_file_provider->finish();
                finish_result.wait();
                EXPECT_TRUE(finish_result.has_exception());
                EXPECT_THROW(finish_result.get(), std::logic_error);
            }
            else
            {
                EXPECT_NO_THROW(test_file_provider->finish().get());
            }
        }
    }
};

TEST_P(regular_file_provider_fixture, construct_delete)
{
    // Expect to get path used on construction
    EXPECT_EQ(test_params.file_path, test_file_provider->get_file_path());
}

TEST_P(regular_file_provider_fixture, create)
{
    uint64_t test_capacity = 100ull; // between 42 and 142
    test_create(test_capacity);
}

TEST_P(regular_file_provider_fixture, create_get_file_permissions_fails)
{
    uint64_t test_capacity = 100ull; // between 42 and 142
    test_create(test_capacity, true);
}

TEST_P(regular_file_provider_fixture, write)
{
    uint64_t             test_capacity = 100ull; // between 42 and 142
    std::vector<uint8_t> test_data(static_cast<size_t>(test_capacity), 0x42); // fill with 42's
    test_write(false, 0, test_data);
}

TEST_P(regular_file_provider_fixture, write_fails)
{
    uint64_t             test_capacity = 100ull; // between 42 and 142
    std::vector<uint8_t> test_data(static_cast<size_t>(test_capacity), 0x42); // fill with 42's
    test_write(false, 0, test_data, true);
}

TEST_P(regular_file_provider_fixture, write_after_create)
{
    uint64_t             test_capacity = 100ull; // between 42 and 142
    std::vector<uint8_t> test_data(static_cast<size_t>(test_capacity), 0x42); // fill with 42's
    bool create_was_successful = test_create(test_capacity);
    test_write(create_was_successful, 0, test_data);
}

TEST_P(regular_file_provider_fixture, get_file_info)
{
    test_get_file_info(false);
}

TEST_P(regular_file_provider_fixture, get_file_info_get_file_size_fails)
{
    test_get_file_info(false, true);
}

TEST_P(regular_file_provider_fixture, get_file_info_after_create)
{
    uint64_t             test_capacity = 100ull; // between 42 and 142
    std::vector<uint8_t> test_data(static_cast<size_t>(test_capacity), 0x42); // fill with 42's
    bool create_was_successful = test_create(test_capacity);
    test_get_file_info(create_was_successful);
}

TEST_P(regular_file_provider_fixture, is_complete)
{
    test_is_complete(false, false);
}

TEST_P(regular_file_provider_fixture, is_complete_after_create)
{
    uint64_t             test_capacity = 100ull; // between 42 and 142
    std::vector<uint8_t> test_data(static_cast<size_t>(test_capacity), 0x42); // fill with 42's
    bool create_was_successful = test_create(test_capacity);
    test_is_complete(create_was_successful, false);
}

TEST_P(regular_file_provider_fixture, is_complete_after_create_and_write)
{
    uint64_t             test_capacity = 100ull; // between 42 and 142
    std::vector<uint8_t> test_data(static_cast<size_t>(test_capacity), 0x42); // fill with 42's
    bool create_was_successful = test_create(test_capacity);
    bool write_was_successful  = test_write(create_was_successful, 0, test_data);
    test_is_complete(create_was_successful, write_was_successful);
}

TEST_P(regular_file_provider_fixture, validate)
{
    test_validate(false, false, validator_result::valid);
    test_validate(false, false, validator_result::fail);
    test_validate(false, false, validator_result::throws);
}

TEST_P(regular_file_provider_fixture, validate_get_write_file_fd_fails)
{
    test_validate(false, false, validator_result::valid, true);
    test_validate(false, false, validator_result::fail, true);
    test_validate(false, false, validator_result::throws, true);
}

TEST_P(regular_file_provider_fixture, validate_seek_to_offset_fails)
{
    test_validate(false, false, validator_result::valid, false, true);
    test_validate(false, false, validator_result::fail, false, true);
    test_validate(false, false, validator_result::throws, false, true);
}

TEST_P(regular_file_provider_fixture, validate_after_create)
{
    uint64_t             test_capacity = 100ull; // between 42 and 142
    std::vector<uint8_t> test_data(static_cast<size_t>(test_capacity), 0x42); // fill with 42's
    bool create_was_successful = test_create(test_capacity);
    test_validate(create_was_successful, false, validator_result::valid);
    test_validate(create_was_successful, false, validator_result::fail);
    test_validate(create_was_successful, false, validator_result::throws);
}

TEST_P(regular_file_provider_fixture, validate_after_create_and_write)
{
    uint64_t             test_capacity = 100ull; // between 42 and 142
    std::vector<uint8_t> test_data(static_cast<size_t>(test_capacity), 0x42); // fill with 42's
    bool create_was_successful = test_create(test_capacity);
    bool write_was_successful  = test_write(create_was_successful, 0, test_data);
    test_validate(create_was_successful, write_was_successful, validator_result::valid);
    test_validate(create_was_successful, write_was_successful, validator_result::fail);
    test_validate(create_was_successful, write_was_successful, validator_result::throws);
}

TEST_P(regular_file_provider_fixture, finish)
{
    test_finish(false);
}

TEST_P(regular_file_provider_fixture, finish_after_create)
{
    uint64_t             test_capacity = 100ull; // between 42 and 142
    std::vector<uint8_t> test_data(static_cast<size_t>(test_capacity), 0x42); // fill with 42's
    bool create_was_successful = test_create(test_capacity);
    test_finish(create_was_successful);
}

TEST_P(regular_file_provider_fixture, finish_after_create_and_write)
{
    uint64_t             test_capacity = 100ull; // between 42 and 142
    std::vector<uint8_t> test_data(static_cast<size_t>(test_capacity), 0x42); // fill with 42's
    bool create_was_successful = test_create(test_capacity);
    test_write(create_was_successful, 0, test_data);
    test_finish(create_was_successful);
}

INSTANTIATE_TEST_CASE_P(regular_file_provider_for_readonly, regular_file_provider_fixture,
    ::testing::Values(
        regular_file_provider_fixture_params::for_readonly( a_file_path, false, false),
        regular_file_provider_fixture_params::for_readonly( a_file_path, false, true ),
        regular_file_provider_fixture_params::for_readonly( a_file_path, true,  false),
        regular_file_provider_fixture_params::for_readonly( a_file_path, true,  true )
    ));
INSTANTIATE_TEST_CASE_P(regular_file_provider_for_replace, regular_file_provider_fixture,
    ::testing::Values(
        regular_file_provider_fixture_params::for_replace(  a_file_path, 0  ),
        regular_file_provider_fixture_params::for_replace(  a_file_path, 42 )
    ));
INSTANTIATE_TEST_CASE_P(regular_file_provider_for_new_file, regular_file_provider_fixture,
    ::testing::Values(
        regular_file_provider_fixture_params::for_new_file( a_file_path, false, 0ull,   0,   ""  ),
        regular_file_provider_fixture_params::for_new_file( a_file_path, false, 0ull,   0,   "me"),
        regular_file_provider_fixture_params::for_new_file( a_file_path, false, 0ull,   123, ""  ),
        regular_file_provider_fixture_params::for_new_file( a_file_path, false, 0ull,   123, "me"),
        regular_file_provider_fixture_params::for_new_file( a_file_path, false, 42ull,  0,   ""  ),
        regular_file_provider_fixture_params::for_new_file( a_file_path, false, 42ull,  0,   "me"), // 5
        regular_file_provider_fixture_params::for_new_file( a_file_path, false, 42ull,  123, ""  ),
        regular_file_provider_fixture_params::for_new_file( a_file_path, false, 42ull,  123, "me"),
        regular_file_provider_fixture_params::for_new_file( a_file_path, false, 142ull, 0,   ""  ),
        regular_file_provider_fixture_params::for_new_file( a_file_path, false, 142ull, 0,   "me"),
        regular_file_provider_fixture_params::for_new_file( a_file_path, false, 142ull, 123, ""  ), // 10
        regular_file_provider_fixture_params::for_new_file( a_file_path, false, 142ull, 123, "me")
    ));
INSTANTIATE_TEST_CASE_P(regular_file_provider_for_new_file_no_empty_on_disk, regular_file_provider_fixture,
    ::testing::Values(
        regular_file_provider_fixture_params::for_new_file( a_file_path, true,  0ull,   0,   ""  ),
        regular_file_provider_fixture_params::for_new_file( a_file_path, true,  0ull,   0,   "me"),
        regular_file_provider_fixture_params::for_new_file( a_file_path, true,  0ull,   123, ""  ),
        regular_file_provider_fixture_params::for_new_file( a_file_path, true,  0ull,   123, "me"),
        regular_file_provider_fixture_params::for_new_file( a_file_path, true,  42ull,  0,   ""  ),
        regular_file_provider_fixture_params::for_new_file( a_file_path, true,  42ull,  0,   "me"), // 5
        regular_file_provider_fixture_params::for_new_file( a_file_path, true,  42ull,  123, ""  ),
        regular_file_provider_fixture_params::for_new_file( a_file_path, true,  42ull,  123, "me"),
        regular_file_provider_fixture_params::for_new_file( a_file_path, true,  142ull, 0,   ""  ),
        regular_file_provider_fixture_params::for_new_file( a_file_path, true,  142ull, 0,   "me"),
        regular_file_provider_fixture_params::for_new_file( a_file_path, true,  142ull, 123, ""  ), // 10
        regular_file_provider_fixture_params::for_new_file( a_file_path, true,  142ull, 123, "me")
    ));

//---- End of source file ------------------------------------------------------
