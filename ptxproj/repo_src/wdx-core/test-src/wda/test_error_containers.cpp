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
///  \brief    Test error container objects.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/core_error.hpp"
#include "rest/data_error.hpp"
#include "rest/attribute_error.hpp"
#include "rest/relationship_error.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda::rest;
using namespace wago;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char              const default_error_text[]        = "Test error text";
static constexpr wdx::status_codes const default_core_status_code    = wdx::status_codes::internal_error;
static constexpr uint16_t          const default_domain_status_code  = 0;
static constexpr char              const default_id[]                = "Test-ID";
static constexpr char              const default_type[]              = "test_type";
static constexpr size_t            const default_data_index          = 123;
static constexpr char              const default_attribute_name[]    = "test_attribute";
static constexpr char              const default_relationship_name[] = "test_relationship";
static constexpr size_t            const default_relationship_index  = 456;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(core_error, construct_delete_with_message)
{
    core_error const error(default_core_status_code, default_error_text, default_domain_status_code,
                           default_type, default_id);
    EXPECT_FALSE(error.get_title().empty());
    EXPECT_TRUE(error.get_message().find(default_id) != std::string::npos);
    EXPECT_TRUE(error.get_message().find(default_type) != std::string::npos);
    EXPECT_TRUE(error.get_message().find(default_error_text) != std::string::npos);
    EXPECT_EQ(default_core_status_code, error.get_core_status_code());
}

TEST(data_error, derived_from_core_error)
{
    data_error const error(default_core_status_code, default_error_text, default_domain_status_code,
                           default_type, default_id, 
                           default_data_index);
    core_error const * const core_ptr = dynamic_cast<core_error const *>(&error);
    EXPECT_NE(nullptr, core_ptr);
}

TEST(data_error, container_specific_values)
{
    data_error const error(default_core_status_code, default_error_text, default_domain_status_code,
                           default_type, default_id, 
                           default_data_index);

    EXPECT_FALSE(error.get_title().empty());
    EXPECT_TRUE(error.get_message().find(default_id) != std::string::npos);
    EXPECT_TRUE(error.get_message().find(default_type) != std::string::npos);
    EXPECT_TRUE(error.get_message().find(default_error_text) != std::string::npos);
    EXPECT_EQ(default_data_index, error.get_data_index());
}

TEST(attribute_error, derived_from_core_error)
{
    attribute_error const error(default_core_status_code, default_error_text, default_domain_status_code,
                                default_type, default_id, 
                                default_data_index, default_attribute_name);
    core_error const * const core_ptr = dynamic_cast<core_error const *>(&error);
    EXPECT_NE(nullptr, core_ptr);
}

TEST(attribute_error, derived_from_data_error)
{
    attribute_error const error(default_core_status_code, default_error_text, default_domain_status_code,
                                default_type, default_id, 
                                default_data_index, default_attribute_name);
    data_error const * const data_ptr = dynamic_cast<data_error const *>(&error);
    EXPECT_NE(nullptr, data_ptr);
}

TEST(attribute_error, container_specific_values)
{
    attribute_error const error(default_core_status_code, default_error_text, default_domain_status_code,
                                default_type, default_id, 
                                default_data_index, default_attribute_name);

    EXPECT_FALSE(error.get_title().empty());
    EXPECT_TRUE(error.get_message().find(default_id) != std::string::npos);
    EXPECT_TRUE(error.get_message().find(default_type) != std::string::npos);
    EXPECT_TRUE(error.get_message().find(default_error_text) != std::string::npos);
    EXPECT_EQ(default_data_index, error.get_data_index());
    EXPECT_EQ(default_attribute_name, error.get_attribute_name());
}

TEST(relationship_error, derived_from_core_error)
{
    relationship_error const error(default_core_status_code, default_error_text, default_domain_status_code,
                                   default_type, default_id, 
                                   default_data_index, default_relationship_name, default_relationship_index);
    core_error const * const core_ptr = dynamic_cast<core_error const *>(&error);
    EXPECT_NE(nullptr, core_ptr);
}

TEST(relationship_error, derived_from_data_error)
{
    relationship_error const error(default_core_status_code, default_error_text, default_domain_status_code,
                                   default_type, default_id, 
                                   default_data_index, default_relationship_name, default_relationship_index);
    data_error const * const data_ptr = dynamic_cast<data_error const *>(&error);
    EXPECT_NE(nullptr, data_ptr);
}

TEST(relationship_error, container_specific_values)
{
    relationship_error const error(default_core_status_code, default_error_text, default_domain_status_code,
                                   default_type, default_id, 
                                   default_data_index, default_relationship_name, default_relationship_index);

    EXPECT_FALSE(error.get_title().empty());
    EXPECT_TRUE(error.get_message().find(default_id) != std::string::npos);
    EXPECT_TRUE(error.get_message().find(default_type) != std::string::npos);
    EXPECT_TRUE(error.get_message().find(default_error_text) != std::string::npos);
    EXPECT_EQ(default_data_index, error.get_data_index());
    EXPECT_EQ(default_relationship_name, error.get_relationship_name());
    EXPECT_EQ(default_relationship_index, error.get_relationship_index());
}

//---- End of source file ------------------------------------------------------
