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
///  \brief    Test functions for combined parameter filtering.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/parameter_filter.hpp"
#include "http/http_exception.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda::rest;
using namespace wago::wdx;

using wago::wdx::wda::http::http_exception;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(filters, with_beta_filter_match_true)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_TRUE(combine_with_beta_filter(filter, filter_name, filter_name, "true"));
    EXPECT_TRUE(filter._only_beta);
}

TEST(filters, with_beta_filter_match_false)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_TRUE(combine_with_beta_filter(filter, filter_name, filter_name, "false"));
    EXPECT_TRUE(filter._without_beta);
}

TEST(filters, with_beta_filter_match_invalid_value)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_THROW(combine_with_beta_filter(filter, filter_name, filter_name, "invalid"), http_exception);
    EXPECT_EQ(parameter_filter(), filter);
}

TEST(filters, with_beta_filter_missmatch)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_FALSE(combine_with_beta_filter(filter, filter_name, "some.filter", "true"));
    EXPECT_EQ(parameter_filter(), filter);
}


TEST(filters, with_deprecated_filter_match_true)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_TRUE(combine_with_deprecated_filter(filter, filter_name, filter_name, "true"));
    EXPECT_TRUE(filter._only_deprecated);
}

TEST(filters, with_deprecated_filter_match_false)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_TRUE(combine_with_deprecated_filter(filter, filter_name, filter_name, "false"));
    EXPECT_TRUE(filter._without_deprecated);
}

TEST(filters, with_deprecated_filter_match_invalid_value)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_THROW(combine_with_deprecated_filter(filter, filter_name, filter_name, "invalid"), http_exception);
    EXPECT_EQ(parameter_filter(), filter);
}

TEST(filters, with_deprecated_filter_missmatch)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_FALSE(combine_with_deprecated_filter(filter, filter_name, "some.filter", "true"));
    EXPECT_EQ(parameter_filter(), filter);
}


TEST(filters, with_writeable_filter_match_true)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_TRUE(combine_with_writeable_filter(filter, filter_name, filter_name, "true"));
    EXPECT_TRUE(filter._only_writeable);
}

TEST(filters, with_writeable_filter_match_false)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_TRUE(combine_with_writeable_filter(filter, filter_name, filter_name, "false"));
    EXPECT_TRUE(filter._without_writeable);
}

TEST(filters, with_writeable_filter_match_invalid_value)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_THROW(combine_with_writeable_filter(filter, filter_name, filter_name, "invalid"), http_exception);
    EXPECT_EQ(parameter_filter(), filter);
}

TEST(filters, with_writeable_filter_missmatch)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_FALSE(combine_with_writeable_filter(filter, filter_name, "some.filter", "true"));
    EXPECT_EQ(parameter_filter(), filter);
}


TEST(filters, with_userSetting_filter_match_true)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_TRUE(combine_with_userSetting_filter(filter, filter_name, filter_name, "true"));
    EXPECT_TRUE(filter._only_usersettings);
}

TEST(filters, with_userSetting_filter_match_false)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_TRUE(combine_with_userSetting_filter(filter, filter_name, filter_name, "false"));
    EXPECT_TRUE(filter._without_usersettings);
}

TEST(filters, with_userSetting_filter_match_invalid_value)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_THROW(combine_with_userSetting_filter(filter, filter_name, filter_name, "invalid"), http_exception);
    EXPECT_EQ(parameter_filter(), filter);
}

TEST(filters, with_userSetting_filter_missmatch)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_FALSE(combine_with_userSetting_filter(filter, filter_name, "some.filter", "true"));
    EXPECT_EQ(parameter_filter(), filter);
}


TEST(filters, with_device_filter_match_sub_module)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_TRUE(combine_with_device_filter(filter, filter_name, filter_name, "1-15"));
    EXPECT_EQ(parameter_filter::only_device(device_selector::specific(device_id(15, 1))), filter);
}

TEST(filters, with_device_filter_match_headstation_id)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_TRUE(combine_with_device_filter(filter, filter_name, filter_name, "0-0"));
    EXPECT_EQ(parameter_filter::only_device(device_selector::headstation()), filter);
}

TEST(filters, with_device_filter_match_headstation)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_TRUE(combine_with_device_filter(filter, filter_name, filter_name, "headstation"));
    EXPECT_EQ(parameter_filter::only_device(device_selector::headstation()), filter);
}

TEST(filters, with_device_filter_match_invalid_value)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_THROW(combine_with_device_filter(filter, filter_name, filter_name, "invalid"), http_exception);
    EXPECT_EQ(parameter_filter(), filter);
}

TEST(filters, with_device_filter_missmatch)
{
    std::string const filter_name = "some.filter.name";

    parameter_filter filter;
    EXPECT_FALSE(combine_with_device_filter(filter, filter_name, "some.filter", "0-0"));
    EXPECT_EQ(parameter_filter(), filter);
}


TEST(filters, with_path_filter_match_path)
{
    std::string const filter_name = "some.filter.name";
    std::string const filter_path = "/some/parameter/path";

    parameter_filter filter;
    EXPECT_TRUE(combine_with_path_filter(filter, filter_name, filter_name, filter_path));
    EXPECT_EQ(parameter_filter::only_subpath(filter_path), filter);
}

// FIXME: Invalid path values should trigger an exception
TEST(filters, DISABLED_with_path_filter_match_invalid_value)
{
    std::string const filter_name = "some.filter.name";
    std::string const filter_path = "invalid!chars";

    parameter_filter filter;
    EXPECT_THROW(combine_with_path_filter(filter, filter_name, filter_name, filter_path), http_exception);
    EXPECT_EQ(parameter_filter(), filter);
}

TEST(filters, with_path_filter_missmatch)
{
    std::string const filter_name = "some.filter.name";
    std::string const filter_path = "/some/parameter/path";

    parameter_filter filter;
    EXPECT_FALSE(combine_with_path_filter(filter, filter_name, "some.filter", filter_path));
    EXPECT_EQ(parameter_filter(), filter);
}


TEST(extraction, match_no_filters)
{
    auto filter = extract_filters_from_query(
        {
            {"device.filter.name",   combine_with_device_filter},
            {"beta.name",            combine_with_beta_filter},
            {"filter.name.for.path", combine_with_path_filter},
            {"filter.writeable",     combine_with_writeable_filter}
        },
        {}
    );
    EXPECT_EQ(parameter_filter(), filter);
}

TEST(extraction, match_one_of_multiple_filters)
{
    auto filter = extract_filters_from_query(
        {
            {"device.filter.name",   combine_with_device_filter},
            {"beta.name",            combine_with_beta_filter},
            {"filter.name.for.path", combine_with_path_filter},
            {"filter.writeable",     combine_with_writeable_filter}
        },
        {
            {"beta.name", "true"}
        }
    );
    EXPECT_EQ(parameter_filter::only_beta(), filter);
}

TEST(extraction, match_some_of_multiple_filters)
{
    auto filter = extract_filters_from_query(
        {
            {"filter.name.for.path", combine_with_path_filter},
            {"device.filter.name",   combine_with_device_filter},
            {"deprecated.name",      combine_with_deprecated_filter},
            {"filter.writeable",     combine_with_writeable_filter}
        },
        {
            {"deprecated.name",    "false"},
            {"device.filter.name", "0-0"}
        }
    );
    EXPECT_EQ((   parameter_filter::without_deprecated()
                | parameter_filter::only_device(device_selector::headstation())), filter);
}

TEST(extraction, match_all_filters)
{
    auto filter = extract_filters_from_query(
        {
            {"beta",         combine_with_beta_filter},
            {"deprecated",   combine_with_deprecated_filter},
            {"writeable",    combine_with_writeable_filter},
            {"user_setting", combine_with_userSetting_filter},
            {"device",       combine_with_device_filter},
            {"path",         combine_with_path_filter}
        },
        {
            {"beta",         "false"},
            {"deprecated",   "true"},
            {"writeable",    "false"},
            {"user_setting", "true"},
            {"device",       "1-3"},
            {"path",         "/some/path/to/parameter"}
        }
    );
    EXPECT_EQ((   parameter_filter::without_beta()
                | parameter_filter::only_deprecated()
                | parameter_filter::without_writeable()
                | parameter_filter::only_usersettings()
                | parameter_filter::only_device(device_selector::specific(device_id(3, 1)))
                | parameter_filter::only_subpath("/some/path/to/parameter")), filter);
}

TEST(extraction, invalid_filter_name)
{
    parameter_filter filter;
    EXPECT_THROW(filter = extract_filters_from_query(
                     {
                         {"filter.name.for.path", combine_with_path_filter},
                         {"deprecated.name",      combine_with_deprecated_filter},
                         {"device.filter.name",   combine_with_device_filter},
                         {"filter.writeable",     combine_with_writeable_filter}
                     },
                     {
                         {"device.filter.name", "0-0"},
                         {"some_other.query",   "a_value"},
                         {"deprecated.name",    "true"}
                     }
                 ),
                 http_exception);
    EXPECT_EQ(parameter_filter(), filter);
}

TEST(extraction, invalid_value)
{
    parameter_filter filter;
    EXPECT_THROW(filter = extract_filters_from_query(
                     {
                         {"deprecated.name", combine_with_deprecated_filter}
                     },
                     {
                         {"deprecated.name", "invalid"}
                     }
                 ),
                 http_exception);
    EXPECT_EQ(parameter_filter(), filter);
}


//---- End of source file ------------------------------------------------------
