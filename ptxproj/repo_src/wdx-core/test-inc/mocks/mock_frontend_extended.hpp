//------------------------------------------------------------------------------
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
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
///  \brief    Mock service core frontend.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_FRONTEND_EXTENDED_HPP_
#define TEST_INC_MOCKS_MOCK_FRONTEND_EXTENDED_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <parameter_service_frontend_extended_i.hpp>

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::future;
using wago::wdx::parameter_service_frontend_extended_i;
using wago::wdx::parameter_response;
using wago::wdx::parameter_instance_id;
using wago::wdx::parameter_instance_path;
using wago::wdx::device_collection_response;
using wago::wdx::device_response;
using wago::wdx::device_collection_id_t;
using wago::wdx::feature_list_response;
using wago::wdx::device_path_t;
using wago::wdx::instance_list_response;
using wago::wdx::class_list_response;
using wago::wdx::method_invocation_named_response;
using wago::wdx::parameter_value;
using wago::wdx::response;
using wago::wdx::value_request;
using wago::wdx::value_path_request;
using wago::wdx::set_parameter_response;
using wago::wdx::parameter_filter;
using wago::wdx::device_id;
using wago::wdx::parameter_response_list_response;
using wago::wdx::monitoring_list_id_t;
using wago::wdx::monitoring_list_response;
using wago::wdx::monitoring_lists_response;
using wago::wdx::monitoring_list_values_response;
using wago::wdx::delete_monitoring_list_response;
using wago::wdx::enum_definition_response;
using std::string;
using std::vector;
using std::map;
using std::shared_ptr;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_frontend_extended : public parameter_service_frontend_extended_i
{
public:
    ~mock_frontend_extended() override = default;

    MOCK_METHOD1(get_parameter_definitions, future<vector<parameter_response>> (vector<parameter_instance_id> ids));
    MOCK_METHOD1(get_parameter_definitions_by_path, future<vector<parameter_response>> (vector<parameter_instance_path> paths));
    MOCK_METHOD1(get_parameters, future<vector<parameter_response>> (vector<parameter_instance_id> ids));
    MOCK_METHOD1(get_parameters_by_path, future<vector<parameter_response>> (vector<parameter_instance_path> paths));
    MOCK_METHOD0(get_all_devices, future<device_collection_response> ());
    MOCK_METHOD1(get_device, future<device_response> (device_id device));
    MOCK_METHOD1(get_subdevices, future<device_collection_response> (device_collection_id_t deviceCollectionID));
    MOCK_METHOD1(get_subdevices_by_collection_name, future<device_collection_response> (const string deviceCollectionName));
    MOCK_METHOD1(get_features, future<vector<feature_list_response>> (vector<device_path_t> devicePaths));
    MOCK_METHOD1(get_instances, future<vector<instance_list_response>> (vector<parameter_instance_path> classPaths));
    MOCK_METHOD1(get_instance_classes, future<vector<class_list_response>> (vector<parameter_instance_path> instancePaths));
    MOCK_METHOD2(invoke_method, future<method_invocation_named_response> (parameter_instance_id methodID, map<string, shared_ptr<parameter_value>> inArgs));
    MOCK_METHOD2(invoke_method_by_path, future<method_invocation_named_response> (parameter_instance_path methodPath, map<string, shared_ptr<parameter_value>> inArgs));
    MOCK_METHOD1(set_parameter_values, future<vector<set_parameter_response>> (vector<value_request>valueRequests));
    MOCK_METHOD1(set_parameter_values_by_path, future<vector<set_parameter_response>> (vector<value_path_request>valuePathRequests));
    MOCK_METHOD2(create_monitoring_list, future<monitoring_list_response> (vector<parameter_instance_id> ids, uint16_t timeout_seconds));
    MOCK_METHOD2(create_monitoring_list_with_paths, future<monitoring_list_response> (vector<parameter_instance_path> paths, uint16_t timeout_seconds));
    MOCK_METHOD1(get_values_for_monitoring_list, future<monitoring_list_values_response> (monitoring_list_id_t id));
    MOCK_METHOD1(get_monitoring_list, future<monitoring_list_response> (monitoring_list_id_t id));
    MOCK_METHOD0(get_all_monitoring_lists, future<monitoring_lists_response> ());
    MOCK_METHOD1(delete_monitoring_list, future<delete_monitoring_list_response> (monitoring_list_id_t id));
    MOCK_METHOD3(get_all_parameters, future<parameter_response_list_response> (parameter_filter filter, size_t paging_offset, size_t paging_limit));
    MOCK_METHOD2(set_parameter_values_by_path_connection_aware, future<vector<set_parameter_response>> (vector<value_path_request>valuePathRequests, bool defer_wda_web_connection_changes));
    MOCK_METHOD0(get_all_enum_definitions, future<vector<enum_definition_response>> ());
    MOCK_METHOD1(get_enum_definition, future<enum_definition_response> (std::string enum_name));
    MOCK_METHOD0(get_features_of_all_devices, future<std::vector<wago::wdx::feature_list_response>> ());
    MOCK_METHOD2(get_feature_definition, future<wago::wdx::feature_response> (device_path_t device, std::string feature_name));
    MOCK_METHOD3(get_all_parameter_definitions, wago::future<parameter_response_list_response>(parameter_filter filter, size_t paging_offset, size_t paging_limit));
    MOCK_METHOD3(get_all_method_definitions, wago::future<parameter_response_list_response>(parameter_filter filter, size_t paging_offset, size_t paging_limit));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, get_parameter_definitions(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_parameter_definitions_by_path(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_parameters(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_parameters_by_path(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_all_devices())
            .Times(0);
        EXPECT_CALL(*this, get_device(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_subdevices(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_subdevices_by_collection_name(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_features(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_instances(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_instance_classes(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, invoke_method(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, invoke_method_by_path(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, set_parameter_values(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, set_parameter_values_by_path(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, create_monitoring_list(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, create_monitoring_list_with_paths(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_values_for_monitoring_list(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_monitoring_list(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_all_monitoring_lists())
            .Times(0);
        EXPECT_CALL(*this, delete_monitoring_list(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_all_parameters(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_all_enum_definitions())
            .Times(0);
        EXPECT_CALL(*this, get_enum_definition(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_features_of_all_devices())
            .Times(0);
        EXPECT_CALL(*this, get_feature_definition(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_all_parameter_definitions(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_all_method_definitions(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_FRONTEND_EXTENDED_HPP_
//---- End of source file ------------------------------------------------------
