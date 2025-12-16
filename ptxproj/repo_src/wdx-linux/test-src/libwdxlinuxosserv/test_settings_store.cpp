//------------------------------------------------------------------------------
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
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
///  \brief    Simple test for settings store component.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "settings_store.hpp"
#include "mocks/mock_filesystem.hpp"

#include <gtest/gtest.h>

#include <regex>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda;
using namespace wago::wdx::linuxos::serv;
using testing::StrEq;
using testing::Exactly;
using testing::AtMost;
using testing::Return;
using testing::DoAll;
using testing::WithArg;
using testing::Invoke;
using testing::InvokeWithoutArgs;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const default_config_path[]         = "/etc/paramd/paramd.conf";
static constexpr char const default_config_path_tmp[]     = "/etc/paramd/paramd.conf.tmp";
static constexpr char const default_config[]              = R"##(# Some test config file
                                                                 
                                                                 # next follows setting allow-unauth-scan-devices= :
                                                                 allow-unauth-scan-devices  =   	 true
                                                                )##";
static constexpr char const default_unauth_scan_devices[] = "true";

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class settings_store_fixture : public ::testing::Test
{
protected:
    mock_filesystem filesystem_mock;

protected:
    settings_store_fixture() = default;
    ~settings_store_fixture() override = default;
    void SetUp() override
    {
        // Set default call expectations
        filesystem_mock.set_default_expectations();
    }
};

class verify_string_stream : public std::stringstream
{
private:
    std::string &output_m;

public:
    verify_string_stream(std::string &output) : output_m(output) {}
    ~verify_string_stream() override { output_m = this->str(); }
};

ACTION(FailedToOpenFile)
{
    throw std::runtime_error("Failed to open mocked test file.");
}


TEST_F(settings_store_fixture, construct_delete)
{
    settings_store store;
}

TEST_F(settings_store_fixture, read_value_for_known_key)
{
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path), std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(InvokeWithoutArgs([](){ return new std::stringstream(default_config); }));

    settings_store store;
    EXPECT_EQ("true", store.get_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices));
}

TEST_F(settings_store_fixture, read_value_for_known_key_from_empty_config)
{
    constexpr char const empty_config[] = "# Only one comment";
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path), std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(InvokeWithoutArgs([&empty_config](){ return new std::stringstream(empty_config); }));

    settings_store store;
    EXPECT_EQ(default_unauth_scan_devices, store.get_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices));
}

TEST_F(settings_store_fixture, read_value_for_known_key_from_empty_value)
{
    constexpr char const empty_config[] = "allow-unauth-scan-devices = ";
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path), std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(InvokeWithoutArgs([&empty_config](){ return new std::stringstream(empty_config); }));

    settings_store store;
    EXPECT_EQ(default_unauth_scan_devices, store.get_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices));
}

TEST_F(settings_store_fixture, read_value_for_known_key_from_malformed_config)
{
    constexpr char const malformed_config[] = R"##(# Some test config file
                                                          
                                                   # next follows a setting:
                                                   allow-unauth-scan-devices = invalid
                                                  )##";
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path), std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(InvokeWithoutArgs([&malformed_config](){ return new std::stringstream(malformed_config); }));

    settings_store store;
    EXPECT_EQ(default_unauth_scan_devices, store.get_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices));
}

TEST_F(settings_store_fixture, read_value_for_known_key_with_file_open_error)
{
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path), std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(FailedToOpenFile());

    settings_store store;
    bool runtime_error_exception_thrown = false;
    try
    {
        store.get_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices);
    }
    catch(std::runtime_error const &e)
    {
        EXPECT_STRNE("", e.what());
        runtime_error_exception_thrown = true;
    }

    EXPECT_TRUE(runtime_error_exception_thrown);
}

TEST_F(settings_store_fixture, read_value_for_unknown_key)
{
    settings_store store;
    bool out_of_range_exception_thrown = false;
    try
    {
        store.get_setting("an_unknown_key");
    }
    catch(std::out_of_range const &e)
    {
        EXPECT_STRNE("", e.what());
        out_of_range_exception_thrown = true;
    }

    EXPECT_TRUE(out_of_range_exception_thrown);
}

TEST_F(settings_store_fixture, write_value_for_known_key)
{
    std::string const value_to_write = "true";
    std::string       output;
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path), std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(InvokeWithoutArgs([](){ return new std::stringstream(default_config); }));
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path_tmp), std::ios::out))
        .Times(Exactly(1))
        .WillRepeatedly(InvokeWithoutArgs([&output](){ return new verify_string_stream(output); }));
    EXPECT_CALL(filesystem_mock, rename(StrEq(default_config_path_tmp), StrEq(default_config_path)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(0));

    settings_store store;
    store.write_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices, value_to_write);
    std::regex settings_regex(  std::string("(.|\n)*\n")
                              + settings_store_i::allow_unauthenticated_requests_for_scan_devices
                              + std::string("[ \t]*=[ \t]*(true|false)\n(.|\n)*"));
    EXPECT_TRUE(std::regex_match(output, settings_regex)) << "Config file output: >>" << output << "<<";
}

TEST_F(settings_store_fixture, write_value_for_known_key_with_empty_config_file)
{
    constexpr char const empty_config[] = "# Only one comment";
    std::string    const value_to_write = "true";
    std::string          output;
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path), std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(InvokeWithoutArgs([&empty_config](){ return new std::stringstream(empty_config); }));
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path_tmp), std::ios::out))
        .Times(Exactly(1))
        .WillRepeatedly(InvokeWithoutArgs([&output](){ return new verify_string_stream(output); }));
    EXPECT_CALL(filesystem_mock, rename(StrEq(default_config_path_tmp), StrEq(default_config_path)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(0));

    settings_store store;
    store.write_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices, value_to_write);
    auto key_index = output.find(settings_store_i::allow_unauthenticated_requests_for_scan_devices);
    ASSERT_NE(std::string::npos, key_index) << "Config file: >>" << output << "<<";
    auto equality_index = output.find('=', key_index + sizeof(settings_store_i::allow_unauthenticated_requests_for_scan_devices));
    ASSERT_NE(std::string::npos, equality_index) << "Config file: >>" << output << "<<";
    EXPECT_NE(std::string::npos, output.find(value_to_write, equality_index + 1)) << "Config file: >>" << output << "<<";
}

TEST_F(settings_store_fixture, write_value_for_known_key_with_file_read_open_error)
{
    std::string const value_to_write = "true";
    std::string       output;
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path), std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(FailedToOpenFile());
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path_tmp), std::ios::out))
        .Times(AtMost(1))
        .WillRepeatedly(DoAll(WithArg<0>(Invoke([filesystem_mock = &filesystem_mock](std::string const &file_path)
                                                {
                                                    EXPECT_CALL(*filesystem_mock, unlink(StrEq(file_path)))
                                                        .Times(Exactly(1))
                                                        .WillRepeatedly(Return(0));
                                                })),
                              InvokeWithoutArgs([](){ return new std::stringstream(); })));

    settings_store store;
    bool runtime_error_exception_thrown = false;
    try
    {
        store.write_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices, value_to_write);
    }
    catch(std::runtime_error const &e)
    {
        EXPECT_STRNE("", e.what());
        runtime_error_exception_thrown = true;
    }

    EXPECT_TRUE(runtime_error_exception_thrown);
}

TEST_F(settings_store_fixture, write_value_for_known_key_with_file_write_open_error)
{
    std::string const value_to_write = "true";
    std::string       output;
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path), std::ios::in))
        .Times(AtMost(1))
        .WillRepeatedly(InvokeWithoutArgs([](){ return new std::stringstream(default_config); }));
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path_tmp), std::ios::out))
        .Times(Exactly(1))
        .WillRepeatedly(FailedToOpenFile());

    settings_store store;
    bool runtime_error_exception_thrown = false;
    try
    {
        store.write_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices, value_to_write);
    }
    catch(std::runtime_error const &e)
    {
        EXPECT_STRNE("", e.what());
        runtime_error_exception_thrown = true;
    }

    EXPECT_TRUE(runtime_error_exception_thrown);
}

TEST_F(settings_store_fixture, write_value_for_known_key_with_rename_error)
{
    std::string const value_to_write = "true";
    std::string       output;
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path), std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(InvokeWithoutArgs([](){ return new std::stringstream(default_config); }));
    EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path_tmp), std::ios::out))
        .Times(Exactly(1))
        .WillRepeatedly(InvokeWithoutArgs([&output](){ return new verify_string_stream(output); }));
    EXPECT_CALL(filesystem_mock, rename(StrEq(default_config_path_tmp), StrEq(default_config_path)))
        .Times(Exactly(1))
        .WillRepeatedly(DoAll(WithArg<0>(Invoke([filesystem_mock = &filesystem_mock](std::string const &file_path)
                                                {
                                                    EXPECT_CALL(*filesystem_mock, unlink(StrEq(file_path)))
                                                        .Times(Exactly(1))
                                                        .WillRepeatedly(Return(0));
                                                })),
                              Return(-1)));

    settings_store store;
    bool runtime_error_exception_thrown = false;
    try
    {
        store.write_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices, value_to_write);
    }
    catch(std::runtime_error const &e)
    {
        EXPECT_STRNE("", e.what());
        runtime_error_exception_thrown = true;
    }

    EXPECT_TRUE(runtime_error_exception_thrown);
}

TEST_F(settings_store_fixture, write_value_for_unknown_key)
{
    std::string const value_to_write = "true";

    settings_store store;
    bool out_of_range_exception_thrown = false;
    try
    {
        store.write_setting("an_unknown_key", value_to_write);
    }
    catch(std::out_of_range const &e)
    {
        EXPECT_STRNE("", e.what());
        out_of_range_exception_thrown = true;
    }

    EXPECT_TRUE(out_of_range_exception_thrown);
}

TEST_F(settings_store_fixture, write_invalid_value_for_known_key)
{
    std::string const value_to_write = "some_scrap";

    settings_store store;
    bool invalid_argument_exception_thrown = false;
    try
    {
        store.write_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices, value_to_write);
    }
    catch(std::invalid_argument const &e)
    {
        EXPECT_STRNE("", e.what());
        invalid_argument_exception_thrown = true;
    }

    EXPECT_TRUE(invalid_argument_exception_thrown);
}

TEST_F(settings_store_fixture, read_all_settings)
{
    std::vector<std::tuple<std::string, std::string>> keys_values = {
        { settings_store_i::allow_unauthenticated_requests_for_scan_devices, "true" },
        { settings_store_i::file_api_upload_id_timeout,                      "3600" }
    };
    for (auto const & test : keys_values)
    {
        std::string key     = std::get<0>(test);
        std::string value   = std::get<1>(test);
        EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path), std::ios::in))
            .Times(Exactly(1))
            .WillRepeatedly(InvokeWithoutArgs([](){ return new std::stringstream(default_config); }));

        settings_store store;
        EXPECT_EQ(value, store.get_setting(key));
    }
}
TEST_F(settings_store_fixture, write_all_settings)
{
    std::vector<std::tuple<std::string, std::string, std::string>> keys_values_patterns = {
        { settings_store_i::allow_unauthenticated_requests_for_scan_devices, "true", "true|false" },
        { settings_store_i::file_api_upload_id_timeout,                      "3600", "[0-9]+"     }
    };
    for (auto const & test : keys_values_patterns)
    {
        std::string key     = std::get<0>(test);
        std::string value   = std::get<1>(test);
        std::string pattern = std::get<2>(test);

        std::string output;
        EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path), std::ios::in))
            .Times(Exactly(1))
            .WillRepeatedly(InvokeWithoutArgs([](){ return new std::stringstream(default_config); }));
        EXPECT_CALL(filesystem_mock, open_stream_proxy(StrEq(default_config_path_tmp), std::ios::out))
            .Times(Exactly(1))
            .WillRepeatedly(InvokeWithoutArgs([&output](){ return new verify_string_stream(output); }));
        EXPECT_CALL(filesystem_mock, rename(StrEq(default_config_path_tmp), StrEq(default_config_path)))
            .Times(Exactly(1))
            .WillRepeatedly(Return(0));

        settings_store store;
        store.write_setting(key, value);
        std::regex settings_regex(  std::string("(.|\n)*\n")
                                + key
                                + "[ \t]*=[ \t]*(" + pattern + ")\n(.|\n)*");
        EXPECT_TRUE(std::regex_match(output, settings_regex)) << "Config file output: >>" << output << "<<";
    }
}

//---- End of source file ------------------------------------------------------
