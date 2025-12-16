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
///  \brief    Test managed object store implementation.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/managed_object_store.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"

#include "mocks/mock_sender.hpp"
#include "mocks/mock_driver.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::managed_object;
using wago::wdx::linuxos::com::managed_object_id;
using wago::wdx::linuxos::com::managed_object_store;
using wago::wdx::linuxos::com::managed_object_store_i;
using wago::wdx::linuxos::com::data_istream;
using wago::wdx::linuxos::com::backend_object_id;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
namespace {
class example_object : public managed_object
{
public:
    using managed_object::managed_object;
    void handle_message(data_istream &received) override;
};
void example_object::handle_message(data_istream &)
{ /* no implementation needed for this test */ }
}

class managed_object_store_fixture : public testing::Test
{
public:
    mock_sender             sender_mock;
    mock_driver             driver_mock;
    std::string             test_connection_name;

    managed_object_store    store_member;
    managed_object_store_i &store;

    managed_object_store_fixture()
    : test_connection_name("Test Connection")
    , store_member(test_connection_name, sender_mock, driver_mock)
    , store(store_member)
    { }

    ~managed_object_store_fixture() override = default;

    void SetUp() override
    {
        sender_mock.set_default_expectations();
        driver_mock.set_default_expectations();
    }

    std::unique_ptr<managed_object> create_test_object(managed_object_id id)
    {
        return std::make_unique<example_object>(id, test_connection_name, sender_mock, driver_mock);
    }
};

TEST_F(managed_object_store_fixture, construct_delete)
{
    EXPECT_STREQ(test_connection_name.c_str(), store.get_connection_name().c_str());
}

TEST_F(managed_object_store_fixture, has_generated_objects)
{
    auto generated_id = store.generate_object_id();
    EXPECT_FALSE(store.has_generated_objects());

    auto &object = store.add_object(create_test_object(generated_id));
    EXPECT_TRUE(store.has_generated_objects());

    store.remove_object(object.get_id());
    EXPECT_FALSE(store.has_generated_objects());
}

TEST_F(managed_object_store_fixture, generate_object_id)
{
    auto generated_id = store.generate_object_id();
    EXPECT_NE(backend_object_id, generated_id);

    auto second_generated_id = store.generate_object_id();
    EXPECT_NE(backend_object_id, second_generated_id);
    EXPECT_NE(generated_id, second_generated_id);
}

TEST_F(managed_object_store_fixture, has_object)
{
    auto generated_id = store.generate_object_id();
    EXPECT_FALSE(store.has_object(generated_id));

    auto &object = store.add_object(create_test_object(generated_id));
    EXPECT_TRUE(store.has_object(generated_id));

    store.remove_object(object.get_id());
    EXPECT_FALSE(store.has_object(generated_id));
}

TEST_F(managed_object_store_fixture, get_object)
{
    auto generated_id = store.generate_object_id();
    EXPECT_THROW(store.get_object(generated_id), wago::wdx::linuxos::com::exception);

    auto &object = store.add_object(create_test_object(generated_id));
    EXPECT_EQ(&object, &store.get_object(generated_id));

    store.remove_object(object.get_id());
    EXPECT_THROW(store.get_object(generated_id), wago::wdx::linuxos::com::exception);
}

TEST_F(managed_object_store_fixture, get_object_meta)
{
    auto generated_id = store.generate_object_id();
    EXPECT_EQ(nullptr, store.get_object_meta(generated_id));

    auto &object = store.add_object(create_test_object(generated_id));
    EXPECT_NE(nullptr, store.get_object_meta(generated_id));

    store.remove_object(object.get_id());
    EXPECT_EQ(nullptr, store.get_object_meta(generated_id));
}

TEST_F(managed_object_store_fixture, add_object)
{
    auto generated_id = store.generate_object_id();
    store.add_object(create_test_object(generated_id));
    EXPECT_THROW(store.add_object(create_test_object(generated_id)), wago::wdx::linuxos::com::exception);
}

TEST_F(managed_object_store_fixture, remove_object)
{
    auto generated_id = store.generate_object_id();
    EXPECT_FALSE(store.has_object(generated_id));
    EXPECT_NO_THROW(store.remove_object(generated_id));
    store.add_object(create_test_object(generated_id));
    EXPECT_TRUE(store.has_object(generated_id));
    EXPECT_NO_THROW(store.remove_object(generated_id));
    EXPECT_FALSE(store.has_object(generated_id));
    EXPECT_NO_THROW(store.remove_object(generated_id));
}

TEST_F(managed_object_store_fixture, remove_objects)
{
    auto test_predicate_true  = [](managed_object const &) { return true; };
    auto test_predicate_false = [](managed_object const &) { return false; };

    auto generated_id = store.generate_object_id();
    EXPECT_FALSE(store.has_object(generated_id));
    EXPECT_NO_THROW(store.remove_objects(test_predicate_true));
    EXPECT_NO_THROW(store.remove_objects(test_predicate_false));
    store.add_object(create_test_object(generated_id));
    EXPECT_TRUE(store.has_object(generated_id));
    EXPECT_NO_THROW(store.remove_objects(test_predicate_false));
    EXPECT_TRUE(store.has_object(generated_id));
    EXPECT_NO_THROW(store.remove_objects(test_predicate_true));
    EXPECT_FALSE(store.has_object(generated_id));
}


//---- End of source file ------------------------------------------------------
