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
///  \brief    Test model loader (WDM).
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "model_loader.hpp"
#include "mocks/mock_filesystem.hpp"

#include <gtest/gtest.h>

#include <ios>
#include <sstream>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::linuxos::serv;
using namespace wago::wdx;
using testing::Exactly;
using testing::Return;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const default_model_path[] = "/etc/unit test/model.json";
static constexpr char const default_model[]      = R"##({
                                                            "Version": "1.0.0",
                                                            "Features": []
                                                        })##";

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class model_loader_fixture : public ::testing::Test
{
protected:
    mock_filesystem filesystem_mock;

protected:
    model_loader_fixture() = default;
    ~model_loader_fixture() override = default;
    void SetUp() override
    {
        // Set default call expectations
        filesystem_mock.set_default_expectations();
    }
};

TEST_F(model_loader_fixture, construct_delete)
{
    model_loader loader(default_model_path);
}

TEST_F(model_loader_fixture, load_model)
{
    EXPECT_CALL(filesystem_mock, open_stream_proxy(default_model_path, std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(Return(new std::stringstream(default_model)));
    model_loader loader(default_model_path);
    auto future = loader.get_model_information();
    ASSERT_TRUE(future.ready());
    ASSERT_TRUE(future.has_value());
    auto response = future.get();
    EXPECT_FALSE(response.has_error());
    EXPECT_EQ((unsigned)status_codes::success, (unsigned)response.status);
    EXPECT_STREQ(default_model, response.wdm_content.c_str());
}

TEST_F(model_loader_fixture, load_model_fail)
{
    std::stringstream *failstream = new std::stringstream();
    failstream->setstate(std::ios_base::failbit);
    EXPECT_CALL(filesystem_mock, open_stream_proxy(default_model_path, std::ios::in))
        .Times(Exactly(1))
        .WillRepeatedly(Return(failstream));
    model_loader loader(default_model_path);
    auto future = loader.get_model_information();
    ASSERT_TRUE(future.ready());
    ASSERT_TRUE(future.has_value());
    auto response = future.get();
    EXPECT_TRUE(response.has_error());
    EXPECT_EQ((unsigned)status_codes::internal_error, (unsigned)response.status);
}


//---- End of source file ------------------------------------------------------
