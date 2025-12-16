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
///  \brief    Reusable fixture for tests of basic/simple proxies and stubs.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_SRC_LIBWDXLINUXOSCOM_BASE_PROXY_AND_STUB_FIXTURE_HPP_
#define TEST_SRC_LIBWDXLINUXOSCOM_BASE_PROXY_AND_STUB_FIXTURE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "proxy_stub_helper.hpp"

#include "mocks/mock_sender.hpp"
#include "common/managed_object.hpp"
#include "mocks/mock_driver.hpp"

#include <memory>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::message_data;
using wago::wdx::linuxos::com::managed_object;
using wago::wdx::linuxos::com::managed_object_id;

//------------------------------------------------------------------------------
// fixture definition
//------------------------------------------------------------------------------
template<class InterfaceMock, class Proxy, class Stub>
class base_proxy_and_stub_fixture : public testing::Test
{
public:
    mock_sender   sender_mock;
    mock_driver   driver_mock;
    InterfaceMock interface_mock;

    managed_object_id test_id;
    std::string       test_connection_name;

    Proxy proxy;
    Stub  stub;

    proxy_stub_helper helper;

    base_proxy_and_stub_fixture()
    : test_id(42)
    , test_connection_name("Test Connection")
    , proxy(test_id, test_connection_name, sender_mock, driver_mock)
    , stub(test_id, test_connection_name, sender_mock, driver_mock, interface_mock, wc_trace_channels::all)
    , helper(driver_mock, sender_mock, proxy, stub)
    { }

    ~base_proxy_and_stub_fixture() override = default;

    void SetUp() override
    {
        sender_mock.set_default_expectations();
        driver_mock.set_default_expectations();
        interface_mock.set_default_expectations();
    }
};

#endif // TEST_SRC_LIBWDXLINUXOSCOM_BASE_PROXY_AND_STUB_FIXTURE_HPP_
//---- End of source file ------------------------------------------------------
