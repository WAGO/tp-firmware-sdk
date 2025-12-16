//------------------------------------------------------------------------------
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
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
///  \brief    Test WAGO Parameter Service Core frontend compatibility.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "mocks/mock_frontend_extended.hpp"

#include <wda_ipc/representation.hpp>
#include <wc/assertion.h>

#include <stdexcept>
#include <string>
#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;
using wago::resolved_future;
using testing::Exactly;
using testing::Return;
using testing::ByMove;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class frontend_fixture : public ::testing::Test
{
protected:
    // static mocks
    mock_frontend_extended                  frontend_mock;

    // test specifica
    parameter_service_frontend_extended_i  *test_frontend = &frontend_mock;

protected:
    frontend_fixture() = default;
    ~frontend_fixture() override = default;

    void SetUp() override
    {
        // Set default call expectations
        frontend_mock.set_default_expectations();
    }
};

// Test basic interface
//######################
TEST_F(frontend_fixture, source_compatibility_construct_delete)
{
    // Noting to do, everything is done in SetUp/TearDown
}

TEST_F(frontend_fixture, source_compatibility_get_all_devices)
{
    device_collection_response expected_response;
    device_collection_response core_response = expected_response;
    EXPECT_CALL(frontend_mock, get_all_devices())
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    device_collection_response actual_response = test_frontend->get_all_devices().get();
    EXPECT_EQ(expected_response.status,         actual_response.status);
    EXPECT_EQ(expected_response.devices.size(), actual_response.devices.size());
}

TEST_F(frontend_fixture, source_compatibility_get_all_parameters)
{
    parameter_response_list_response expected_response;
    parameter_response_list_response core_response = expected_response;
    EXPECT_CALL(frontend_mock, get_all_parameters(::testing::_, 0, SIZE_MAX))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    parameter_response_list_response actual_response = test_frontend->get_all_parameters(parameter_filter()).get();
    EXPECT_EQ(expected_response.status,                 actual_response.status);
    EXPECT_EQ(expected_response.total_entries,          actual_response.total_entries);
    EXPECT_EQ(expected_response.param_responses.size(), actual_response.param_responses.size());
}

TEST_F(frontend_fixture, source_compatibility_get_parameters)
{
    std::vector<parameter_response> expected_responses = { parameter_response() };
    std::vector<parameter_response> core_responses     = expected_responses;
    EXPECT_CALL(frontend_mock, get_parameters(::testing::_))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));

    std::vector<parameter_response> actual_responses = test_frontend->get_parameters({}).get();
    EXPECT_EQ(expected_responses.size(),                            actual_responses.size());
    EXPECT_EQ(expected_responses.at(0).domain_specific_status_code, actual_responses.at(0).domain_specific_status_code);
    EXPECT_EQ(expected_responses.at(0).value,                       actual_responses.at(0).value);
    EXPECT_EQ(expected_responses.at(0).definition,                  actual_responses.at(0).definition);
    EXPECT_EQ(expected_responses.at(0).id,                          actual_responses.at(0).id);
    EXPECT_EQ(expected_responses.at(0).path,                        actual_responses.at(0).path);
    EXPECT_EQ(expected_responses.at(0).get_message(),               actual_responses.at(0).get_message());
    EXPECT_EQ(std::string(""),                                      actual_responses.at(0).get_message());
}

TEST_F(frontend_fixture, source_compatibility_get_parameters_by_path)
{
    std::vector<parameter_response> expected_responses = { parameter_response() };
    std::vector<parameter_response> core_responses     = expected_responses;
    EXPECT_CALL(frontend_mock, get_parameters_by_path(::testing::_))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));

    std::vector<parameter_response> actual_responses = test_frontend->get_parameters_by_path({}).get();
    EXPECT_EQ(expected_responses.size(), actual_responses.size());
    EXPECT_EQ(expected_responses.at(0).domain_specific_status_code, actual_responses.at(0).domain_specific_status_code);
    EXPECT_EQ(expected_responses.at(0).value,                       actual_responses.at(0).value);
    EXPECT_EQ(expected_responses.at(0).definition,                  actual_responses.at(0).definition);
    EXPECT_EQ(expected_responses.at(0).id,                          actual_responses.at(0).id);
    EXPECT_EQ(expected_responses.at(0).path,                        actual_responses.at(0).path);
    EXPECT_EQ(expected_responses.at(0).get_message(),               actual_responses.at(0).get_message());
    EXPECT_EQ(std::string(""),                                      actual_responses.at(0).get_message());
}

TEST_F(frontend_fixture, source_compatibility_invoke_method)
{
    method_invocation_named_response expected_response;
    method_invocation_named_response core_response = expected_response;
    EXPECT_CALL(frontend_mock, invoke_method(::testing::_, ::testing::_))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    parameter_instance_id                                   const test_id;
    std::map<std::string, std::shared_ptr<parameter_value>> const test_map;
    method_invocation_named_response actual_response = test_frontend->invoke_method(test_id, test_map).get();
    EXPECT_EQ(expected_response.status,                      actual_response.status);
    EXPECT_EQ(expected_response.domain_specific_status_code, actual_response.domain_specific_status_code);
    EXPECT_EQ(expected_response.out_args.size(),             actual_response.out_args.size());
    EXPECT_EQ(expected_response.get_message(),               actual_response.get_message());
    EXPECT_EQ(std::string(""),                               actual_response.get_message());
}

TEST_F(frontend_fixture, source_compatibility_invoke_method_by_path)
{
    method_invocation_named_response expected_response;
    method_invocation_named_response core_response = expected_response;
    EXPECT_CALL(frontend_mock, invoke_method_by_path(::testing::_, ::testing::_))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    parameter_instance_path                                 const test_path;
    std::map<std::string, std::shared_ptr<parameter_value>> const test_map;
    method_invocation_named_response actual_response = test_frontend->invoke_method_by_path(test_path, test_map).get();
    EXPECT_EQ(expected_response.status,                      actual_response.status);
    EXPECT_EQ(expected_response.domain_specific_status_code, actual_response.domain_specific_status_code);
    EXPECT_EQ(expected_response.out_args.size(),             actual_response.out_args.size());
    EXPECT_EQ(expected_response.get_message(),               actual_response.get_message());
    EXPECT_EQ(std::string(""),                               actual_response.get_message());
}

TEST_F(frontend_fixture, source_compatibility_set_parameter_values)
{
    std::vector<set_parameter_response> expected_responses = { set_parameter_response() };
    std::vector<set_parameter_response> core_responses     = expected_responses;
    EXPECT_CALL(frontend_mock, set_parameter_values(::testing::_))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));

    std::vector<set_parameter_response> actual_responses = test_frontend->set_parameter_values({}).get();
    EXPECT_EQ(expected_responses.size(),                            actual_responses.size());
    EXPECT_EQ(expected_responses.at(0).domain_specific_status_code, actual_responses.at(0).domain_specific_status_code);
    EXPECT_EQ(expected_responses.at(0).get_message(),               actual_responses.at(0).get_message());
    EXPECT_EQ(std::string(""),                                      actual_responses.at(0).get_message());
}

TEST_F(frontend_fixture, source_compatibility_set_parameter_values_by_path)
{
    std::vector<set_parameter_response> expected_responses = { set_parameter_response() };
    std::vector<set_parameter_response> core_responses     = expected_responses;
    EXPECT_CALL(frontend_mock, set_parameter_values_by_path(::testing::_))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));

    std::vector<set_parameter_response> actual_responses = test_frontend->set_parameter_values_by_path({}).get();
    EXPECT_EQ(expected_responses.size(),                            actual_responses.size());
    EXPECT_EQ(expected_responses.at(0).domain_specific_status_code, actual_responses.at(0).domain_specific_status_code);
    EXPECT_EQ(expected_responses.at(0).get_message(),               actual_responses.at(0).get_message());
    EXPECT_EQ(std::string(""),                                      actual_responses.at(0).get_message());
}


// Test some calls of special interest from extended interface
//#############################################################
TEST_F(frontend_fixture, source_compatibility_create_monitoring_list)
{
    monitoring_list_response expected_response = {};
    expected_response.status = status_codes::success;
    expected_response.monitoring_list.id = 42;
    expected_response.monitoring_list.one_off = true;
    expected_response.monitoring_list.timeout_seconds = 0;
    monitoring_list_response core_response = expected_response;
    EXPECT_CALL(frontend_mock, create_monitoring_list(::testing::_, ::testing::_))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    monitoring_list_response actual_response = test_frontend->create_monitoring_list({}, expected_response.monitoring_list.timeout_seconds).get();
    EXPECT_EQ(expected_response.status, actual_response.status);
    EXPECT_EQ(expected_response.monitoring_list.id, actual_response.monitoring_list.id);
    EXPECT_EQ(expected_response.monitoring_list.one_off, actual_response.monitoring_list.one_off);
    EXPECT_EQ(expected_response.monitoring_list.timeout_seconds, actual_response.monitoring_list.timeout_seconds);
}

TEST_F(frontend_fixture, source_compatibility_delete_monitoring_list)
{
    delete_monitoring_list_response expected_response;
    delete_monitoring_list_response core_response = expected_response;
    EXPECT_CALL(frontend_mock, delete_monitoring_list(::testing::_))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    delete_monitoring_list_response actual_response = test_frontend->delete_monitoring_list(42).get();
    EXPECT_EQ(expected_response.status, actual_response.status);
}

TEST_F(frontend_fixture, source_compatibility_get_values_for_monitoring_list)
{
    monitoring_list_values_response expected_response({});
    monitoring_list_values_response core_response = expected_response;
    EXPECT_CALL(frontend_mock, get_values_for_monitoring_list(::testing::_))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    monitoring_list_values_response actual_response = test_frontend->get_values_for_monitoring_list(42).get();
    EXPECT_EQ(expected_response.status, actual_response.status);
    EXPECT_EQ(expected_response.parameter_values.size(), actual_response.parameter_values.size());
}


// Test some data classes of special interest needed for frontend
//################################################################
TEST(frontend, device_id_string_validation)
{
    EXPECT_NO_THROW(wago::wda_ipc::from_string<wago::wdx::device_id>(std::to_string(UINT8_MAX) + "-0"));
    std::string const invalid_device_collections[] = {
        std::to_string(UINT8_MAX + 1ULL) + "-0",
        std::to_string(-1              ) + "-0",
        std::string(   "a"             ) + "-0",
        std::string(   "unknown"       ) + "-0"
    };
    for(auto const &invalid_id : invalid_device_collections)
    {
        EXPECT_THROW(wago::wda_ipc::from_string<wago::wdx::device_id>(invalid_id), std::runtime_error)
            << "Invalid id: " << invalid_id;
    }

    EXPECT_NO_THROW(wago::wda_ipc::from_string<wago::wdx::device_id>("0-" + std::to_string(UINT16_MAX)));
    std::string const invalid_devices[] = {
        "0-" + std::to_string(UINT16_MAX + 1ULL),
        "0-" + std::to_string(-1               ),
        "0-" + std::string(   "b"              ),
        "0-" + std::string(   "unknown"        )
    };
    for(auto const &invalid_id : invalid_devices)
    {
        EXPECT_THROW(wago::wda_ipc::from_string<wago::wdx::device_id>(invalid_id), std::runtime_error)
            << "Invalid id: " << invalid_id;
    }
}

TEST(frontend, parameter_instance_path_string_validation)
{
    EXPECT_NO_THROW(wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>("0-0-some-simple-path"));
    EXPECT_NO_THROW(wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>("0-0-some-class-instance-0-path"));
    EXPECT_NO_THROW(wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>("0-0-some-class-instance-1-path"));

    EXPECT_NO_THROW(wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(std::to_string(UINT8_MAX) + "-0-some-simple-path"));

    std::string const invalid_device_collections[] = {
        std::to_string(UINT8_MAX + 1ULL) + "-0-some-simple-path",
        std::to_string(-1              ) + "-0-some-simple-path",
        std::string(   "a"             ) + "-0-some-simple-path",
        std::string(   "unknown"       ) + "-0-some-simple-path"
    };
    for(auto const &invalid_path : invalid_device_collections)
    {
        EXPECT_THROW(wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(invalid_path), std::runtime_error)
            << "Invalid path: " << invalid_path;
    }

    EXPECT_NO_THROW(wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>("0-" + std::to_string(UINT16_MAX) +"-some-simple-path"));
    std::string const invalid_devices[] = {
        "0-" + std::to_string(UINT16_MAX + 1ULL) +"-some-simple-path",
        "0-" + std::to_string(-1               ) +"-some-simple-path",
        "0-" + std::string(   "b"              ) +"-some-simple-path",
        "0-" + std::string(   "unknown"        ) +"-some-simple-path"
    };
    for(auto const &invalid_path : invalid_devices)
    {
        EXPECT_THROW(wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(invalid_path), std::runtime_error)
            << "Invalid path: " << invalid_path;
    }

    WC_STATIC_ASSERT(DYNAMIC_PLACEHOLDER_INSTANCE_ID == UINT16_MAX);
    EXPECT_NO_THROW(wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>("0-0-some-class-instance-" + std::to_string(UINT16_MAX - 1ULL) + "-path"));
    std::string const invalid_instances[] = {
        // FIXME: Core code should detect also these cases
//        "0-0-some-class-instance-" + std::to_string(UINT16_MAX       ) + "-path",
//        "0-0-some-class-instance-" + std::to_string(UINT16_MAX + 1ULL) + "-path",
//        "0-0-some-class-instance-" + std::to_string(-1               ) + "-path",
//        "0-0-some-class-instance-" + std::string(   "c"              ) + "-path",
//        "0-0-some-class-instance-" + std::string(   "unknown"        ) + "-path"
    };
    for(auto const &invalid_path : invalid_instances)
    {
        EXPECT_THROW(wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(invalid_path), std::runtime_error)
            << "Invalid path: " << invalid_path;
    }
}

TEST(frontend, instance_id_string_validation)
{
    EXPECT_NO_THROW(wago::wda_ipc::from_string<wago::wdx::instance_id_t>("0"));

    EXPECT_NO_THROW(wago::wda_ipc::from_string<wago::wdx::instance_id_t>(std::to_string(UINT16_MAX)));
    std::string const invalid_ids[] = {
        std::to_string(UINT16_MAX + 1ULL),
        std::to_string(-1               ),
        std::string(   "a"              ),
        std::string(   "unknown"        )
    };
    for(auto const &invalid_id : invalid_ids)
    {
        EXPECT_THROW(wago::wda_ipc::from_string<wago::wdx::instance_id_t>(invalid_id), std::runtime_error)
            << "Invalid id: " << invalid_id;
    }
}

TEST(frontend, monitoring_list_id_string_validation)
{
    EXPECT_NO_THROW(wago::wda_ipc::from_string<wago::wdx::monitoring_list_id_t>("32"));

    EXPECT_NO_THROW(wago::wda_ipc::from_string<wago::wdx::monitoring_list_id_t>(std::to_string(UINT64_MAX)));
    std::string const invalid_ids[] = {
        std::to_string(UINT64_MAX) + "0", // Adding "0" because 64+ bit variables are not available, yet
        std::to_string(-1              ),
        std::string(   "a"             ),
        std::string(   "unknown"       )
    };
    for(auto const &invalid_id : invalid_ids)
    {
        EXPECT_THROW(wago::wda_ipc::from_string<wago::wdx::monitoring_list_id_t>(invalid_id), std::runtime_error)
            << "Invalid id: " << invalid_id;
    }
}


//---- End of source file ------------------------------------------------------
