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
///  \brief    Test device description loader (WDD).
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "device_description_loader.hpp"
#include "mocks/mock_filesystem.hpp"

#include <gtest/gtest.h>

#include <ios>
#include <sstream>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::linuxos::serv;
using namespace wago::wdx;
using testing::Exactly;
using testing::Return;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const wdd_extension[]                 = ".wdd.json";
static constexpr char const default_respository_path[]      = "/etc/unit test/descriptions";
static constexpr char const default_order_number[]          = "1234-5678/K000-1234";
static constexpr char const default_safe_order_number[]     = "1234-5678_K000-1234";
static constexpr char const default_firmware_version[]      = "0.1.2.3beta";
static constexpr char const default_safe_firmware_version[] = "0_1_2_3beta";
static constexpr char const default_description[]           = R"##({
                                                                       "ModelVersion": "1.0.0",
                                                                       "Features": [
                                                                           "BasicDeviceIdentity"
                                                                       ]
                                                                   })##";
static std::string    const default_base_file_path          = std::string(default_respository_path)
                                                            + "/" + default_safe_order_number
                                                            + wdd_extension;
static std::string    const default_full_file_path          = std::string(default_respository_path)
                                                            + "/" + default_safe_order_number
                                                            + "_" + default_safe_firmware_version
                                                            + wdd_extension;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class device_description_loader_fixture : public ::testing::Test
{
protected:
    mock_filesystem filesystem_mock;

protected:
    device_description_loader_fixture() = default;
    ~device_description_loader_fixture() override = default;
    void SetUp() override
    {
        // Set default call expectations
        filesystem_mock.set_default_expectations();
    }
};

TEST_F(device_description_loader_fixture, construct_delete)
{
    device_description_loader loader(default_respository_path, { device_selector::headstation() });
}

TEST_F(device_description_loader_fixture, get_devices)
{
    device_selection const devices = { device_selector::headstation() };
    device_description_loader loader(default_respository_path, devices);
    auto response = loader.get_provided_devices();
    ASSERT_FALSE(response.has_error());
    ASSERT_EQ((unsigned)status_codes::success, (unsigned)response.status);
    EXPECT_EQ(devices.size(), response.selected_devices.size());
    ASSERT_GE(devices.size(), response.selected_devices.size());
    for(size_t i = 0; i < response.selected_devices.size(); ++i)
    {
        EXPECT_EQ(response.selected_devices.at(i), devices.at(i));
    }
}

TEST_F(device_description_loader_fixture, load_model_full)
{
    EXPECT_CALL(filesystem_mock, is_file_existing(default_full_file_path))
        .Times(Exactly(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, open_stream_proxy(default_full_file_path, std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(Return(new std::stringstream(default_description)));
    device_selection const devices = { device_selector::headstation() };
    device_description_loader loader(default_respository_path, devices);
    auto future = loader.get_device_information(default_order_number, default_firmware_version);
    ASSERT_TRUE(future.ready());
    ASSERT_TRUE(future.has_value());
    auto response = future.get();
    EXPECT_FALSE(response.has_error());
    EXPECT_EQ((unsigned)status_codes::success, (unsigned)response.status);
    EXPECT_STREQ(default_description, response.content.c_str());
}

TEST_F(device_description_loader_fixture, load_model_base)
{
    EXPECT_CALL(filesystem_mock, is_file_existing(default_full_file_path))
        .Times(Exactly(1))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(filesystem_mock, is_file_existing(default_base_file_path))
        .Times(Exactly(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, open_stream_proxy(default_base_file_path, std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(Return(new std::stringstream(default_description)));
    device_selection const devices = { device_selector::headstation() };
    device_description_loader loader(default_respository_path, devices);
    auto future = loader.get_device_information(default_order_number, default_firmware_version);
    ASSERT_TRUE(future.ready());
    ASSERT_TRUE(future.has_value());
    auto response = future.get();
    EXPECT_FALSE(response.has_error());
    EXPECT_EQ((unsigned)status_codes::success, (unsigned)response.status);
    EXPECT_STREQ(default_description, response.content.c_str());
}

TEST_F(device_description_loader_fixture, load_no_model)
{
    EXPECT_CALL(filesystem_mock, is_file_existing(default_full_file_path))
        .Times(Exactly(1))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(filesystem_mock, is_file_existing(default_base_file_path))
        .Times(Exactly(1))
        .WillRepeatedly(Return(false));
    device_selection const devices = { device_selector::headstation() };
    device_description_loader loader(default_respository_path, devices);
    auto future = loader.get_device_information(default_order_number, default_firmware_version);
    ASSERT_TRUE(future.ready());
    ASSERT_TRUE(future.has_value());
    auto response = future.get();
    EXPECT_FALSE(response.has_error());
    EXPECT_EQ((unsigned)status_codes::no_error_yet, (unsigned)response.status);
}

TEST_F(device_description_loader_fixture, load_model_full_fail)
{
    std::stringstream *failstream = new std::stringstream();
    failstream->setstate(std::ios_base::failbit);
    EXPECT_CALL(filesystem_mock, is_file_existing(default_full_file_path))
        .Times(Exactly(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, open_stream_proxy(default_full_file_path, std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(Return(failstream));
    device_selection const devices = { device_selector::headstation() };
    device_description_loader loader(default_respository_path, devices);
    auto future = loader.get_device_information(default_order_number, default_firmware_version);
    ASSERT_TRUE(future.ready());
    ASSERT_TRUE(future.has_value());
    auto response = future.get();
    EXPECT_TRUE(response.has_error());
    EXPECT_EQ((unsigned)status_codes::internal_error, (unsigned)response.status);
}

TEST_F(device_description_loader_fixture, load_model_base_fail)
{
    std::stringstream *failstream = new std::stringstream();
    failstream->setstate(std::ios_base::failbit);
    EXPECT_CALL(filesystem_mock, is_file_existing(default_full_file_path))
        .Times(Exactly(1))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(filesystem_mock, is_file_existing(default_base_file_path))
        .Times(Exactly(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, open_stream_proxy(default_base_file_path, std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(Return(failstream));
    device_selection const devices = { device_selector::headstation() };
    device_description_loader loader(default_respository_path, devices);
    auto future = loader.get_device_information(default_order_number, default_firmware_version);
    ASSERT_TRUE(future.ready());
    ASSERT_TRUE(future.has_value());
    auto response = future.get();
    EXPECT_TRUE(response.has_error());
    EXPECT_EQ((unsigned)status_codes::internal_error, (unsigned)response.status);
}


//---- End of source file ------------------------------------------------------
