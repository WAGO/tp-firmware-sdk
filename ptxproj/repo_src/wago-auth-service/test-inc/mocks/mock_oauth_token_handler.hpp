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
///  \brief    Mock for oauth token handler (generate and validate tokens).
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_OAUTH_TOKEN_HANDLER_HPP_
#define TEST_INC_MOCKS_MOCK_OAUTH_TOKEN_HANDLER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/oauth_token_handler_i.hpp"

#include <wc/compiler.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::authserv::oauth_token_handler_i;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_oauth_token_handler: public oauth_token_handler_i
{
  public:
    MOCK_METHOD5(generate_auth_code, std::string(uint32_t    const  duration_s,
                                                 std::string const &client_id,
                                                 std::string const &scopes,
                                                 std::string const &user_name,
                                                 std::string const &code_challenge));
    MOCK_METHOD4(generate_access_token, std::string(uint32_t    const  duration_s,
                                                    std::string const &client_id,
                                                    std::string const &scopes,
                                                    std::string const &user_name));
    MOCK_METHOD4(generate_refresh_token, std::string(uint32_t    const  duration_s,
                                                     std::string const &client_id,
                                                     std::string const &scopes,
                                                     std::string const &user_name));
    MOCK_METHOD2(validate_auth_code, validation_result(std::string const &code,
                                                       std::string const &code_verifier));
    MOCK_METHOD1(validate_access_token, validation_result(std::string const &token));
    MOCK_METHOD1(validate_refresh_token, validation_result(std::string const &token));

    MOCK_METHOD1(revoke_token, void(std::string const &token));
    MOCK_METHOD0(revoke_all_tokens, void());
    MOCK_METHOD0(cleanup_blacklist_impl, void());
    virtual void cleanup_blacklist() noexcept override
    {
        cleanup_blacklist_impl();
    }


    void set_default_expectations()
    {
        EXPECT_CALL(*this, generate_auth_code(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, generate_access_token(::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, generate_refresh_token(::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, validate_auth_code(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, validate_access_token(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, validate_refresh_token(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, revoke_token(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, revoke_all_tokens())
            .Times(0);
        EXPECT_CALL(*this, cleanup_blacklist_impl())
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_OAUTH_TOKEN_HANDLER_HPP_
//---- End of source file ------------------------------------------------------
