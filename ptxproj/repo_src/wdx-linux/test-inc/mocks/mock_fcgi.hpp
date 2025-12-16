//------------------------------------------------------------------------------
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
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
///  \brief    Mock for FCGI interface (system abstraction).
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_FCGI_HPP_
#define TEST_INC_MOCKS_MOCK_FCGI_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction_serv.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using wago::wdx::linuxos::serv::sal::fcgi;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
using ::testing::Exactly;
using ::testing::AnyNumber;
using ::testing::WithoutArgs;
using ::testing::Invoke;
using ::testing::Return;

GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_fcgi : public fcgi
{
protected:
    char *env_element1 = nullptr;
    char *environment[1] = { env_element1 };
    FCGX_Request *init_request = nullptr;
    bool use_of_puts = false;

private:
    fcgi *saved_instance;

public:
    static void set_instance(fcgi *new_instance)
    {
        instance = new_instance;
    }

    mock_fcgi()
    {
        saved_instance = &get_instance();
        set_instance(this);
    }
    ~mock_fcgi()
    {
        set_instance(saved_instance);

        // FCGX_PutS() is implemented with FCGX_PutStr() and strlen()
        // See: https://fossies.org/dox/fcgi-2.4.0/fcgiapp_8c_source.html
        EXPECT_FALSE(use_of_puts) << "Avoid use of (FCGX_)PutS, use (FCGX_)PutStr instead";
    }

    MOCK_METHOD(int, Init, (), (noexcept));
    int Init_followers()
    {
        EXPECT_CALL(*this, ShutdownPending())
            .Times(Exactly(1))
            .RetiresOnSaturation();

        return 0;
    }

    MOCK_METHOD(int, OpenSocket, (char const *path,
                                  int         backlog), (noexcept));

    MOCK_METHOD(void, ShutdownPending, (), (noexcept));

    MOCK_METHOD(int, InitRequest, (FCGX_Request *request,
                                   int           sock,
                                   int           flags), (noexcept));
    int InitRequest_followers(FCGX_Request *request,
                              int WC_UNUSED_PARAM(sock),
                              int WC_UNUSED_PARAM(flags))
    {
        memset(request, 0, sizeof(FCGX_Request));
        request->envp = environment;
        init_request = request;
        EXPECT_CALL(*this, Free(request, ::testing::_))
            .Times(Exactly(1))
            .RetiresOnSaturation();
        EXPECT_CALL(*this, Finish_r(request))
            .Times(AnyNumber());

        return 0;
    }

    MOCK_METHOD(int, Accept_r, (FCGX_Request *request), (noexcept));

    MOCK_METHOD(char *, GetParam, (char const      *name,
                                   FCGX_ParamArray  envp), (noexcept));

    MOCK_METHOD(char *, GetLine, (char        *str,
                                  int          n,
                                  FCGX_Stream *stream), (noexcept));

    MOCK_METHOD(int, GetStr, (char        *str,
                              int          n,
                              FCGX_Stream *stream), (noexcept));

    MOCK_METHOD(int, PutS, (char const  *str,
                            FCGX_Stream *stream), (noexcept));
    int PutS_followers()
    {
        use_of_puts = true;
        return 0;
    }

    MOCK_METHOD(int, PutStr, (char const  *str,
                              int          n,
                              FCGX_Stream *stream), (noexcept));

    MOCK_METHOD(int, FFlush, (FCGX_Stream *stream), (noexcept));

    MOCK_METHOD(void, Finish_r, (FCGX_Request *request), (noexcept));

    MOCK_METHOD(void, Free, (FCGX_Request *request,
                             int           close), (noexcept));

    MOCK_METHOD(int, HasSeenEOF, (FCGX_Stream *stream), (noexcept));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, Init())
            .Times(AnyNumber())
            .WillRepeatedly(WithoutArgs(Invoke(this, &mock_fcgi::Init_followers)));
        EXPECT_CALL(*this, OpenSocket(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, InitRequest(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, Accept_r(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, GetParam(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, GetParam(::testing::StrEq("REMOTE_ADDR"), ::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(nullptr));
        EXPECT_CALL(*this, GetParam(::testing::StrEq("REMOTE_PORT"), ::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(nullptr));
        EXPECT_CALL(*this, GetParam(::testing::StrEq("REQUEST_METHOD"), ::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(const_cast<char *>("GET")));
        EXPECT_CALL(*this, GetLine(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, GetStr(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, PutS(::testing::_, ::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(WithoutArgs(Invoke(this, &mock_fcgi::PutS_followers)));
        EXPECT_CALL(*this, PutStr(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, FFlush(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, Finish_r(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, Free(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, HasSeenEOF(::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_FCGI_HPP_
//---- End of source file ------------------------------------------------------
