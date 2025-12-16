//------------------------------------------------------------------------------
// Copyright (c) 2025 WAGO GmbH & Co. KG
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
///  \brief    Mock for systemd interface (system abstraction).
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_SYSTEMD_HPP_
#define TEST_INC_MOCKS_MOCK_SYSTEMD_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using wago::authserv::sal::systemd;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_systemd : public systemd
{
private:
    systemd *saved_instance;

public:
    static void set_instance(systemd *new_instance)
    {
        instance = new_instance;
    }

    mock_systemd()
    {
        saved_instance = &get_instance();
        set_instance(this);
    }
    ~mock_systemd()
    {
        set_instance(saved_instance);
    }

    MOCK_CONST_METHOD0(get_listen_fds_start, int());
    MOCK_CONST_METHOD1(get_listen_fds_with_names, int(char ***fd_names));
    MOCK_CONST_METHOD1(check_for_socket_unix, int(int fd));
    MOCK_CONST_METHOD1(find_systemd_socket, int(std::string const &socket_name));

    MOCK_CONST_METHOD0(notify_ready, void());
    MOCK_CONST_METHOD0(notify_stopping, void());

    void set_default_expectations()
    {
        EXPECT_CALL(*this, get_listen_fds_start())
            .Times(0);
        EXPECT_CALL(*this, get_listen_fds_with_names(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, check_for_socket_unix(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, find_systemd_socket(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, notify_ready())
            .Times(0);
        EXPECT_CALL(*this, notify_stopping())
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_SYSTEMD_HPP_
//---- End of source file ------------------------------------------------------
