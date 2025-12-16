//------------------------------------------------------------------------------
// Copyright (c) 2023-2024 WAGO GmbH & Co. KG
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
///  \brief    Mock for the setting store.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_SETTINGS_STORE_HPP_
#define TEST_INC_MOCKS_MOCK_SETTINGS_STORE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/settings_store_i.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using testing::WithArg;
using testing::Invoke;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_settings_store: public wago::authserv::settings_store_i
{
public:
    MOCK_CONST_METHOD1(get_global_setting, std::string const&(wago::authserv::settings_store_i::global_setting key));
    MOCK_METHOD2(set_global_config, bool(wago::authserv::settings_store_i::global_config_map const &new_values,
                                         wago::authserv::settings_store_i::configuration_error_map &error_messages_out));
    MOCK_CONST_METHOD1(client_exists, bool(const std::string& client_id));
    MOCK_CONST_METHOD1(get_client, wago::authserv::settings_store_i::oauth_client&(const std::string& client_id));
    MOCK_CONST_METHOD0(get_all_scopes, std::vector<std::string>&());
    MOCK_METHOD0(reload_config_impl, void(void));
    virtual void reload_config() noexcept
    {
        reload_config_impl();
    }

    void set_default_expectations()
    {
        EXPECT_CALL(*this, get_global_setting(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, set_global_config(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, client_exists(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_client(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_all_scopes())
            .Times(0);
        EXPECT_CALL(*this, reload_config_impl())
            .Times(0);
    }

    void expect_global_setting_read(settings_store_i::global_setting key, std::string const &value, bool optional = false)
    {
        EXPECT_CALL(*this, get_global_setting(key))
            .Times(::testing::AtLeast(optional ? 0 : 1))
            .WillRepeatedly(
                testing::ReturnRef(value)
            );
    }

    void expect_global_setting_write(settings_store_i::global_config_map const &new_values, configuration_error_map &error_messages, bool optional = false)
    {
        EXPECT_CALL(*this, set_global_config(new_values, ::testing::_))
            .Times(::testing::AtLeast(optional ? 0 : 1))
            .WillRepeatedly(WithArg<1>(Invoke([&error_messages](configuration_error_map &error_message_out){
                error_message_out = error_messages;
                return error_messages.empty();
            })));
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_SETTINGS_STORE_HPP_
//---- End of source file ------------------------------------------------------
