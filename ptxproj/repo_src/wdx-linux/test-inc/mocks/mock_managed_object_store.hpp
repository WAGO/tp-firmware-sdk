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
///  \brief    Mock for managed_object_store_i interface (libwdxlinuxoscom).
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_MANAGED_OBJECT_STORE_HPP_
#define TEST_INC_MOCKS_MOCK_MANAGED_OBJECT_STORE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/managed_object_store_i.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::managed_object_store_i;
using wago::wdx::linuxos::com::managed_object;
using wago::wdx::linuxos::com::managed_object_id;
using wago::wdx::linuxos::com::sender_i;
using wago::wdx::linuxos::com::driver_i;
using testing::Invoke;
using testing::Return;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_managed_object_store : public managed_object_store_i
{
public:
    // custom override to face gmock's limited support 
    // for move-only arguments
    managed_object &add_object(std::unique_ptr<managed_object> &&obj) override
    {
        return add_object(obj);
    }

    MOCK_METHOD0(has_generated_objects, bool());
    MOCK_METHOD0(generate_object_id, managed_object_id());
    MOCK_METHOD1(has_object, bool(managed_object_id));
    MOCK_METHOD1(get_object, managed_object &(managed_object_id));
    MOCK_METHOD1(get_object_meta, std::shared_ptr<object_meta>(managed_object_id));
    MOCK_METHOD1(add_object, managed_object &(std::unique_ptr<managed_object> &));
    MOCK_METHOD1(remove_object, void(managed_object_id id));
    MOCK_METHOD1(remove_objects, void(std::function<bool(managed_object const &obj)> predicate));

    MOCK_CONST_METHOD0(get_connection_name, std::string const &());
    MOCK_CONST_METHOD0(get_sender, sender_i &());
    MOCK_CONST_METHOD0(get_driver, driver_i &());
    MOCK_METHOD1(find, managed_object * (std::function<bool(managed_object const &obj)> predicate));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, has_generated_objects())
            .Times(0);
        EXPECT_CALL(*this, generate_object_id())
            .Times(0);
        EXPECT_CALL(*this, has_object(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_object(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_object_meta(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, add_object(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remove_object(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remove_objects(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_connection_name())
            .Times(0);
        EXPECT_CALL(*this, get_sender())
            .Times(0);
        EXPECT_CALL(*this, get_driver())
            .Times(0);
        EXPECT_CALL(*this, find(::testing::_))
            .Times(0);
    }

    void expect_object_to_be_get(managed_object &obj)
    {
        EXPECT_CALL(*this, get_object(testing::Eq(obj.get_id())))
            .Times(1)
            .WillRepeatedly(Invoke([&obj](auto) {
                return std::ref(obj);
            }));
    }

    void expect_object_meta_to_be_get(managed_object_id obj_id)
    {
        EXPECT_CALL(*this, get_object_meta(testing::Eq(obj_id)))
            .Times(1)
            .WillRepeatedly(Invoke([](auto) {
                return std::make_shared<object_meta>();
            }));
    }

    void expect_object_ids_to_be_generated(managed_object_id first, managed_object_id last)
    {
        static managed_object_id next_id;
        next_id = first;
        auto count = static_cast<int>(last - first) + 1;
        EXPECT_CALL(*this, generate_object_id())
            .Times(count)
            .WillRepeatedly(Invoke([first]() {
                return next_id++;
            }));
    }

    using temp_object_storage = std::vector<std::shared_ptr<managed_object>>;
    template <class ManagedObjectType>
    std::shared_ptr<temp_object_storage> expect_objects_to_be_added(managed_object_id first, managed_object_id last)
    {
        static managed_object_id next_id;
        next_id = first;
        auto count = static_cast<int>(last - first) + 1;
        auto added = std::make_shared<temp_object_storage>();
        EXPECT_CALL(*this, add_object(::testing::_))
            .Times(count)
            .WillRepeatedly(Invoke([added, first](std::unique_ptr<managed_object> &object) {
                EXPECT_EQ(next_id++, object->get_id());
                ManagedObjectType *provider_stub = dynamic_cast<ManagedObjectType*>(object.get());
                EXPECT_TRUE(provider_stub != nullptr);
                return std::ref(add_object_helper(added, object));
            }));
        return added;
    }

    template <class ManagedObjectType>
    std::shared_ptr<temp_object_storage> expect_object_to_be_added(managed_object_id expected_id)
    {
        return expect_objects_to_be_added<ManagedObjectType>(expected_id, expected_id);
    }

    void expect_object_to_be_removed(managed_object_id expected_id)
    {
        EXPECT_CALL(*this, remove_object(testing::Eq(expected_id)))
            .Times(1);
    }

    void expect_object_to_be_found(managed_object & object)
    {
        EXPECT_CALL(*this, find(testing::_))
            .Times(1)
            .WillRepeatedly(Invoke([&object](auto predicate) {
                EXPECT_TRUE(predicate(object));
                return &object;
            }));
    }

private:
    static managed_object & add_object_helper(std::shared_ptr<temp_object_storage> storage, std::unique_ptr<managed_object> &object)
    {
        storage->emplace_back(std::move(object));
        return *storage->back();
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_MANAGED_OBJECT_STORE_HPP_
//---- End of source file ------------------------------------------------------
