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
///  \brief    Test authorize endpoint implementation.
///
///  \author   FHa: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "response_helper.hpp"

#include "mocks/mock_request.hpp"
#include "mocks/mock_settings_store.hpp"
#include "mocks/mock_hostname.hpp"
#include "mocks/mock_filesystem.hpp"

#include "wago/authserv/http/http_status_code.hpp"
#include "definitions.hpp"

#include <string>
#include <memory>
#include <gtest/gtest.h>


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::authserv;
using namespace wago::authserv::http;
using wago::authserv::settings_store_i;
using wago::authserv::response_helper_i;
using wago::authserv::response_helper;
using testing::Throw;
using testing::WithArg;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------

class response_helper_fixture: public ::testing::Test
{
protected:
    mock_request                         request_mock;
    std::shared_ptr<mock_settings_store> settings_store_mock;
    mock_hostname                        hostname_mock;
    mock_filesystem                      filesystem_mock;

    std::string                          service_name;
    std::string                          hostname;

    // request parameters
    std::string test_request_client_label;
    std::string username;
    std::string test_cancel_uri;

    // settings
    std::string test_setting_silent_mode_enabled;
    std::string test_setting_system_use_notification;

    // generated values
    std::string test_initial_setup_template;
    std::string test_login_template;
    std::string test_password_change_template;
    std::string test_confirmation_template;

    // expected result
    std::string expected_hostname;
    std::string expected_service_name;
    std::string expected_service_version;
    std::string expected_api_version;
    std::string expected_username;
    std::string expected_client_label;
    std::string expected_cancel_uri;
    std::string expected_silent_mode_class;
    std::string expected_system_use_notification;

public:
    response_helper_fixture()
    : settings_store_mock(std::make_shared<mock_settings_store>())
    , service_name("test_service")
    , hostname("test_hostname")
    , test_request_client_label("my_client")
    , username("testuser")
    , test_cancel_uri("cancel_uri")

    , test_setting_silent_mode_enabled("true")
    , test_setting_system_use_notification("system use notification test")

    , test_initial_setup_template("setup template without testing parameters")
    , test_login_template("I'm a login template! '$$HOSTNAME$$' '$$SERVICE_NAME$$' '$$SERVICE_VERSION$$' '$$API_VERSION$$' '$$CLIENT_NAME$$' '$$CANCEL_URI$$' '$$ERROR_MESSAGE$$' '$$SUCCESS_MESSAGE$$' '$$SILENT_MODE_CLASS$$' '$$SYSTEM_USE_NOTIFICATION$$'")
    , test_password_change_template("I'm a password change template! '$$HOSTNAME$$' '$$SERVICE_NAME$$' '$$SERVICE_VERSION$$' '$$API_VERSION$$' '$$USER_NAME$$' '$$CANCEL_URI$$' '$$ERROR_MESSAGE$$' '$$INFO_MESSAGE$$' '$$SILENT_MODE_CLASS$$'")
    , test_confirmation_template("I'm a confirmation template! '$$HOSTNAME$$' '$$SERVICE_NAME$$' '$$SERVICE_VERSION$$' '$$API_VERSION$$' '$$CONTINUE_URI$$' '$$ERROR_MESSAGE$$' '$$SUCCESS_MESSAGE$$' '$$SILENT_MODE_CLASS$$'")

    , expected_hostname(hostname)
    , expected_service_name(service_name)
    , expected_service_version(WC_SUBST_STR(AUTHSERV_VERSION))
    , expected_api_version(WC_SUBST_STR(AUTH_API_VERSION))
    , expected_username(username)
    , expected_client_label(test_request_client_label)
    , expected_cancel_uri(test_cancel_uri)
    , expected_silent_mode_class("silent-mode")
    , expected_system_use_notification(test_setting_system_use_notification)
    {}

    void SetUp() override
    {
        request_mock.set_default_expectations();
        settings_store_mock->set_default_expectations();
        hostname_mock.set_default_expectations();
        filesystem_mock.set_default_expectations();

        // read of default files
        filesystem_mock.expect_file_read(password_setup_page_file_path, test_initial_setup_template);
        filesystem_mock.expect_file_read(login_page_file_path, test_login_template);
        filesystem_mock.expect_file_read(password_change_page_file_path, test_password_change_template);
        filesystem_mock.expect_file_read(confirmation_page_file_path, test_confirmation_template);
    }

    void expect_information_collection_for_html_page(bool contains_system_use_notification = false)
    {
        hostname_mock.expect_hostname_read(hostname);
        settings_store_mock->expect_global_setting_read(settings_store_i::silent_mode_enabled, test_setting_silent_mode_enabled);
        if (contains_system_use_notification) 
        {
            settings_store_mock->expect_global_setting_read(settings_store_i::system_use_notification, test_setting_system_use_notification);
        }
    }

    void expect_login_response(http_status_code expected_http_status     = http_status_code::ok,
                               std::string      expected_error_message   = "",
                               std::string      expected_success_message = "")
    {
        expect_information_collection_for_html_page(true);
        std::string expected_body = "I'm a login template! '" + expected_hostname + "' '" + expected_service_name + "' '" + expected_service_version + "' '" + expected_api_version + "' '" + expected_client_label + "' '" + expected_cancel_uri + "' '" + expected_error_message + "' '" + expected_success_message + "' '" + expected_silent_mode_class + "' '" + expected_system_use_notification + "'";
        expect_response(expected_body, expected_http_status);
    }

    void expect_password_change_response(http_status_code expected_http_status   = http_status_code::ok,
                                         std::string      expected_error_message = "",
                                         std::string      expected_info_message = "")
    {
        expect_information_collection_for_html_page();
        std::string expected_body = "I'm a password change template! '" + expected_hostname + "' '" + expected_service_name + "' '" + expected_service_version + "' '" + expected_api_version + "' '" + expected_username + "' '" + expected_cancel_uri + "' '" + expected_error_message + "' '" + expected_info_message + "' '" + expected_silent_mode_class + "'";
        expect_response(expected_body, expected_http_status);
    }

    void expect_confirmation_response(http_status_code expected_http_status     = http_status_code::ok,
                                      std::string      expected_success_message = "",
                                      std::string      expected_error_message   = "")
    {
        expect_information_collection_for_html_page();
        std::string expected_body = "I'm a confirmation template! '" + expected_hostname + "' '" + expected_service_name + "' '" + expected_service_version + "' '" + expected_api_version + "' '" + expected_cancel_uri + "' '" + expected_error_message + "' '" + expected_success_message + "' '" + expected_silent_mode_class + "'";
        expect_response(expected_body, expected_http_status);
    }

    void expect_response(std::string      expected_body,
                         http_status_code expected_http_status)
    {
        if (expected_body != "")
        {
            EXPECT_CALL(request_mock, send_data(testing::StrEq(expected_body.c_str()), expected_body.size()))
                .Times(Exactly(1));
        }

        EXPECT_CALL(request_mock, respond_mock(::testing::_))
            .Times(Exactly(1))
            .WillOnce(testing::Invoke([expected_http_status](response_i const &response) {
                EXPECT_EQ(expected_http_status, response.get_status_code());
            }));

        EXPECT_CALL(request_mock, finish())
            .Times(AtMost(1));
    }
};

TEST_F(response_helper_fixture, html_escaping_on_login_page)
{
    // change all parameters that need to be escaped
    hostname = "<script> evilHostname() </script>";
    expected_hostname = "&lt;script&gt; evilHostname() &lt;/script&gt;";

    test_request_client_label = "<script> evilLabel() </script>";
    expected_client_label = "&lt;script&gt; evilLabel() &lt;/script&gt;";

    test_setting_system_use_notification = "<script> evilMessage() </script>";
    expected_system_use_notification = "&lt;script&gt; evilMessage() &lt;/script&gt;";

    expect_login_response();
    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_login_page(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            test_request_client_label
        );
    }
}

TEST_F(response_helper_fixture, html_escaping_on_password_change_page)
{
    // change all parameters that need to be escaped
    service_name = "<script> evilServiceName() </script>";
    expected_service_name = "&lt;script&gt; evilServiceName() &lt;/script&gt;";

    hostname = "<script> evilHostname() </script>";
    expected_hostname = "&lt;script&gt; evilHostname() &lt;/script&gt;";

    username = "<script> evilUsername() </script>";
    expected_username = "&lt;script&gt; evilUsername() &lt;/script&gt;";

    expect_password_change_response();
    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_password_change_page(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            username
        );
    }
}

TEST_F(response_helper_fixture, html_escaping_on_success_page)
{
    // change all parameters that need to be escaped
    service_name = "<script> evilServiceName() </script>";
    expected_service_name = "&lt;script&gt; evilServiceName() &lt;/script&gt;";

    hostname = "<script> evilHostname() </script>";
    expected_hostname = "&lt;script&gt; evilHostname() &lt;/script&gt;";

    std::string success_message = "test success message";
    expect_confirmation_response(http_status_code::ok, success_message);
    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_success_confirmation(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            success_message
        );
    }
}

TEST_F(response_helper_fixture, html_escaping_on_error_page)
{
    // change all parameters that need to be escaped
    service_name = "<script> evilServiceName() </script>";
    expected_service_name = "&lt;script&gt; evilServiceName() &lt;/script&gt;";

    hostname = "<script> evilHostname() </script>";
    expected_hostname = "&lt;script&gt; evilHostname() &lt;/script&gt;";

    std::string error_message = "test error message";
    expect_confirmation_response(http_status_code::ok, "", error_message);
    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_error_confirmation(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            response_helper_i::no_error,
            error_message
        );
    }
}

TEST_F(response_helper_fixture, silent_mode_enabled_true)
{
    test_setting_silent_mode_enabled = "true";
    expected_silent_mode_class = "silent-mode";

    expect_login_response();
    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_login_page(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            test_request_client_label
        );
    }
}

TEST_F(response_helper_fixture, silent_mode_enabled_false)
{
    test_setting_silent_mode_enabled = "false";
    expected_silent_mode_class = "";

    expect_login_response();
    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_login_page(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            test_request_client_label
        );
    }
}

TEST_F(response_helper_fixture, linebreak_in_system_use_notification)
{
    test_setting_system_use_notification = "This is a info message.\nWritten in the config file.\\nIt should have two linebreaks.";
    expected_system_use_notification = "This is a info message.<br>Written in the config file.<br>It should have two linebreaks.";

    expect_login_response();
    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_login_page(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            test_request_client_label
        );
    }
}

TEST_F(response_helper_fixture, empty_system_use_notification_no_file)
{
    test_setting_system_use_notification = "";
    expected_system_use_notification = "";

    EXPECT_CALL(filesystem_mock, is_regular_file(system_use_notification_file))
        .Times(AtLeast(1))
        .WillRepeatedly(
            Return(false)
        );
    expect_login_response();

    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_login_page(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            test_request_client_label
        );
    }
}

TEST_F(response_helper_fixture, empty_system_use_notification_empty_file)
{
    // no system_use_notification configured
    test_setting_system_use_notification = "";
    // read from file instead
    char const file_system_use_notification[] = "";
    filesystem_mock.expect_file_read(system_use_notification_file, file_system_use_notification);

    expected_system_use_notification = "";

    expect_login_response();
    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_login_page(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            test_request_client_label
        );
    }
}

TEST_F(response_helper_fixture, system_use_notification_from_file)
{
    // no system_use_notification configured
    test_setting_system_use_notification = "";
    // read from file instead
    char const file_system_use_notification[] = "Some simple\nSystem Use Notification";
    filesystem_mock.expect_file_read(system_use_notification_file, file_system_use_notification);
    // and expect the system use notification to be equal to the file
    expected_system_use_notification = "Some simple<br>System Use Notification";

    expect_login_response();
    {response_helper templater(service_name, settings_store_mock);
        templater.send_login_page(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            test_request_client_label
        );
    }
}

TEST_F(response_helper_fixture, system_use_notification_from_file_whitespace_trimmed)
{
    // no system_use_notification configured
    test_setting_system_use_notification = "";
    // read from file instead
    char const file_system_use_notification[] = "\t \nSome simple\nSystem Use Notification\t \n";
    filesystem_mock.expect_file_read(system_use_notification_file, file_system_use_notification);
    // and expect the system use notification to be equal to the file but stripped
    expected_system_use_notification = "Some simple<br>System Use Notification";

    expect_login_response();
    {response_helper templater(service_name, settings_store_mock);
        templater.send_login_page(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            test_request_client_label
        );
    }
}

TEST_F(response_helper_fixture, system_use_notification_from_file_overflow)
{
    // no system_use_notification configured
    test_setting_system_use_notification = "";

    // read from file instead
    char   const overflow_text[]      = "Overflow text for System Use Notification to exceed maximum length";
    size_t const overflow_text_length = sizeof(overflow_text) - 1;
    char   const repeat_text[]        = "This System Use Notification test text is exactly 64 bytes long|";
    size_t const repeat_text_length   = sizeof(repeat_text) - 1;
    ASSERT_GT(overflow_text_length, repeat_text_length);

    // Build oversized System Use Notification
    std::string file_system_use_notification;
    file_system_use_notification.reserve(system_use_notification_max + overflow_text_length);
    for(size_t i = 0; i < (system_use_notification_max / repeat_text_length); ++i)
    {
        file_system_use_notification += repeat_text;
    }
    ASSERT_GE(system_use_notification_max, file_system_use_notification.length());
    file_system_use_notification += overflow_text;
    ASSERT_LT(system_use_notification_max, file_system_use_notification.length());

    filesystem_mock.expect_file_read(system_use_notification_file, file_system_use_notification);
    
    // and expect the system use notification to be equal to the file but stripped
    expected_system_use_notification = file_system_use_notification.substr(0, system_use_notification_max);
    ASSERT_EQ(system_use_notification_max, expected_system_use_notification.length());

    expect_login_response();
    {response_helper templater(service_name, settings_store_mock);
        templater.send_login_page(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            test_request_client_label
        );
    }
}

TEST_F(response_helper_fixture, system_use_notification_file_error)
{
    // no system_use_notification configured
    test_setting_system_use_notification = "";
    
    // expect read from file and return that it isn't present 
    EXPECT_CALL(filesystem_mock, is_regular_file(system_use_notification_file))
    .Times(AtLeast(1))
    .WillRepeatedly(
        Return(true)
    );
    EXPECT_CALL(filesystem_mock, open_stream(system_use_notification_file, testing::_))
    .Times(AtLeast(1))
    .WillRepeatedly(
        Throw(std::runtime_error("Test exception on file load"))
    );
    
    // expect empty system use notification
    expected_system_use_notification = "";
    
    expect_login_response(http_status_code::ok, "Failed to load system use notification");
    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_login_page(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            test_request_client_label
        );
    }
}

TEST_F(response_helper_fixture, json_response_success_page)
{
    EXPECT_CALL(request_mock, get_accepted_types())
        .Times(AnyNumber())
        .WillRepeatedly(Return("application/json"));
    expect_response("", http_status_code::ok);

        EXPECT_CALL(request_mock, send_data(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<0>(Invoke([](std::string content) {
            EXPECT_THAT(content, ::testing::HasSubstr(
                "\"success\":true"
            ));
        })));
        
    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_success_confirmation(
            request_mock,
            test_cancel_uri,
            http_status_code::ok,
            "only for html"
        );
    }
}

TEST_F(response_helper_fixture, json_response_on_failed_password_change)
{
    EXPECT_CALL(request_mock, get_accepted_types())
        .Times(AnyNumber())
        .WillRepeatedly(Return("application/json"));
    expect_response("", http_status_code::bad_request);

    std::string error_message = "error";
    EXPECT_CALL(request_mock, send_data(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<0>(Invoke([error_message](std::string content) {
            EXPECT_THAT(content, ::testing::HasSubstr(
                "\"success\":false"
            ));

            EXPECT_THAT(content, ::testing::HasSubstr(
                "\"error\":\"invalid_username_or_password\""
            ));

            EXPECT_THAT(content, ::testing::HasSubstr(
                "\"error_description\":\"" + error_message + "\""
            ));
        })));
        
    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_password_change_page(
            request_mock,
            test_cancel_uri,
            http_status_code::bad_request,
            "only for html",
            response_helper_i::invalid_username_or_password,
            error_message
        );
    }
}

TEST_F(response_helper_fixture, json_response_error_page)
{
    EXPECT_CALL(request_mock, get_accepted_types())
        .Times(AnyNumber())
        .WillRepeatedly(Return("application/json"));
    expect_response("", http_status_code::bad_request);

    std::string error_message = "error";
    EXPECT_CALL(request_mock, send_data(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<0>(Invoke([error_message](std::string content) {
            EXPECT_THAT(content, ::testing::HasSubstr(
                "\"success\":false"
            ));

            EXPECT_THAT(content, ::testing::HasSubstr(
                "\"error\":\"invalid_request\""
            ));

            EXPECT_THAT(content, ::testing::HasSubstr(
                "\"error_description\":\"" + error_message + "\""
            ));
        })));

    {
        response_helper templater(service_name, settings_store_mock);
        templater.send_error_confirmation(
            request_mock,
            test_cancel_uri,
            http_status_code::bad_request,
            response_helper_i::invalid_request,
            error_message
        );
    }
}
