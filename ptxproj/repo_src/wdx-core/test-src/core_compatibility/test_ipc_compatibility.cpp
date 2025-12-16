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
///  \brief    Test WAGO Parameter Service Core serialization compatibility.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wda_ipc/representation.hpp>

#include <string>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(ipc, DISABLED_serializer_compatibility_parameter_instance_path)
{
    std::string const device_part    = "0-0";
    std::string const parameter_part = "some-path";
    std::string const string_path    = device_part + "-" + parameter_part;
    std::string const device_path    = device_part;
    std::string const parameter_path = parameter_part.substr(0, parameter_part.find("-")) + "/" + parameter_part.substr(parameter_part.find("-") + 1);

    auto path = wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(string_path);
    EXPECT_EQ(device_path,    path.device_path);
    EXPECT_EQ(parameter_path, path.parameter_path);
}


//---- End of source file ------------------------------------------------------
