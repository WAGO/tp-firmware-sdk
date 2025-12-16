//------------------------------------------------------------------------------
// Copyright (c) 2024 WAGO GmbH & Co. KG
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
///  \brief    Test parameter value.
///
///  \author   Röh: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <limits>

#include "wago/wdx/parameter_value.hpp"
#include "instances/check_parameter_value.hpp"
#include "wda_ipc/ipc.hpp"

using namespace wago::wdx;

class parameter_value_fixture: public testing::TestWithParam<std::shared_ptr<parameter_value>>{};

TEST(parameter_value_test, create_string)
{
    std::string test_string = "Hallo Welt";

    auto v = parameter_value::create(test_string);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::string);
    EXPECT_EQ(v->get_string(), test_string);
    EXPECT_EQ(v->get_json(), R"("Hallo Welt")");
}

TEST(parameter_value_test, create_boolean)
{
    bool test_bool = true;

    auto v = parameter_value::create(test_bool);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::boolean);
    EXPECT_EQ(v->get_boolean(), test_bool);
    EXPECT_EQ(v->get_json(), test_bool ? "true" : "false");
    EXPECT_ANY_THROW(v->get_string());
}

TEST(parameter_value_test, create_float_32)
{
    float test_float = 12.34;

    auto v = parameter_value::create_float32(test_float);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::float32);
    EXPECT_NEAR(v->get_float32(), test_float, 0.0001f);
    EXPECT_ANY_THROW(v->get_boolean());
}

TEST(parameter_value_test, create_float_64)
{
    float test_float = 12.34;

    auto v = parameter_value::create_float64(test_float);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::float64);
    EXPECT_NEAR(v->get_float64(), test_float, 0.0001f);
    EXPECT_ANY_THROW(v->get_float32());
}

TEST(parameter_value_test, create_uint8)
{
    uint8_t test_int = 123;

    auto v = parameter_value::create_uint8(test_int);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::uint8);
    EXPECT_EQ(v->get_uint8(), test_int);
    EXPECT_EQ(v->get_json(), std::to_string(test_int));
    EXPECT_ANY_THROW(v->get_uint16());
}

TEST(parameter_value_test, create_uint8_excess_limits)
{
    int test_int = std::numeric_limits<uint8_t>::max() + 1;

    auto v = parameter_value::create_uint8(test_int);
    EXPECT_NE(v->get_uint8(), test_int); // excess bits are lost
}

TEST(parameter_value_test, create_uint16)
{
    int test_int = 1234;

    auto v = parameter_value::create_uint16(test_int);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::uint16);
    EXPECT_EQ(v->get_uint16(), test_int);
    EXPECT_ANY_THROW(v->get_float32());
}

TEST(parameter_value_test, create_unit16_excess_limits)
{
    unsigned int test_int = std::numeric_limits<uint16_t>::max() + 1;

    auto v = parameter_value::create_uint16(test_int);
    EXPECT_NE(v->get_uint16(), test_int); // excess bits are lost
}

TEST(parameter_value_test, create_uint32)
{
    uint32_t test_int = std::numeric_limits<uint32_t>::max();

    auto v = parameter_value::create_uint32(test_int);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::uint32);
    EXPECT_EQ(v->get_uint32(), test_int);
    EXPECT_ANY_THROW(v->get_uint16());
}

TEST(parameter_value_test, create_unit32_array)
{
    std::vector<uint32_t> test_values =
        {std::numeric_limits<uint32_t>::max(), 1, 2, 3};

    auto v = parameter_value::create_uint32_array(test_values);
    EXPECT_EQ(v->is_array(), true);
    EXPECT_EQ(v->get_type(), parameter_value_types::uint32);
    EXPECT_EQ(v->get_items()[0].get_uint32(), test_values[0]);
    EXPECT_EQ(v->get_items()[3].get_uint32(), test_values[3]);
}

TEST(parameter_value_test, create_unit64)
{
    uint64_t test_int = std::numeric_limits<uint64_t>::max();

    auto v = parameter_value::create_uint64(test_int);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::uint64);
    EXPECT_EQ(v->get_uint64(), test_int);
    EXPECT_ANY_THROW(v->get_uint32());
    EXPECT_EQ(v->get_json(), std::to_string(test_int));
}

TEST(parameter_value_test, create_int8)
{
    int test_int = -123;

    auto v = parameter_value::create_int8(test_int);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::int8);
    EXPECT_EQ(v->get_int8(), test_int);
    EXPECT_ANY_THROW(v->get_uint8());
}

TEST(parameter_value_test, create_int8_excess_lower_limit)
{
    int test_int = std::numeric_limits<int8_t>::min() -1;

    auto v = parameter_value::create_int8(test_int);
    EXPECT_NE(v->get_int8(), test_int); // overflow
}

TEST(parameter_value_test, create_int16)
{
    int test_int = -1234;

    auto v = parameter_value::create_int16(test_int);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::int16);
    EXPECT_EQ(v->get_int16(), test_int);
    EXPECT_ANY_THROW(v->get_uint16());
}

TEST(parameter_value_test, create_int16_excess_lower_limit)
{
    int test_int = std::numeric_limits<int16_t>::min() -1;

    auto v = parameter_value::create_int16(test_int);
    EXPECT_NE(v->get_int16(), test_int); // overflow
}

TEST(parameter_value_test, create_int32)
{
    int32_t test_int = -123123;

    auto v = parameter_value::create_int32(test_int);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::int32);
    EXPECT_EQ(v->get_int32(), test_int);
    EXPECT_ANY_THROW(v->get_uint32());
}

TEST(parameter_value_test, create_int64)
{
    int64_t test_int = -123123;

    auto v = parameter_value::create_int64(test_int);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::int64);
    EXPECT_EQ(v->get_int64(), test_int);
    EXPECT_ANY_THROW(v->get_int32());
    EXPECT_ANY_THROW(v->get_uint64());
}

TEST(parameter_value_test, create_instance_ref)
{
    instance_id_t instance_id = 4;

    auto v = parameter_value::create_instance_ref(instance_id);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::instance_ref);
    EXPECT_EQ(v->get_instance_ref(), instance_id);
    EXPECT_ANY_THROW(v->get_uint16());
}

TEST(parameter_value_test, create_unset_instance_ref)
{
    auto v = parameter_value::create_unset_instance_ref();
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::instance_ref);
    EXPECT_TRUE(v->is_unset_instance_ref());
    EXPECT_ANY_THROW(v->get_uint16());
}

TEST(parameter_value_test, create_instance_identity_ref)
{
    std::string base_path = "path";
    instance_id_t instance_id = 42;

    auto v = parameter_value::create_instance_identity_ref(base_path, instance_id);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::instance_identity_ref);
    EXPECT_EQ(v->get_instance_identity_ref(), base_path + "/" + std::to_string(instance_id));
    EXPECT_ANY_THROW(v->get_uint16());
}

TEST(parameter_value_test, create_instance_identity_ref_by_path)
{
    std::string instance_path = "path/42";

    auto v = parameter_value::create_instance_identity_ref(instance_path);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::instance_identity_ref);
    EXPECT_EQ(v->get_instance_identity_ref(), instance_path);
    EXPECT_ANY_THROW(v->get_uint16());
}

TEST(parameter_value_test, create_instance_identity_ref_by_path_invalid_base_path_only)
{
    std::string instance_path = "path";

    EXPECT_THROW(parameter_value::create_instance_identity_ref(instance_path), parameter_exception);
}

TEST(parameter_value_test, create_instance_identity_ref_by_path_invalid_base_path_without_instance)
{
    std::string instance_path = "path/";

    EXPECT_THROW(parameter_value::create_instance_identity_ref(instance_path), parameter_exception);
}

TEST(parameter_value_test, create_instance_identity_ref_by_path_invalid_instance_only)
{
    std::string instance_path = "42";

    EXPECT_THROW(parameter_value::create_instance_identity_ref(instance_path), parameter_exception);
}

TEST(parameter_value_test, create_unset_instance_identity_ref)
{
    auto v = parameter_value::create_unset_instance_identity_ref();
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::instance_identity_ref);
    EXPECT_TRUE(v->is_unset_instance_identity_ref());
    EXPECT_ANY_THROW(v->get_uint16());
}

TEST(parameter_value_test, create_instance_identity_ref_array)
{
    std::string base_path = "path";
    int size = 3;
    std::vector<std::pair<std::string,instance_id_t>> instances;
    for(int i = 0; i < size; ++i)
    {
        instances.push_back(std::pair<std::string, instance_id_t>(base_path, i));
    }

    auto v = parameter_value::create_instance_identity_ref_array(instances);
    EXPECT_EQ(v->is_array(), true);
    EXPECT_EQ(v->get_type(), parameter_value_types::instance_identity_ref);
    EXPECT_EQ(v->get_items().size(), size);
    EXPECT_EQ(v->get_items()[2].get_instance_identity_ref(),
              base_path + "/" + std::to_string(2));
    EXPECT_ANY_THROW(v->get_instance_identity_ref());
}

TEST(parameter_value_test, create_instance_identity_ref_from_json)
{
    std::string instance_path = "some/longer/path/42";
    json_string_t json_value  = "\"" + instance_path + "\"";

    auto v = parameter_value::create_with_json(parameter_value_types::instance_identity_ref,parameter_value_rank::scalar, json_value);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::instance_identity_ref);
    EXPECT_EQ(v->get_instance_identity_ref(), instance_path);
    EXPECT_ANY_THROW(v->get_uint16());
}

TEST(parameter_value_test, create_instance_identity_ref_from_json_invalid_path_only)
{
    std::string instance_path = "some/path";
    json_string_t json_value  = "\"" + instance_path + "\"";

    EXPECT_THROW(parameter_value::create_with_json(parameter_value_types::instance_identity_ref,parameter_value_rank::scalar, json_value)->get_instance_identity_ref(),
                 parameter_exception);
}

TEST(parameter_value_test, create_instance_identity_ref_from_json_invalid_path_without_instance)
{
    std::string instance_path = "some/path/";
    json_string_t json_value  = "\"" + instance_path + "\"";

    EXPECT_THROW(parameter_value::create_with_json(parameter_value_types::instance_identity_ref,parameter_value_rank::scalar, json_value)->get_instance_identity_ref(),
                 parameter_exception);
}

TEST(parameter_value_test, create_instance_identity_ref_from_json_invalid_instance_only)
{
    std::string instance_path = "42";
    json_string_t json_value  = "\"" + instance_path + "\"";

    EXPECT_THROW(parameter_value::create_with_json(parameter_value_types::instance_identity_ref,parameter_value_rank::scalar, json_value)->get_instance_identity_ref(),
                 parameter_exception);
}

TEST(parameter_value_test, create_enum_value)
{
    int test_value = 4;

    auto v = parameter_value::create_enum_value(test_value);
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::enum_member);
    EXPECT_EQ(v->get_enum_value(), test_value);
    EXPECT_ANY_THROW(v->get_uint16());
}

TEST(parameter_value_test, create_with_unknown_type_string)
{
    auto v = parameter_value::create_with_unknown_type("\"JSON\"");
    EXPECT_EQ(v->is_array(), false);
    EXPECT_EQ(v->get_type(), parameter_value_types::unknown);
    EXPECT_ANY_THROW(v->get_string());
    v->set_type_internal(parameter_value_types::string, parameter_value_rank::scalar);
    EXPECT_EQ(v->get_type(), parameter_value_types::string);
    EXPECT_EQ(v->get_string(), "JSON");
}

TEST(parameter_value_test, create_with_unknown_type_int)
{
    int test_int = 5;

    auto v = parameter_value::create_with_unknown_type(std::to_string(test_int));
    v->set_type_internal(parameter_value_types::uint16, parameter_value_rank::scalar);
    EXPECT_EQ(v->get_uint16(), test_int);
}

TEST(parameter_value_test, create_with_unknown_type_enum)
{
    int test_int = 5;

    auto v = parameter_value::create_with_unknown_type(std::to_string(test_int));
    v->set_type_internal(parameter_value_types::enum_member, parameter_value_rank::scalar);
    EXPECT_EQ(v->get_enum_value(), test_int);
}

TEST(parameter_value_test, create_with_unknown_type_bool)
{
    auto v = parameter_value::create_with_unknown_type("false");
    v->set_type_internal(parameter_value_types::boolean, parameter_value_rank::scalar);
    EXPECT_EQ(v->get_boolean(), false);
}

TEST(parameter_value_test, create_with_unknown_type_float)
{
    float test_float = 12.34;

    auto v = parameter_value::create_with_unknown_type(std::to_string(test_float));
    v->set_type_internal(parameter_value_types::float32, parameter_value_rank::scalar);
    EXPECT_NEAR(v->get_float32(), test_float, 0.0001f);
}

TEST(parameter_value_test, create_with_unknown_type_string_test_throw)
{
    auto v = parameter_value::create_with_unknown_type("\"Hallo\"");
    EXPECT_ANY_THROW(v->set_type_internal(parameter_value_types::boolean, parameter_value_rank::scalar));
}

TEST(parameter_value_test, create_with_unknown_type_int_test_throw)
{
    auto v = parameter_value::create_with_unknown_type("-5");
    EXPECT_ANY_THROW(v->set_type_internal(parameter_value_types::uint16, parameter_value_rank::scalar));
}

TEST(parameter_value_test, create_with_unknown_type_float_test_throw)
{
    auto v = parameter_value::create_with_unknown_type("1.5");
    EXPECT_ANY_THROW(v->set_type_internal(parameter_value_types::uint16, parameter_value_rank::scalar));
}

TEST(parameter_value_test, create_with_unknown_type_uint64)
{
    auto v = parameter_value::create_with_unknown_type(std::to_string(std::numeric_limits<uint64_t>::max())); // 2^64 - 1
    v->set_type_internal(parameter_value_types::uint64, parameter_value_rank::scalar);
    EXPECT_EQ(v->get_uint64(), std::numeric_limits<uint64_t>::max());
}

TEST(parameter_value_test, create_with_unknown_type_uint64_test_throw)
{
    auto v = parameter_value::create_with_unknown_type("18446744073709551616"); // 2^64
    EXPECT_ANY_THROW(v->set_type_internal(parameter_value_types::uint64, parameter_value_rank::scalar));
}

TEST(parameter_value_test, create_with_unknown_type_int64)
{
    auto v = parameter_value::create_with_unknown_type(std::to_string(std::numeric_limits<int64_t>::max())); // 2^63 - 1
    v->set_type_internal(parameter_value_types::int64, parameter_value_rank::scalar);
    EXPECT_EQ(v->get_int64(), std::numeric_limits<int64_t>::max());
}

TEST(parameter_value_test, create_with_unknown_type_int64_test_throw)
{
    auto v = parameter_value::create_with_unknown_type("9223372036854775808"); // 2^63+1
    EXPECT_ANY_THROW(v->set_type_internal(parameter_value_types::int64, parameter_value_rank::scalar));
}

TEST(parameter_value_test, create_with_unknown_type_float64)
{
    float test_float = 123456789.34;

    auto v = parameter_value::create_with_unknown_type(std::to_string(test_float));
    v->set_type_internal(parameter_value_types::float64, parameter_value_rank::scalar);
    EXPECT_NEAR(v->get_float64(), test_float, 0.0000001);
}

TEST(parameter_value_test, create_with_unknown_type_uint16_test_throw_upper_limit)
{
    std::string test_str = std::to_string(std::numeric_limits<uint16_t>::max() + 1);
    
    auto v = parameter_value::create_with_unknown_type(test_str); 
    EXPECT_ANY_THROW(v->set_type_internal(parameter_value_types::uint16, parameter_value_rank::scalar));
}

TEST(parameter_value_test, create_with_unknown_type_uint16_test_throw_)
{
    auto v = parameter_value::create_with_unknown_type("-1");
    EXPECT_ANY_THROW(v->set_type_internal(parameter_value_types::uint16, parameter_value_rank::scalar));
}

TEST(parameter_value_test, create_with_unknown_type_int16_test_throw)
{
    auto v = parameter_value::create_with_unknown_type("-32769");
    EXPECT_ANY_THROW(v->set_type_internal(parameter_value_types::int16, parameter_value_rank::scalar));
}

TEST(parameter_value_test, create_with_unknown_type_int16)
{
    int test_int = -32768;

    auto v = parameter_value::create_with_unknown_type(std::to_string(test_int));
    v->set_type_internal(parameter_value_types::int16, parameter_value_rank::scalar);
    EXPECT_EQ(v->get_int16(), test_int);
}

TEST(parameter_value_test, create_string_array)
{
    auto v = parameter_value::create_string_array({"Hallo", "Welt"});
    EXPECT_EQ(v->is_array(), true);
    EXPECT_EQ(v->get_type(), parameter_value_types::string);
    EXPECT_EQ(v->get_items()[0].get_string(), "Hallo");
    EXPECT_EQ(v->get_items()[1].get_string(), "Welt");
    EXPECT_EQ(v->get_json(), R"(["Hallo","Welt"])");
}

TEST(parameter_value_test, create_float32_array)
{
    auto v = parameter_value::create_float32_array({});
    EXPECT_EQ(v->is_array(), true);
    EXPECT_EQ(v->get_type(), parameter_value_types::float32);
}

TEST(parameter_value_test, create_with_unknown_type_uint64_array)
{
    auto v = parameter_value::create_with_unknown_type("[123, 42, 8]");
    v->set_type_internal(parameter_value_types::uint64, parameter_value_rank::array);
    EXPECT_EQ(v->is_array(), true);
    EXPECT_EQ(v->get_type(), parameter_value_types::uint64);
    EXPECT_EQ(v->get_items().size(), 3);
    EXPECT_EQ(v->get_items().at(1).get_uint64(), 42);
}

TEST(parameter_value_test, create_with_unknown_type_string_array_test_throw)
{
    auto v = parameter_value::create_with_unknown_type("[123, \"Hallo\"]");
    EXPECT_ANY_THROW(v->set_type_internal(parameter_value_types::uint64, parameter_value_rank::array));
}

TEST(parameter_value_test, create_with_unknown_type_float_array_test_throw)
{
    auto v = parameter_value::create_with_unknown_type("[123, 1.5]");
    EXPECT_ANY_THROW(v->set_type_internal(parameter_value_types::uint64, parameter_value_rank::array));
}

TEST(parameter_value_test, create_with_unknown_type_json)
{
    auto v = parameter_value::create_with_unknown_type(R"([{"Id": 2, "Classes": ["Meine", "Klasse"]}, {"Id": 4, "Classes": ["Klasse"]}])");
    v->set_type_internal(parameter_value_types::instantiations, parameter_value_rank::scalar);
    auto inst = v->get_instantiations();
    EXPECT_EQ(inst.size(), 2);
    EXPECT_TRUE(inst[0].has_class("Meine"));
    EXPECT_EQ(inst[0].id, 2);
    EXPECT_TRUE(inst[1].has_class("Klasse"));
    EXPECT_EQ(inst[1].id, 4);
}

TEST(parameter_value_test, create_with_json)
{
    auto v1 = parameter_value::create_uint32_array({1, 3, 5});
    auto v2 = parameter_value::create_uint16_array({1, 3, 5});
    auto v3 = parameter_value::create_with_json(parameter_value_types::uint32, parameter_value_rank::array, "[1,3,5]");
    EXPECT_FALSE(*v1 == *v2);
    EXPECT_TRUE(*v1 == *v3);
    EXPECT_TRUE(*v2 != *v3);
}

namespace {
    void create_test_definitions_for_instance_identity_checks(std::shared_ptr<class_definition>  class_def,
                                                              parameter_definition              &param_def,
                                                              char const                         base_path[])
    {
        class_def->base_path = base_path;
        param_def.value_rank = parameter_value_rank::scalar;
        param_def.value_type = parameter_value_types::instance_identity_ref;
        param_def.ref_classes_def = { class_def };
    }
}

TEST(check_parameter_value_test, valid_instance_identity_ref)
{
    char const base_path[] = "some/base/path";
    auto class_def = std::make_shared<class_definition>();
    parameter_definition param_def;
    create_test_definitions_for_instance_identity_checks(class_def, param_def, base_path);

    auto test_value = parameter_value::create_instance_identity_ref(base_path, 15);
    EXPECT_FALSE(has_error(check_parameter_value(test_value, &param_def)));
}

TEST(check_parameter_value_test, valid_instance_identity_ref_case_insensitive)
{
    char const base_path[]       = "some/baseCamel/path";
    char const base_path_lower[] = "some/basecamel/path";
    auto class_def = std::make_shared<class_definition>();
    parameter_definition param_def;
    create_test_definitions_for_instance_identity_checks(class_def, param_def, base_path);

    auto test_value = parameter_value::create_instance_identity_ref(base_path_lower, 15);
    EXPECT_FALSE(has_error(check_parameter_value(test_value, &param_def)));
}

TEST(check_parameter_value_test, unset_instance_identity_ref)
{
    char const base_path[] = "some/base/path";
    auto class_def = std::make_shared<class_definition>();
    parameter_definition param_def;
    create_test_definitions_for_instance_identity_checks(class_def, param_def, base_path);

    auto test_value = parameter_value::create_unset_instance_identity_ref();
    EXPECT_FALSE(has_error(check_parameter_value(test_value, &param_def)));
}

TEST(check_parameter_value_test, invalid_instance_identity_ref)
{
    char const base_path[] = "some/base/path";
    auto class_def = std::make_shared<class_definition>();
    parameter_definition param_def;
    create_test_definitions_for_instance_identity_checks(class_def, param_def, base_path);

    auto test_value = parameter_value::create_instance_identity_ref("some/other/path", 15);
    EXPECT_TRUE(has_error(check_parameter_value(test_value, &param_def)));
    EXPECT_EQ(status_codes::invalid_value, check_parameter_value(test_value, &param_def));
}

namespace {
    void create_test_definitions_for_instance_identity_array_checks(std::shared_ptr<class_definition>  class_def,
                                                                    parameter_definition              &param_def,
                                                                    char const                         base_path[])
    {
        class_def->base_path = base_path;
        param_def.value_rank = parameter_value_rank::array;
        param_def.value_type = parameter_value_types::instance_identity_ref;
        param_def.ref_classes_def = { class_def };
    }
}

TEST(check_parameter_value_test, valid_instance_identity_ref_array)
{
    char const base_path[] = "some/base/path";
    auto class_def = std::make_shared<class_definition>();
    parameter_definition param_def;
    create_test_definitions_for_instance_identity_array_checks(class_def, param_def, base_path);

    auto test_value = parameter_value::create_instance_identity_ref_array({ std::make_pair(base_path, 15),
                                                                            std::make_pair(base_path, 32) });
    EXPECT_FALSE(has_error(check_parameter_value(test_value, &param_def)));
}

TEST(check_parameter_value_test, unset_instance_identity_ref_array)
{
    char const base_path[] = "some/base/path";
    auto class_def = std::make_shared<class_definition>();
    parameter_definition param_def;
    create_test_definitions_for_instance_identity_array_checks(class_def, param_def, base_path);

    auto test_value = parameter_value::create_instance_identity_ref_array({ std::make_pair(base_path, 15),
                                                                            std::make_pair("",         0),
                                                                            std::make_pair(base_path, 32)  });
    EXPECT_FALSE(has_error(check_parameter_value(test_value, &param_def)));
}

TEST(check_parameter_value_test, empty_instance_identity_ref_array)
{
    char const base_path[] = "some/base/path";
    auto class_def = std::make_shared<class_definition>();
    parameter_definition param_def;
    create_test_definitions_for_instance_identity_array_checks(class_def, param_def, base_path);

    auto test_value = parameter_value::create_instance_identity_ref_array(std::vector<std::string>());
    EXPECT_FALSE(has_error(check_parameter_value(test_value, &param_def)));
}

TEST(check_parameter_value_test, invalid_instance_identity_ref_array)
{
    char const base_path[] = "some/base/path";
    auto class_def = std::make_shared<class_definition>();
    parameter_definition param_def;
    create_test_definitions_for_instance_identity_array_checks(class_def, param_def, base_path);

    auto test_value = parameter_value::create_instance_identity_ref_array({ std::make_pair(base_path, 15),
                                                                            std::make_pair("some/other/path", 42),
                                                                            std::make_pair(base_path, 32)  });
    EXPECT_TRUE(has_error(check_parameter_value(test_value, &param_def)));
    EXPECT_EQ(status_codes::invalid_value, check_parameter_value(test_value, &param_def));
}

TEST_P(parameter_value_fixture, ipc_serialization)
{
    auto value = GetParam();
    EXPECT_EQ(*value, *wago::wda_ipc::from_ipc_string<std::shared_ptr<parameter_value>>(wago::wda_ipc::to_ipc_string(value)));
}

TEST_P(parameter_value_fixture, move_semantics)
{
    auto original = GetParam();
    
    // Store expected values before move operations
    auto expected_type = original->get_type();
    auto expected_rank = original->get_rank();
    auto expected_json = original->get_json();
    
    // Test move constructor
    auto moved_constructed = std::move(*original);
    
    // Verify the moved object works correctly
    EXPECT_EQ(moved_constructed.get_type(), expected_type);
    EXPECT_EQ(moved_constructed.get_rank(), expected_rank);
    EXPECT_EQ(moved_constructed.get_json(), expected_json);
    
    // Test move assignment
    auto another = parameter_value::create_string("temp");
    *another = std::move(moved_constructed);
    
    // Verify the move-assigned object works correctly
    EXPECT_EQ(another->get_type(), expected_type);
    EXPECT_EQ(another->get_rank(), expected_rank);
    EXPECT_EQ(another->get_json(), expected_json);
}

INSTANTIATE_TEST_CASE_P(values, parameter_value_fixture, testing::Values(
    parameter_value::create_string("WAGO"),
    parameter_value::create_boolean(true),
    parameter_value::create_float32(1.234567f),
    parameter_value::create_float64(1.2345678901234),
    parameter_value::create_uint8(UINT8_MAX),
    parameter_value::create_uint16(UINT16_MAX),
    parameter_value::create_uint32(UINT32_MAX),
    parameter_value::create_uint64(UINT64_MAX),
    parameter_value::create_int8(INT8_MIN),
    parameter_value::create_int16(INT16_MIN),
    parameter_value::create_int32(INT32_MIN),
    parameter_value::create_int64(INT64_MIN),
    parameter_value::create_file_id("WAGO"),
    parameter_value::create_instantiations({class_instantiation(1, "myClass")}),
    parameter_value::create_instance_ref(UINT16_MAX),
    parameter_value::create_unset_instance_ref(),
    parameter_value::create_instance_identity_ref("MY/PATH", UINT16_MAX),
    parameter_value::create_unset_instance_identity_ref(),
    parameter_value::create_enum_value(UINT16_MAX),
    parameter_value::create_bytes({1, 2, 3, 4, 5}),
    parameter_value::create_string_array({"", "STRING"}),
    parameter_value::create_boolean_array({true, false}),
    parameter_value::create_float32_array({1.234567f, 0.123456f}),
    parameter_value::create_float64_array({1.23456789, 0.123456789}),
    parameter_value::create_uint8_array({UINT8_MAX, 0}),
    parameter_value::create_uint16_array({UINT16_MAX, 0}),
    parameter_value::create_uint32_array({UINT32_MAX, 0}),
    parameter_value::create_uint64_array({UINT64_MAX, 0}),
    parameter_value::create_int8_array({INT8_MAX, INT8_MIN}),
    parameter_value::create_int16_array({INT16_MAX, INT16_MIN}),
    parameter_value::create_int32_array({INT32_MAX, INT32_MIN}),
    parameter_value::create_int64_array({INT64_MAX, INT64_MIN}),
    parameter_value::create_file_id_array({"WAGO", "STRING"}),
    parameter_value::create_instance_ref_array({UINT16_MAX, 0}),
    parameter_value::create_instance_identity_ref_array({std::make_pair("My/Path", UINT16_MAX), std::make_pair("My/Other/Path", 42)}),
    parameter_value::create_enum_value_array({UINT16_MAX, 0}),
    parameter_value::create_bytes_array({{1, 2, 3, 4, 5}, {1, 2, 4, 8}})
    ));
