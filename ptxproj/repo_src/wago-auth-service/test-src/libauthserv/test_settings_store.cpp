//------------------------------------------------------------------------------
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
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
///  \brief    Test authd settings store
///
///  \author   FHa : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "settings_store.hpp"
#include "mocks/mock_filesystem.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::authserv::settings_store_i;
using wago::authserv::settings_store;
using testing::AnyNumber;
using testing::Throw;
using testing::AtLeast;
using testing::Return;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------

TEST(settings_store_i, test_for_valid_grant_type) {
    settings_store_i::oauth_client client = {"myId", "myLabel", "myRedirect", {"myGrantType"}};
    EXPECT_TRUE(client.supports_grant_type("myGrantType"));
}

TEST(settings_store_i, test_for_mutliple_grant_types) {
    settings_store_i::oauth_client client = {"myId", "myLabel", "myRedirect", {"myGrantType", "myGrantType2"}};
    EXPECT_TRUE(client.supports_grant_type("myGrantType"));
    EXPECT_TRUE(client.supports_grant_type("myGrantType2"));
}

TEST(settings_store_i, test_for_invalid_grant_types) {
    settings_store_i::oauth_client client = {"myId", "myLabel", "myRedirect", {"myGrantType"}};
    EXPECT_FALSE(client.supports_grant_type("invalidGrantType"));
}

class settings_store_fixture: public ::testing::Test
{
protected:
    mock_filesystem filesystem_mock;

    settings_store_fixture() = default;
    ~settings_store_fixture() override = default;

    void SetUp() override
    {
        filesystem_mock.set_default_expectations();
    }

    void set_doesnt_exists_as_default()
    {
        EXPECT_CALL(filesystem_mock, open_stream(::testing::_, ::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(Throw(std::runtime_error("TEST: Failed to open file.")));
        EXPECT_CALL(filesystem_mock, list_directory(::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(Throw(std::runtime_error("TEST: Couldn't open directory")));
    }
};

TEST_F(settings_store_fixture, use_default_when_no_global_config_present)
{
    set_doesnt_exists_as_default();
    settings_store settings_store_obj;
    // expect defaults
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime),      settings_store_i::global_setting_defaults[settings_store_i::auth_code_lifetime]);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::access_token_lifetime),   settings_store_i::global_setting_defaults[settings_store_i::access_token_lifetime]);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::refresh_token_lifetime),  settings_store_i::global_setting_defaults[settings_store_i::refresh_token_lifetime]);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::system_use_notification), settings_store_i::global_setting_defaults[settings_store_i::system_use_notification]);}

TEST_F(settings_store_fixture, no_clients_and_resource_server_folder)
{
    set_doesnt_exists_as_default();
    filesystem_mock.expect_file_read("/etc/authd/authd.conf",
        "# Test settings for authd\n"
        "# stored in /etc/authd/authd.conf\n"
        "\n"
        "auth_code.lifetime = 1\n"
        "access_token.lifetime = 2\n"
        "refresh_token.lifetime = 4\n"
        "system_use_notification = My notification\n"
        "");

    settings_store settings_store_obj;
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime),      "1");
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::access_token_lifetime),   "2");
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::refresh_token_lifetime),  "4");
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::system_use_notification), "My notification");
    EXPECT_TRUE(settings_store_obj.get_all_scopes().empty());
}

TEST_F(settings_store_fixture, no_values_in_config)
{
    set_doesnt_exists_as_default();
    filesystem_mock.expect_file_read("/etc/authd/authd.conf",
        "# Test settings for authd\n"
        "# stored in /etc/authd/authd.conf\n"
        "");

    settings_store settings_store_obj;
    // expect defaults
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime),      settings_store_i::global_setting_defaults[settings_store_i::auth_code_lifetime]);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::access_token_lifetime),   settings_store_i::global_setting_defaults[settings_store_i::access_token_lifetime]);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::refresh_token_lifetime),  settings_store_i::global_setting_defaults[settings_store_i::refresh_token_lifetime]);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::system_use_notification), settings_store_i::global_setting_defaults[settings_store_i::system_use_notification]);
    EXPECT_TRUE(settings_store_obj.get_all_scopes().empty());
}

TEST_F(settings_store_fixture, invalid_values_in_config)
{
    set_doesnt_exists_as_default();
    filesystem_mock.expect_file_read("/etc/authd/authd.conf",
        "# Test settings for authd\n"
        "# stored in /etc/authd/authd.conf\n"
        "auth_code.lifetime = 2\n"
        "access_token.lifetime = 2\n"
        "");

    settings_store settings_store_obj;
    // expect defaults
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime),      settings_store_i::global_setting_defaults[settings_store_i::auth_code_lifetime]);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::access_token_lifetime),   settings_store_i::global_setting_defaults[settings_store_i::access_token_lifetime]);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::refresh_token_lifetime),  settings_store_i::global_setting_defaults[settings_store_i::refresh_token_lifetime]);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::system_use_notification), settings_store_i::global_setting_defaults[settings_store_i::system_use_notification]);
    EXPECT_TRUE(settings_store_obj.get_all_scopes().empty());
}

TEST_F(settings_store_fixture, invalid_keys_in_config)
{
    set_doesnt_exists_as_default();
    filesystem_mock.expect_file_read("/etc/authd/authd.conf",
        "# Test settings for authd\n"
        "# stored in /etc/authd/authd.conf\n"
        "this_key_doesnt_exist = 1\n"
        "this_neither = 1\n"
        "auth_code.lifetime = 1\n"
        "access_token.lifetime = 10\n"
        "");

    settings_store settings_store_obj;
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime),      "1");
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::access_token_lifetime),   "10");
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::refresh_token_lifetime),  settings_store_i::global_setting_defaults[settings_store_i::refresh_token_lifetime]);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::system_use_notification), settings_store_i::global_setting_defaults[settings_store_i::system_use_notification]);
    EXPECT_TRUE(settings_store_obj.get_all_scopes().empty());
}

TEST_F(settings_store_fixture, read_resource_server_config)
{
    set_doesnt_exists_as_default();
    filesystem_mock.expect_file_read("/etc/authd/authd.conf", "");
    filesystem_mock.expect_directory_list("/etc/authd/resource_servers", { "myResourceServer.conf" });
    filesystem_mock.expect_file_read("/etc/authd/resource_servers/myResourceServer.conf", ""
        "label = My Resource Server\n" // Can't be accessed yet. Only Scopes needed in the moment
        "scopes = test:scope"
        "");

    settings_store settings_store_obj;
    EXPECT_EQ(settings_store_obj.get_all_scopes(), std::vector<std::string>{"myResourceServer:test:scope"});
}

TEST_F(settings_store_fixture, read_client_config)
{
    set_doesnt_exists_as_default();
    filesystem_mock.expect_file_read("/etc/authd/authd.conf", "");
    filesystem_mock.expect_directory_list("/etc/authd/clients", { "myClient.conf" });
    filesystem_mock.expect_file_read("/etc/authd/clients/myClient.conf", ""
        "label = My Client\n"
        "redirect_uri = host.td/redirect\n"
        "grant_types = grant_type\n"
        "");

    settings_store settings_store_obj;
    EXPECT_TRUE(settings_store_obj.client_exists("myClient"));
    auto client = settings_store_obj.get_client("myClient");
    EXPECT_EQ(client.id, "myClient");
    EXPECT_EQ(client.label, "My Client");
    EXPECT_EQ(client.redirect_uri, "host.td/redirect");
    EXPECT_EQ(client.grant_types, std::vector<std::string>{"grant_type"});
    EXPECT_TRUE(settings_store_obj.get_all_scopes().empty());
}

TEST_F(settings_store_fixture, ignore_non_config_files_in_client_folder)
{
    set_doesnt_exists_as_default();
    filesystem_mock.expect_file_read("/etc/authd/authd.conf", "");
    filesystem_mock.expect_directory_list("/etc/authd/clients", { "myClient.conf", "~myClient.conf", "otherClient", "wrong.ending"});
    filesystem_mock.expect_file_read("/etc/authd/clients/myClient.conf", ""
        "label = My Client\n"
        "redirect_uri = test/url"
        "");
    EXPECT_CALL(filesystem_mock, is_regular_file("/etc/authd/clients/~myClient.conf")).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(filesystem_mock, is_regular_file("/etc/authd/clients/otherClient")).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(filesystem_mock, is_regular_file("/etc/authd/clients/wrong.ending")).WillRepeatedly(testing::Return(true));

    settings_store settings_store_obj;
    EXPECT_FALSE(settings_store_obj.client_exists("~myClient"));
    EXPECT_FALSE(settings_store_obj.client_exists("otherClient"));
    EXPECT_FALSE(settings_store_obj.client_exists("wrong"));
    EXPECT_FALSE(settings_store_obj.client_exists("wrong.ending"));
    EXPECT_TRUE(settings_store_obj.client_exists("myClient"));
    auto client = settings_store_obj.get_client("myClient");
    EXPECT_EQ(client.id, "myClient");
    EXPECT_EQ(client.label, "My Client");
    EXPECT_EQ(client.redirect_uri, "test/url");
    EXPECT_TRUE(client.grant_types.empty());
    EXPECT_TRUE(settings_store_obj.get_all_scopes().empty());
}

TEST_F(settings_store_fixture, reload_settings)
{
    set_doesnt_exists_as_default();
    filesystem_mock.expect_file_read("/etc/authd/authd.conf",
        "# Test settings for authd\n"
        "# stored in /etc/authd/authd.conf\n"
        "\n"
        "auth_code.lifetime = 1\n"
        "access_token.lifetime = 2\n"
        "refresh_token.lifetime = 4\n"
        "system_use_notification = My notification\n"
        "");

    settings_store settings_store_obj;
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime),      "1");
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::access_token_lifetime),   "2");
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::refresh_token_lifetime),  "4");
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::system_use_notification), "My notification");
    EXPECT_FALSE(settings_store_obj.client_exists("myClient"));
    EXPECT_TRUE(settings_store_obj.get_all_scopes().empty());

    // new settings for reload

    filesystem_mock.expect_file_read("/etc/authd/authd.conf",
        "# Test settings for authd\n"
        "# stored in /etc/authd/authd.conf\n"
        "\n"
        "auth_code.lifetime = 10\n"
        "access_token.lifetime = 20\n"
        "refresh_token.lifetime = 40\n"
        "system_use_notification = My new notification\n"
        "");
    filesystem_mock.expect_directory_list("/etc/authd/clients", {"myClient.conf"});
    filesystem_mock.expect_file_read("/etc/authd/clients/myClient.conf",
        "label = My Client\n"
        "redirect_uri = test/url\n"
        "");
    filesystem_mock.expect_directory_list("/etc/authd/resource_servers", { "myResourceServer.conf" });
    filesystem_mock.expect_file_read("/etc/authd/resource_servers/myResourceServer.conf",
        "scopes = test:scope"
        "");

    settings_store_obj.reload_config();
    // global
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime),      "10");
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::access_token_lifetime),   "20");
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::refresh_token_lifetime),  "40");
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::system_use_notification), "My new notification");
    // client
    EXPECT_TRUE(settings_store_obj.client_exists("myClient"));
    EXPECT_EQ(settings_store_obj.get_client("myClient").label, "My Client");
    // resource server
    EXPECT_EQ(settings_store_obj.get_all_scopes(), std::vector<std::string>{"myResourceServer:test:scope"});
}

TEST_F(settings_store_fixture, set_new_config)
{
    // create settings_store with default values
    set_doesnt_exists_as_default();
    settings_store settings_store_obj;
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime),
              settings_store_i::global_setting_defaults[settings_store_i::auth_code_lifetime]);

    filesystem_mock.expect_file_read("/etc/authd/authd.conf", "");
    filesystem_mock.expect_file_write("/etc/authd/authd.conf.tmp");
    EXPECT_CALL(filesystem_mock, rename(testing::_, testing::_)).Times(testing::AtLeast(1));

    settings_store_i::global_config_map new_values;
    new_values.emplace(settings_store_i::auth_code_lifetime, "100");
    settings_store_i::configuration_error_map error_messages;
    bool res = settings_store_obj.set_global_config(new_values, error_messages);

    EXPECT_TRUE(res);
    EXPECT_TRUE(error_messages.empty());
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime), "100");
}

TEST_F(settings_store_fixture, reject_config_with_invalid_lifetimes)
{
    // create settings_store with default values
    set_doesnt_exists_as_default();
    settings_store settings_store_obj;
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime),
              settings_store_i::global_setting_defaults[settings_store_i::auth_code_lifetime]);

    settings_store_i::global_config_map new_values;
    new_values.emplace(settings_store_i::auth_code_lifetime, "60");
    new_values.emplace(settings_store_i::access_token_lifetime, "60");
    new_values.emplace(settings_store_i::refresh_token_lifetime, "240");
    settings_store_i::configuration_error_map error_messages;
    bool res = settings_store_obj.set_global_config(new_values, error_messages);

    EXPECT_FALSE(res);
    EXPECT_FALSE(error_messages.empty());
    EXPECT_TRUE(error_messages.count(settings_store_i::auth_code_lifetime) > 0);
    EXPECT_TRUE(error_messages.count(settings_store_i::access_token_lifetime) > 0);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime),
              settings_store_i::global_setting_defaults[settings_store_i::auth_code_lifetime]);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::refresh_token_lifetime),
              settings_store_i::global_setting_defaults[settings_store_i::refresh_token_lifetime]);
}

TEST_F(settings_store_fixture, reject_config_with_lifetime_0)
{
    // create settings_store with default values
    set_doesnt_exists_as_default();
    settings_store settings_store_obj;
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime),
              settings_store_i::global_setting_defaults[settings_store_i::auth_code_lifetime]);

    settings_store_i::global_config_map new_values;
    new_values.emplace(settings_store_i::auth_code_lifetime, "0");
    new_values.emplace(settings_store_i::access_token_lifetime, "60");
    settings_store_i::configuration_error_map error_messages;
    bool res = settings_store_obj.set_global_config(new_values, error_messages);

    EXPECT_FALSE(res);
    EXPECT_FALSE(error_messages.empty());
    EXPECT_TRUE(error_messages.count(settings_store_i::auth_code_lifetime) > 0);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::auth_code_lifetime),
              settings_store_i::global_setting_defaults[settings_store_i::auth_code_lifetime]);
    EXPECT_EQ(settings_store_obj.get_global_setting(settings_store_i::access_token_lifetime),
              settings_store_i::global_setting_defaults[settings_store_i::access_token_lifetime]);
}

//---- End of source file ------------------------------------------------------

