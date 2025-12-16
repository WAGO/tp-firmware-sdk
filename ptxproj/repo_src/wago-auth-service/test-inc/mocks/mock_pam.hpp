//------------------------------------------------------------------------------
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
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
///  \brief    Mock for pam library.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_PAM_HPP_
#define TEST_INC_MOCKS_MOCK_PAM_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using wago::authserv::sal::pam;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
using ::testing::Exactly;
using ::testing::AnyNumber;
using ::testing::WithArgs;
using ::testing::Invoke;

GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_pam : public pam
{
private:
    pam *saved_instance;

public:
    pam_handle_t       * pam_dummy_m        = (pam_handle_t *)this;
    pam_conv     const * pam_conversation_m = nullptr;
    
    static void set_instance(pam *new_instance)
    {
        instance = new_instance;
    }

    mock_pam()
    {
        saved_instance = &get_instance();
        set_instance(this);
    }
    ~mock_pam()
    {
        set_instance(saved_instance);
    }

    MOCK_METHOD(int, start, (char         const  *service_name,
                             char         const  *user,
                             pam_conv     const  *pam_conversation,
                             pam_handle_t       **pamh), (const, noexcept));
    int pam_start_followers(char         const  *service_name,
                            char         const  *user,
                            pam_conv     const  *pam_conversation,
                            pam_handle_t       **pamh)
    {
        EXPECT_NE(nullptr, service_name);
        EXPECT_NE(nullptr, user);
        EXPECT_STRNE("", service_name);
        EXPECT_STRNE("", user);
        EXPECT_NE(nullptr, pam_conversation->appdata_ptr);
        EXPECT_NE(nullptr, pam_conversation->conv);
        EXPECT_CALL(*this, end(pam_dummy_m, ::testing::_))
            .Times(Exactly(1))
            .RetiresOnSaturation();
        *pamh              = pam_dummy_m;
        pam_conversation_m = pam_conversation;
        return PAM_SUCCESS;
    }
    MOCK_METHOD(int, end, (pam_handle_t *pamh,
                           int           pam_status), (const, noexcept));
    MOCK_METHOD(int, authenticate, (pam_handle_t *pamh,
                                    int           flags), (const, noexcept));
    int pam_authenticate_conversation() noexcept
    {
        pam_message  const   pam_message_1  = { PAM_PROMPT_ECHO_ON, "user" };
        pam_message  const   pam_message_2  = { PAM_PROMPT_ECHO_OFF, "password" };
        pam_message  const * pam_messages[] = { &pam_message_1, &pam_message_2 };
        pam_response       * pam_response   = nullptr;
        int const result = pam_conversation_m->conv(2, pam_messages, &pam_response, pam_conversation_m->appdata_ptr);
        if(pam_response != nullptr)
        {
            if(pam_response[0].resp != nullptr)
            {
                free(pam_response[0].resp);
            }
            if(pam_response[1].resp != nullptr)
            {
                free(pam_response[1].resp);
            }
            free(pam_response);
        }
        return result;
    }
    int pam_authenticate_conversation_error() noexcept
    {
        pam_message  const   pam_message_1  = { PAM_PROMPT_ECHO_ON, "user" };
        pam_message  const   pam_message_2  = { PAM_PROMPT_ECHO_ON + PAM_PROMPT_ECHO_OFF, "garbage" };
        pam_message  const * pam_messages[] = { &pam_message_1, &pam_message_2 };
        pam_response       * pam_response   = nullptr;
        int const result = pam_conversation_m->conv(2, pam_messages, &pam_response, pam_conversation_m->appdata_ptr);
        if(pam_response != nullptr)
        {
            if(pam_response[0].resp != nullptr)
            {
                free(pam_response[0].resp);
            }
            if(pam_response[1].resp != nullptr)
            {
                free(pam_response[1].resp);
            }
            free(pam_response);
        }
        return result;
    }
    MOCK_METHOD(int, acct_mgmt, (pam_handle_t *pamh,
                                 int           flags), (const, noexcept));
    MOCK_METHOD(const char *, strerror, (pam_handle_t *pamh,
                                         int           errnum), (const, noexcept));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, start(::testing::_, ::testing::_, ::testing::_, ::testing::Pointee(nullptr)))
            .Times(AnyNumber())
            .WillRepeatedly(WithArgs<0, 1, 2, 3>(Invoke(this, &mock_pam::pam_start_followers)));
        EXPECT_CALL(*this, end(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, authenticate(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, acct_mgmt(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, strerror(::testing::_, ::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_PAM_HPP_
//---- End of source file ------------------------------------------------------
