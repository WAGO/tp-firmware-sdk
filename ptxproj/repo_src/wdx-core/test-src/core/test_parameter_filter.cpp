//------------------------------------------------------------------------------
// Copyright (c) 2024 WAGO GmbH & Co. KG
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
///  \brief    Test parameter filter.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "wago/wdx/requests.hpp"
#include "wda_ipc/ipc.hpp"

using namespace wago::wdx;

class parameter_filter_fixture: public testing::TestWithParam<parameter_filter>{};

TEST_P(parameter_filter_fixture, ipc_serialization)
{
    auto filter = GetParam();
    EXPECT_EQ(filter, wago::wda_ipc::from_ipc_string<parameter_filter>(wago::wda_ipc::to_ipc_string(filter)));
}

INSTANTIATE_TEST_CASE_P(filters, parameter_filter_fixture, testing::Values(
    parameter_filter::only_device(device_selector::headstation()),
    parameter_filter::without_usersettings(),
    parameter_filter::only_usersettings(),
    parameter_filter::without_writeable(),
    parameter_filter::only_writeable(),
    parameter_filter::without_methods(),
    parameter_filter::only_methods(),
    parameter_filter::without_file_ids(),
    parameter_filter::only_file_ids(),
    parameter_filter::without_beta(),
    parameter_filter::only_beta(),
    parameter_filter::without_deprecated(),
    parameter_filter::only_deprecated(),
    parameter_filter::only_feature("SomeFeature"),
    parameter_filter::only_subpath("some/path")
));
