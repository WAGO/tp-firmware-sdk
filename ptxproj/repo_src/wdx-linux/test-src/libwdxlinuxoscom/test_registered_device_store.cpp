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
///  \brief    Test registered device store implementation.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "backend/registered_device_store.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::registered_device_store;
using wago::wdx::linuxos::com::registered_device_store_i;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class registered_device_store_fixture : public testing::Test
{
public:
    std::string               const  test_connection_name;
    registered_device_store          store_member;
    registered_device_store_i       &store;

    registered_device_store_fixture()
    : test_connection_name("Test Connection")
    , store_member(test_connection_name)
    , store(store_member)
    { }

    ~registered_device_store_fixture() override = default;
};

TEST_F(registered_device_store_fixture, construct_delete)
{
    EXPECT_STREQ(test_connection_name.c_str(), store.get_connection_name().c_str());
}

TEST_F(registered_device_store_fixture, has_devices)
{
    wago::wdx::device_id               const id(5, wago::wdx::device_collections::root);
    wago::wdx::register_device_request const device_request = { id, "0123-4567", "01.00.00" };

    EXPECT_FALSE(store.has_devices());

    store.add_device(device_request);
    EXPECT_TRUE(store.has_devices());

    store.remove_device(id);
    EXPECT_FALSE(store.has_devices());
}

TEST_F(registered_device_store_fixture, has_device)
{
    wago::wdx::device_id               const id(32, wago::wdx::device_collections::rlb);
    wago::wdx::register_device_request const device_request = { id, "9876-5246", "00.02.01" };

    EXPECT_FALSE(store.has_device(id));

    store.add_device(device_request);
    EXPECT_TRUE(store.has_device(id));

    store.remove_device(id);
    EXPECT_FALSE(store.has_device(id));
}

TEST_F(registered_device_store_fixture, get_device_meta)
{
    wago::wdx::device_id               const id(19, wago::wdx::device_collections::rlb);
    wago::wdx::register_device_request const device_request = { id, "9276-5286", "00.00.01" };

    EXPECT_EQ(nullptr, store.get_device_meta(id));

    store.add_device(device_request);
    EXPECT_NE(nullptr, store.get_device_meta(id));

    store.remove_device(id);
    EXPECT_EQ(nullptr, store.get_device_meta(id));
}

TEST_F(registered_device_store_fixture, add_device)
{
    wago::wdx::device_id               const id(3, wago::wdx::device_collections::root);
    wago::wdx::register_device_request const device_request = { id, "1276-5246", "00.00.42" };

    store.add_device(device_request);
    EXPECT_THROW(store.add_device(device_request), wago::wdx::linuxos::com::exception);
}

TEST_F(registered_device_store_fixture, remove_device)
{
    wago::wdx::device_id               const id(3, wago::wdx::device_collections::rlb);
    wago::wdx::register_device_request const device_request = { id, "4682-5246", "99.00.00" };

    EXPECT_FALSE(store.has_device(id));
    EXPECT_NO_THROW(store.remove_device(id));
    store.add_device(device_request);
    EXPECT_TRUE(store.has_device(id));
    EXPECT_NO_THROW(store.remove_device(id));
    EXPECT_FALSE(store.has_device(id));
    EXPECT_NO_THROW(store.remove_device(id));
}

TEST_F(registered_device_store_fixture, remove_objects)
{
    wago::wdx::device_id               const id(250, wago::wdx::device_collections::rlb);
    wago::wdx::register_device_request const device_request = { id, "4382-5246", "99.80.70" };
    auto test_predicate_true  = [](registered_device_store_i::device_meta const &) { return true; };
    auto test_predicate_false = [](registered_device_store_i::device_meta const &) { return false; };

    EXPECT_FALSE(store.has_device(id));
    EXPECT_NO_THROW(store.remove_devices(test_predicate_true));
    EXPECT_NO_THROW(store.remove_devices(test_predicate_false));
    store.add_device(device_request);
    EXPECT_TRUE(store.has_device(id));
    EXPECT_NO_THROW(store.remove_devices(test_predicate_false));
    EXPECT_TRUE(store.has_device(id));
    EXPECT_NO_THROW(store.remove_devices(test_predicate_true));
    EXPECT_FALSE(store.has_device(id));
}


//---- End of source file ------------------------------------------------------
