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
///  \brief    Test device description loader.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "loader/device_description_loader.hpp"
#include "instances/device.hpp"
#include "wago/wdx/parameter_exception.hpp"
#include "common/log.hpp"

#include <gtest/gtest.h>
#include <string>
#include <memory>

//------------------------------------------------------------------------------
// constants, defines and functions
//------------------------------------------------------------------------------
#define STR(x)                                          #x
#define VALUE_STR(x)                                STR(x)

#define TEST_DEVICE_ORDERNUMBER                "0123-9876"
#define TEST_DEVICE_FW_VERSION                  "01.02.03"
#define TEST_MODEL_NAME                        "TestModel"
#define TEST_FEATURE_NAME                    "TestFeature"
#define TEST_CLASS_NAME                        "TestClass"
#define TEST_INSTANCE_PARAMETER_ID_1                   123
#define TEST_INSTANCE_PARAMETER_ID_2                   124
#define TEST_CLASS_BASE_PATH             "Test/Class/Base"
#define TEST_INSTANCE_PARAMETER_PATH_1        "TestParam1"
#define TEST_INSTANCE_PARAMETER_PATH_2        "TestParam2"

using namespace wago::wdx;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
struct device_description_loader_test_fixture : public ::testing::Test
{
    std::unique_ptr<device_model> test_device_model;
    std::unique_ptr<device> test_device;

    virtual void SetUp() override 
    {
        current_log_mode  = lax;
        test_device_model = std::make_unique<device_model>();
        test_device       = std::make_unique<device>(device_id(0, 0), TEST_DEVICE_ORDERNUMBER, TEST_DEVICE_FW_VERSION);
    }

    virtual void TearDown() override 
    {
        test_device_model.release();
        test_device.release();
    }

    void SetDefaultDeviceModel()
    {
        test_device_model->name = TEST_MODEL_NAME;
        auto test_feature = std::make_shared<feature_definition>();
        test_feature->name = TEST_FEATURE_NAME;
        test_feature->classes.emplace_back(TEST_CLASS_NAME);
        test_device_model->features.push_back(std::move(test_feature));
        auto test_class = std::make_shared<class_definition>();
        test_class->name = TEST_CLASS_NAME;
        test_class->base_id = 13;
        test_class->base_path = TEST_CLASS_BASE_PATH;
        test_class->feature_def = test_device_model->features.at(0);
        class_definition::build_instantiations_parameter(test_class);
        auto test_class_parameter_1 = std::make_shared<parameter_definition>();
        test_class_parameter_1->id = TEST_INSTANCE_PARAMETER_ID_1;
        test_class_parameter_1->path = TEST_INSTANCE_PARAMETER_PATH_1;
        test_class_parameter_1->value_type = parameter_value_types::uint32;
        test_class_parameter_1->value_rank = parameter_value_rank::scalar;
        test_class_parameter_1->only_online = false;
        test_class_parameter_1->user_setting = true;
        test_class_parameter_1->writeable = true;
        test_class_parameter_1->class_def = test_class;
        test_class->parameter_definitions.push_back(test_class_parameter_1);
        test_class->resolved_parameter_definitions.push_back(test_class_parameter_1);
        auto test_class_parameter_2 = std::make_shared<parameter_definition>();
        test_class_parameter_2->id = TEST_INSTANCE_PARAMETER_ID_2;
        test_class_parameter_2->path = TEST_INSTANCE_PARAMETER_PATH_2;
        test_class_parameter_2->value_type = parameter_value_types::uint32;
        test_class_parameter_2->value_rank = parameter_value_rank::scalar;
        test_class_parameter_2->only_online = false;
        test_class_parameter_2->user_setting = true;
        test_class_parameter_2->writeable = true;
        test_class_parameter_2->class_def = test_class;
        test_class->parameter_definitions.push_back(test_class_parameter_2);
        test_class->resolved_parameter_definitions.push_back(test_class_parameter_2);
        test_class->is_resolved = true;
        test_device_model->classes.push_back(test_class);
        test_device_model->definitions.push_back(test_class->instantiations_parameter);
        test_device_model->definitions.push_back(test_class->parameter_definitions.at(0));
    }
};

TEST_F(device_description_loader_test_fixture, load_empty_device_model)
{
    device_description_loader loader(*test_device_model, *test_device);

    ASSERT_EQ(std::string(TEST_DEVICE_ORDERNUMBER), test_device->get_order_number());
    ASSERT_EQ(std::string(TEST_DEVICE_FW_VERSION),  test_device->get_firmware_version());
}

TEST_F(device_description_loader_test_fixture, load_default_device_model)
{
    this->SetDefaultDeviceModel();

    device_description_loader loader(*test_device_model, *test_device);

    EXPECT_EQ(std::string(TEST_MODEL_NAME), test_device_model->name);
    EXPECT_NE(nullptr, test_device_model->find_feature_definition(TEST_FEATURE_NAME).get());
    EXPECT_NE(nullptr, test_device_model->find_class_definition(TEST_CLASS_NAME).get());
    EXPECT_NE(nullptr, test_device_model->find_definition(TEST_INSTANCE_PARAMETER_ID_1).get());
}

constexpr char const wdd_content_instance_description[] = \
    "{"
    "    \"ModelReference\": \"" TEST_MODEL_NAME "\","
    "    \"WDMMVersion\": \"1.0.0\","
    "    \"Features\": ["
    "        \"" TEST_FEATURE_NAME "\""
    "    ],"
    "    \"ParameterValues\": ["
    "    ],"
    "    \"Instantiations\": ["
    "        {"
    "            \"Class\": \"" TEST_CLASS_NAME "\","
    "            \"Instances\": ["
    "                {"
    "                    \"ID\": 1,"
    "                    \"ParameterValues\": ["
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_1) ","
    "                            \"Value\": 31"
    "                        },"
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_2) ","
    "                            \"Value\": 42"
    "                        }"
    "                    ]"
    "                },"
    "                {"
    "                    \"ID\": 2,"
    "                    \"ParameterValues\": ["
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_1) ","
    "                            \"Value\": 53"
    "                        },"
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_2) ","
    "                            \"Value\": 64"
    "                        }"
    "                    ]"
    "                }"
    "            ]"
    "        }"
    "    ]"
    "}"
    ;

TEST_F(device_description_loader_test_fixture, load_default_device_model_and_instance_description)
{
    this->SetDefaultDeviceModel();

    device_description_loader loader(*test_device_model, *test_device);
    std::string wdd_content_string(wdd_content_instance_description);
    loader.load(wdd_content_string);

    EXPECT_EQ(2,                            test_device_model->find_class_definition(TEST_CLASS_NAME)->resolved_parameter_definitions.size());
    EXPECT_EQ(TEST_INSTANCE_PARAMETER_ID_1, test_device->class_instantiations.at(0).collected_classes.resolved_parameter_definitions.at(0)->id);
    EXPECT_EQ(TEST_INSTANCE_PARAMETER_ID_1, test_device->class_instantiations.at(1).collected_classes.resolved_parameter_definitions.at(0)->id);
    EXPECT_EQ(TEST_INSTANCE_PARAMETER_ID_2, test_device->class_instantiations.at(0).collected_classes.resolved_parameter_definitions.at(1)->id);
    EXPECT_EQ(TEST_INSTANCE_PARAMETER_ID_2, test_device->class_instantiations.at(1).collected_classes.resolved_parameter_definitions.at(1)->id);

    auto instance_no1_param1 =     test_device->parameter_instances.get_instance(parameter_instance_id(TEST_INSTANCE_PARAMETER_ID_1,1));
    auto instance_no2_param1 =     test_device->parameter_instances.get_instance(parameter_instance_id(TEST_INSTANCE_PARAMETER_ID_1,2));
    EXPECT_EQ(instance_no1_param1, test_device->parameter_instances.get_instance(path(TEST_CLASS_BASE_PATH "/" "1" "/" TEST_INSTANCE_PARAMETER_PATH_1)));
    EXPECT_EQ(instance_no2_param1, test_device->parameter_instances.get_instance(path(TEST_CLASS_BASE_PATH "/" "2" "/" TEST_INSTANCE_PARAMETER_PATH_1)));

    auto instance_no1_param2 =     test_device->parameter_instances.get_instance(parameter_instance_id(TEST_INSTANCE_PARAMETER_ID_2,1));
    auto instance_no2_param2 =     test_device->parameter_instances.get_instance(parameter_instance_id(TEST_INSTANCE_PARAMETER_ID_2,2));
    EXPECT_EQ(instance_no1_param2, test_device->parameter_instances.get_instance(path(TEST_CLASS_BASE_PATH "/" "1" "/" TEST_INSTANCE_PARAMETER_PATH_2)));
    EXPECT_EQ(instance_no2_param2, test_device->parameter_instances.get_instance(path(TEST_CLASS_BASE_PATH "/" "2" "/" TEST_INSTANCE_PARAMETER_PATH_2)));

    // Parameter instances for device: class instance count * class parameter count + class instantiations parameter
    ASSERT_EQ(2*2 + 1, test_device->parameter_instances.get_all().size());
    ASSERT_NE(nullptr, instance_no1_param1);
    ASSERT_NE(nullptr, instance_no2_param1);
    ASSERT_NE(nullptr, instance_no1_param2);
    ASSERT_NE(nullptr, instance_no2_param2);

    // Parameters have no overrides and all instances shoud share the same definition
    EXPECT_EQ(instance_no1_param1->definition, instance_no2_param1->definition);
    EXPECT_EQ(instance_no1_param2->definition, instance_no2_param2->definition);
}

constexpr char const wdd_content_instance_override_description[] = \
    "{"
    "    \"ModelReference\": \"" TEST_MODEL_NAME "\","
    "    \"WDMMVersion\": \"1.1.0\","
    "    \"Features\": ["
    "        \"" TEST_FEATURE_NAME "\""
    "    ],"
    "    \"ParameterValues\": ["
    "    ],"
    "    \"Instantiations\": ["
    "        {"
    "            \"Class\": \"" TEST_CLASS_NAME "\","
    "            \"Instances\": ["
    "                {"
    "                    \"ID\": 1,"
    "                    \"ParameterValues\": ["
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_1) ","
    "                            \"Value\": 42"
    "                        },"
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_2) ","
    "                            \"Value\": 42"
    "                        }"
    "                    ],"
    "                    \"Overrides\": ["
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_1) ","
    "                            \"AllowedValues\": {"
    "                                \"List\": [42, 53]"
    "                            }"
    "                        }"
    "                    ]"
    "                },"
    "                {"
    "                    \"ID\": 2,"
    "                    \"ParameterValues\": ["
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_1) ","
    "                            \"Value\": 31"
    "                        },"
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_2) ","
    "                            \"Value\": 31"
    "                        }"
    "                    ]"
    "                },"
    "                {"
    "                    \"ID\": 3,"
    "                    \"ParameterValues\": ["
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_1) ","
    "                            \"Value\": 32"
    "                        },"
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_2) ","
    "                            \"Value\": 32"
    "                        }"
    "                    ]"
    "                },"
    "                {"
    "                    \"ID\": 4,"
    "                    \"ParameterValues\": ["
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_1) ","
    "                            \"Value\": 53"
    "                        },"
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_2) ","
    "                            \"Value\": 53"
    "                        }"
    "                    ],"
    "                    \"Overrides\": ["
    "                        {"
    "                            \"ID\": " VALUE_STR(TEST_INSTANCE_PARAMETER_ID_1) ","
    "                            \"AllowedValues\": {"
    "                                \"Not\": [42]"
    "                            }"
    "                        }"
    "                    ]"
    "                }"
    "            ]"
    "        }"
    "    ]"
    "}"
    ;

TEST_F(device_description_loader_test_fixture, load_default_device_model_and_instance_override_description)
{
    this->SetDefaultDeviceModel();

    device_description_loader loader(*test_device_model, *test_device);
    std::string wdd_content_string(wdd_content_instance_override_description);
    loader.load(wdd_content_string);

    auto instance_no1_param1 = test_device->parameter_instances.get_instance(parameter_instance_id(TEST_INSTANCE_PARAMETER_ID_1,1));
    auto instance_no2_param1 = test_device->parameter_instances.get_instance(parameter_instance_id(TEST_INSTANCE_PARAMETER_ID_1,2));
    auto instance_no3_param1 = test_device->parameter_instances.get_instance(parameter_instance_id(TEST_INSTANCE_PARAMETER_ID_1,3));
    auto instance_no4_param1 = test_device->parameter_instances.get_instance(parameter_instance_id(TEST_INSTANCE_PARAMETER_ID_1,4));
    auto instance_no1_param2 = test_device->parameter_instances.get_instance(parameter_instance_id(TEST_INSTANCE_PARAMETER_ID_2,1));
    auto instance_no2_param2 = test_device->parameter_instances.get_instance(parameter_instance_id(TEST_INSTANCE_PARAMETER_ID_2,2));
    auto instance_no3_param2 = test_device->parameter_instances.get_instance(parameter_instance_id(TEST_INSTANCE_PARAMETER_ID_2,3));
    auto instance_no4_param2 = test_device->parameter_instances.get_instance(parameter_instance_id(TEST_INSTANCE_PARAMETER_ID_2,4));

    // Parameter instances for device: class instance count * class parameter count + class instantiations parameter
    ASSERT_EQ(4*2 + 1, test_device->parameter_instances.get_all().size());
    ASSERT_NE(nullptr, instance_no1_param1);
    ASSERT_NE(nullptr, instance_no2_param1);
    ASSERT_NE(nullptr, instance_no3_param1);
    ASSERT_NE(nullptr, instance_no4_param1);
    ASSERT_NE(nullptr, instance_no1_param2);
    ASSERT_NE(nullptr, instance_no2_param2);
    ASSERT_NE(nullptr, instance_no3_param2);
    ASSERT_NE(nullptr, instance_no4_param2);

    // Instance 2/3 have no overrides, they should share the same definition for parameter 1
    EXPECT_EQ(instance_no2_param1->definition, instance_no3_param1->definition);
    EXPECT_NE(instance_no2_param1->definition, instance_no1_param1->definition);
    EXPECT_NE(instance_no2_param1->definition, instance_no4_param1->definition);
    EXPECT_NE(instance_no1_param1->definition, instance_no4_param1->definition);

    // Overrides should be visible in parameter definitions for parameter 1
    EXPECT_EQ(true,  instance_no1_param1->definition->overrideables.allowed_values_set);
    EXPECT_EQ(2,     instance_no1_param1->definition->overrideables.allowed_values.whitelist.size());
    EXPECT_EQ(true,  instance_no4_param1->definition->overrideables.allowed_values_set);
    EXPECT_EQ(1,     instance_no4_param1->definition->overrideables.allowed_values.blacklist.size());

    // Instances without overrides should not contain modified parameter definitions for parameter 1
    EXPECT_EQ(false, instance_no2_param1->definition->overrideables.allowed_values_set);
    EXPECT_EQ(0,     instance_no2_param1->definition->overrideables.allowed_values.whitelist.size());
    EXPECT_EQ(false, instance_no3_param1->definition->overrideables.allowed_values_set);
    EXPECT_EQ(0,     instance_no3_param1->definition->overrideables.allowed_values.whitelist.size());

    // Parameter 2 has no overrides and all instances shoud share the same definition
    EXPECT_EQ(instance_no1_param2->definition, instance_no2_param2->definition);
    EXPECT_EQ(instance_no1_param2->definition, instance_no3_param2->definition);
    EXPECT_EQ(instance_no1_param2->definition, instance_no4_param2->definition);
}

constexpr char const wdd_content_description_v1_0_0[] = \
    "{"
    "    \"ModelReference\": \"" TEST_MODEL_NAME "\","
    "    \"WDMMVersion\": \"1.0.0\","
    "    \"Features\": ["
    "        \"" TEST_FEATURE_NAME "\""
    "    ]"
    "}"
    ;

TEST_F(device_description_loader_test_fixture, load_default_device_model_and_v1_0_0_description)
{
    this->SetDefaultDeviceModel();

    device_description_loader loader(*test_device_model, *test_device);
    std::string wdd_content_string(wdd_content_description_v1_0_0);
    loader.load(wdd_content_string);
}

constexpr char const wdd_content_description_v1_1_0[] = \
    "{"
    "    \"ModelReference\": \"" TEST_MODEL_NAME "\","
    "    \"WDMMVersion\": \"1.1.0\","
    "    \"Features\": ["
    "        \"" TEST_FEATURE_NAME "\""
    "    ]"
    "}"
    ;

TEST_F(device_description_loader_test_fixture, load_default_device_model_and_v1_1_0_description)
{
    this->SetDefaultDeviceModel();

    device_description_loader loader(*test_device_model, *test_device);
    std::string wdd_content_string(wdd_content_description_v1_1_0);
    loader.load(wdd_content_string);
}

constexpr char const wdd_content_description_v1_1_999[] = \
    "{"
    "    \"ModelReference\": \"" TEST_MODEL_NAME "\","
    "    \"WDMMVersion\": \"1.1.999\","
    "    \"Features\": ["
    "        \"" TEST_FEATURE_NAME "\""
    "    ]"
    "}"
    ;

TEST_F(device_description_loader_test_fixture, load_default_device_model_and_v1_1_999_description)
{
    this->SetDefaultDeviceModel();

    device_description_loader loader(*test_device_model, *test_device);
    std::string wdd_content_string(wdd_content_description_v1_1_999);
    loader.load(wdd_content_string);
}

constexpr char const wdd_content_description_v1_1_65536[] = \
    "{"
    "    \"ModelReference\": \"" TEST_MODEL_NAME "\","
    "    \"WDMMVersion\": \"1.1.65536\","
    "    \"Features\": ["
    "        \"" TEST_FEATURE_NAME "\""
    "    ]"
    "}"
    ;

TEST_F(device_description_loader_test_fixture, load_default_device_model_and_invalid_v1_1_65536_description)
{
    this->SetDefaultDeviceModel();

    device_description_loader loader(*test_device_model, *test_device);
    std::string wdd_content_string(wdd_content_description_v1_1_65536);
    EXPECT_THROW(loader.load(wdd_content_string), parameter_exception) << "Accepted invalid WDD (version number out of range)";
}

constexpr char const wdd_content_description_v1__0[] = \
    "{"
    "    \"ModelReference\": \"" TEST_MODEL_NAME "\","
    "    \"WDMMVersion\": \"1..0\","
    "    \"Features\": ["
    "        \"" TEST_FEATURE_NAME "\""
    "    ]"
    "}"
    ;

TEST_F(device_description_loader_test_fixture, load_default_device_model_and_invalid_v1__0_description)
{
    this->SetDefaultDeviceModel();

    device_description_loader loader(*test_device_model, *test_device);
    std::string wdd_content_string(wdd_content_description_v1__0);
    EXPECT_THROW(loader.load(wdd_content_string), parameter_exception) << "Accepted invalid WDD (missing minor version number)";
}

constexpr char const wdd_content_description_v1_1_a[] = \
    "{"
    "    \"ModelReference\": \"" TEST_MODEL_NAME "\","
    "    \"WDMMVersion\": \"1.1.a\","
    "    \"Features\": ["
    "        \"" TEST_FEATURE_NAME "\""
    "    ]"
    "}"
    ;

TEST_F(device_description_loader_test_fixture, load_default_device_model_and_invalid_v1_1_a_description)
{
    this->SetDefaultDeviceModel();

    device_description_loader loader(*test_device_model, *test_device);
    std::string wdd_content_string(wdd_content_description_v1_1_a);
    EXPECT_THROW(loader.load(wdd_content_string), parameter_exception) << "Accepted invalid WDD (non-numeric version number)";
}

constexpr char const wdd_content_description_v1_4_0[] = \
    "{"
    "    \"ModelReference\": \"" TEST_MODEL_NAME "\","
    "    \"WDMMVersion\": \"1.4.0\","
    "    \"Features\": ["
    "        \"" TEST_FEATURE_NAME "\""
    "    ]"
    "}"
    ;

TEST_F(device_description_loader_test_fixture, load_default_device_model_and_invalid_next_minor_description)
{
    this->SetDefaultDeviceModel();

    device_description_loader loader(*test_device_model, *test_device);
    std::string wdd_content_string(wdd_content_description_v1_4_0);
    EXPECT_THROW(loader.load(wdd_content_string), parameter_exception) << "Accepted unsupported WDD (minor version change)";
}

constexpr char const wdd_content_description_v2_0_0[] = \
    "{"
    "    \"ModelReference\": \"" TEST_MODEL_NAME "\","
    "    \"WDMMVersion\": \"2.0.0\","
    "    \"Features\": ["
    "        \"" TEST_FEATURE_NAME "\""
    "    ]"
    "}"
    ;

TEST_F(device_description_loader_test_fixture, load_default_device_model_and_invalid_v2_0_0_description)
{
    this->SetDefaultDeviceModel();

    device_description_loader loader(*test_device_model, *test_device);
    std::string wdd_content_string(wdd_content_description_v2_0_0);
    EXPECT_THROW(loader.load(wdd_content_string), parameter_exception) << "Accepted unsupported WDD (major version change)";
}
