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
///  \brief    Test managed object abstract base class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/managed_object.hpp"

#include "mocks/mock_sender.hpp"
#include "mocks/mock_driver.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::managed_object;
using wago::wdx::linuxos::com::managed_object_id;
using wago::wdx::linuxos::com::data_istream;

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

class managed_object_fixture : public testing::Test
{
public:
    mock_sender       sender_mock;
    mock_driver       driver_mock;
    managed_object_id test_id;
    std::string       test_connection_name;

    example_object object;

    managed_object_fixture()
    : test_id(42)
    , test_connection_name("Test Connection")
    , object(test_id, test_connection_name, sender_mock, driver_mock)
    { }

    ~managed_object_fixture() override = default;

    void SetUp() override
    {
        sender_mock.set_default_expectations();
        driver_mock.set_default_expectations();
    }
};

TEST_F(managed_object_fixture, construct_delete)
{
    // nothing to do
}

TEST_F(managed_object_fixture, get_id)
{
    auto id = object.get_id();
    EXPECT_EQ(test_id, id);
}

TEST_F(managed_object_fixture, get_connection_name)
{
    auto connection_name = object.get_connection_name();
    EXPECT_EQ(test_connection_name, connection_name);
}

TEST_F(managed_object_fixture, get_sender)
{
    auto & sender = object.get_sender();
    EXPECT_EQ(&sender_mock, &sender);
}

TEST_F(managed_object_fixture, get_driver)
{
    auto & driver = object.get_driver();
    EXPECT_EQ(&driver_mock, &driver);
}

//---- End of source file ------------------------------------------------------
