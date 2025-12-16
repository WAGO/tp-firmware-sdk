//------------------------------------------------------------------------------
// Copyright (c) 2022-2024 WAGO GmbH & Co. KG
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
///  \brief    Test WAGO Parameter Service Core value compatibility.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/status_codes.hpp"
#include "wago/wdx/parameter_value.hpp"
#include "wago/wdx/parameter_exception.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(value, source_compatibility_status_codes)
{
    // Status codes enum expected to be of type uint16_t
    EXPECT_EQ(sizeof(uint16_t), sizeof(status_codes));

    // These values should never change!
    EXPECT_EQ( 0, static_cast<uint16_t>(status_codes::success));
    EXPECT_EQ( 1, static_cast<uint16_t>(status_codes::no_error_yet));

    // Currently known and implemented status codes up to:
    EXPECT_GE(58, static_cast<uint16_t>(status_codes::highest));
}

TEST(value, source_compatibility_string_type)
{
    std::string const test_value = "Test string";

    std::shared_ptr<parameter_value> const shared_value = parameter_value::create_string(test_value);
    EXPECT_EQ(test_value, shared_value->get_string());
}

TEST(value, source_compatibility_instance_ref_unset)
{
    std::shared_ptr<parameter_value> const shared_value = parameter_value::create_unset_instance_ref();
    EXPECT_TRUE(shared_value->is_unset_instance_ref());
    EXPECT_EQ(0, shared_value->get_instance_ref());
}

TEST(value, source_compatibility_instance_identity_ref_unset)
{
    std::shared_ptr<parameter_value> const shared_value = parameter_value::create_unset_instance_identity_ref();
    EXPECT_TRUE(shared_value->is_unset_instance_identity_ref());
    EXPECT_TRUE(shared_value->get_instance_identity_ref().empty());
}

// TODO: Enable test if round trip is supported by core
TEST(value, source_compatibility_instance_identity_ref_round_trip)
{
    constexpr char          const base_path[] = "Networking/Bridges";
    constexpr instance_id_t const instance = 1;

    auto const direct_value = parameter_value::create_instance_identity_ref(base_path, instance);
    EXPECT_FALSE(direct_value->get_instance_identity_ref().empty());

    auto const round_trip_value = parameter_value::create_instance_identity_ref(direct_value->get_instance_identity_ref());
    EXPECT_FALSE(round_trip_value->get_instance_identity_ref().empty());
    EXPECT_EQ(*direct_value, *round_trip_value);
}

TEST(value, validate_64_bit_unsigned_integer)
{
    std::shared_ptr<parameter_value> const shared_value = parameter_value::create_with_unknown_type(std::to_string(UINT64_MAX));
    EXPECT_NO_THROW(shared_value->set_type_internal(parameter_value_types::uint64, parameter_value_rank::scalar));
    EXPECT_EQ(UINT64_MAX, shared_value->get_uint64());
}

TEST(value, validate_64_bit_unsigned_integer_overflow)
{
    std::shared_ptr<parameter_value> const shared_value = parameter_value::create_with_unknown_type(std::to_string(UINT64_MAX) + "0");
    EXPECT_THROW(shared_value->set_type_internal(parameter_value_types::uint64, parameter_value_rank::scalar), wago::wdx::parameter_exception);
}

TEST(value, validate_64_bit_integer)
{
    std::shared_ptr<parameter_value> const shared_value_1 = parameter_value::create_with_unknown_type(std::to_string(INT64_MAX));
    EXPECT_NO_THROW(shared_value_1->set_type_internal(parameter_value_types::int64, parameter_value_rank::scalar));
    EXPECT_EQ(INT64_MAX, shared_value_1->get_int64());

    std::shared_ptr<parameter_value> const shared_value_2 = parameter_value::create_with_unknown_type(std::to_string(INT64_MIN));
    EXPECT_NO_THROW(shared_value_2->set_type_internal(parameter_value_types::int64, parameter_value_rank::scalar));
    EXPECT_EQ(INT64_MIN, shared_value_2->get_int64());
}

TEST(value, validate_64_bit_integer_overflow)
{
    std::shared_ptr<parameter_value> const shared_value_1 = parameter_value::create_with_unknown_type(std::to_string(INT64_MAX) + "0");
    EXPECT_THROW(shared_value_1->set_type_internal(parameter_value_types::int64, parameter_value_rank::scalar), wago::wdx::parameter_exception);

    std::shared_ptr<parameter_value> const shared_value_2 = parameter_value::create_with_unknown_type(std::to_string(INT64_MIN) + "0");
    EXPECT_THROW(shared_value_2->set_type_internal(parameter_value_types::int64, parameter_value_rank::scalar), wago::wdx::parameter_exception);
}


//---- End of source file ------------------------------------------------------
