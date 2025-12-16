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
///  \brief    Client implementation for WAGO parameter service API test client.
///
///  \author   PEn: WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "apitest.hpp"
#include <wago/wdx/linuxos/client/client.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::linuxos::apitest;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {


//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
class apitest_client final : public client::abstract_client
{
private:
    std::unique_ptr<apitest> test;
    std::string              error_parameters;

public:
    std::string get_name() override;
    std::vector<client::client_option> get_additional_options() override;
    void init(client::init_args args) override;
};

std::string apitest_client::get_name()
{
    return "WAGO Parameter Service API-Test client";
}

std::vector<client::client_option> apitest_client::get_additional_options()
{
    return {{'E', "error-parameter",
             "Support exit and exception parameters. Valid values: true / false", error_parameters}};
}

void apitest_client::init(client::init_args args)
{
    test = std::make_unique<apitest>(args.backend, (error_parameters == "true"));
}

} // namespace apitest

namespace client {

std::unique_ptr<abstract_client> create_client()
{
    return std::make_unique<apitest::apitest_client>();
}

} // namespace client
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
