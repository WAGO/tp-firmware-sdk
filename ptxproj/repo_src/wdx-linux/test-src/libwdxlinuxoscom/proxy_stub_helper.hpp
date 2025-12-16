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
///  \brief    Helper functions for proxy and stub tests
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_SRC_LIBWDXLINUXOSCOM_PROXY_STUB_HELPER_HPP_
#define TEST_SRC_LIBWDXLINUXOSCOM_PROXY_STUB_HELPER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
class proxy_stub_helper
{
private:
    mock_driver &driver_mock;
    mock_sender &sender_mock;

    managed_object &proxy;
    managed_object &stub;
    std::shared_ptr<message_data> msg_for_proxy;
    std::shared_ptr<message_data> msg_for_stub;

public:
    proxy_stub_helper(mock_driver    &driver,
                      mock_sender    &sender,
                      managed_object &proxy_object,
                      managed_object &stub_object);

    using roundtrip = std::function<void()>;

    /// Expect the given number of roundtrips (number of 'sends' and 
    /// 'receives' on both proxy and stub side). To execute any required 
    /// run() calls execute the returned callback.
    roundtrip expect_roundtrip(bool is_async, int number = 1);

    /// Expect the given number of 'sends' on the proxy side and 'receive'
    /// on the stub side.
    /// To execute any required run() calls
    /// execute the returned callback.
    roundtrip expect_send_and_receive(bool is_async,
                                      int  number = 1);

private:
    roundtrip roundtrip_internal(bool is_async,
                                 int  proxy_stub_number,
                                 int  stub_proxy_number);
    void roundtrip_run_helper();
    void roundtrip_send_helper(managed_object                &object,
                               std::shared_ptr<message_data> &message_for_target,
                               int                            number);
};


#endif // TEST_SRC_LIBWDXLINUXOSCOM_PROXY_STUB_HELPER_HPP_
//---- End of source file ------------------------------------------------------
