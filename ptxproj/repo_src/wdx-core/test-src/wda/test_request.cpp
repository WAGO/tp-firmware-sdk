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
///  \brief    Test of request class.
///
///  \author   Röh:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/request.hpp"
#include "mocks/mock_request.hpp"
#include "mocks/mock_response.hpp"
#include "rest/definitions.hpp"

#include <wda_ipc/representation.hpp>

#include <gtest/gtest.h>

#include <map>
#include <vector>
#include <sstream>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::wda::rest::request;
using wago::wdx::wda::rest::serializer_i;
using wago::wdx::wda::rest::deserializer_i;
using wago::wdx::wda::http::http_method;
using std::string;
using ::testing::ReturnRef;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------

class request_fixture : public ::testing::Test
{
public:
    std::unique_ptr<mock_request> request_mock = std::make_unique<mock_request>();
    int                              dummy           = 1;
    serializer_i             const * serializer      = reinterpret_cast<serializer_i   const *>(&dummy);
    deserializer_i           const * deserializer    = reinterpret_cast<deserializer_i const *>(&dummy);
    string                           key             = "key";
    string                           value           = "value";
    std::map<string, string>         path_parameters;

    void SetUp() override
    {
        request_mock->set_default_expectations();
        path_parameters[key] = value;
    }
};

TEST_F(request_fixture, constructor)
{
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
}

TEST_F(request_fixture, get_doc_link)
{
    std::string test_doc_link = "/test/doc";
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters, test_doc_link);
    EXPECT_EQ(test_doc_link, my_request.get_doc_link());
}

TEST_F(request_fixture, is_https)
{
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_TRUE(my_request.is_https());
}

TEST_F(request_fixture, get_method)
{
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_EQ(http_method::get, my_request.get_method());
}


TEST_F(request_fixture, get_request_uri)
{
    std::string request_uri   = "/test-url/a/request/uri";

    EXPECT_CALL(*request_mock, get_request_uri())
        .Times(Exactly(1))
        .WillRepeatedly(Return(request_uri));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_EQ(request_uri, my_request.get_request_uri().as_string());
}

TEST_F(request_fixture, get_query_parameter)
{
    EXPECT_CALL(*request_mock, get_query_parameter(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(value));;
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_EQ(value, my_request.get_query_parameter(key));
}

TEST_F(request_fixture, has_http_header)
{
    EXPECT_CALL(*request_mock, has_http_header(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(true));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_TRUE(my_request.has_http_header(key));
}

TEST_F(request_fixture, get_http_header)
{
    EXPECT_CALL(*request_mock, get_http_header(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(value));;
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_EQ(value, my_request.get_http_header(key));
}

TEST_F(request_fixture, get_accepted_types)
{
    EXPECT_CALL(*request_mock, get_accepted_types())
        .Times(Exactly(1))
        .WillRepeatedly(Return("TEST"));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_EQ("TEST", my_request.get_accepted_types());
}

TEST_F(request_fixture, get_content_type)
{
    EXPECT_CALL(*request_mock, get_content_type())
        .Times(Exactly(1))
        .WillRepeatedly(Return("TEST"));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_EQ("TEST", my_request.get_content_type());
}

TEST_F(request_fixture, get_content_length)
{
    EXPECT_CALL(*request_mock, get_content_length())
        .Times(Exactly(1))
        .WillRepeatedly(Return("TEST"));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_EQ("TEST", my_request.get_content_length());
}

TEST_F(request_fixture, get_content)
{
    EXPECT_CALL(*request_mock, get_content())
        .Times(Exactly(1))
        .WillRepeatedly(Return("TEST"));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_EQ("TEST", my_request.get_content());
}

TEST_F(request_fixture, get_content_stream)
{
    std::stringstream test_stream;

    EXPECT_CALL(*request_mock, get_content_stream())
        .Times(Exactly(1))
        .WillRepeatedly(ReturnRef(test_stream));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_EQ(&test_stream, &my_request.get_content_stream());
}

TEST_F(request_fixture, add_response_header)
{
    EXPECT_CALL(*request_mock, add_response_header(::testing::Ref(key), ::testing::Ref(value)))
        .Times(Exactly(1));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    my_request.add_response_header(key, value);
}

TEST_F(request_fixture, respond)
{
    mock_response response_mock;
    response_i &response = response_mock;

    EXPECT_CALL(*request_mock, respond_mock(::testing::Ref(response)))
        .Times(Exactly(1));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    my_request.respond(response);
}

TEST_F(request_fixture, is_responded)
{
    EXPECT_CALL(*request_mock, is_responded())
        .Times(Exactly(1))
        .WillRepeatedly(Return(true));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_TRUE(my_request.is_responded());
}

TEST_F(request_fixture, send_data)
{
    char const *bytes = "";
    size_t      size  = 0;

    EXPECT_CALL(*request_mock, send_data(::testing::Eq(bytes), ::testing::Eq(size)))
        .Times(Exactly(1));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    my_request.send_data(bytes, size);
}

TEST_F(request_fixture, finish)
{
    EXPECT_CALL(*request_mock, finish())
        .Times(Exactly(1));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    my_request.finish();
}

TEST_F(request_fixture, has_path_parameters_found)
{
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_TRUE(my_request.has_path_parameter(key));
}

TEST_F(request_fixture, has_path_parameters_not_found)
{
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_FALSE(my_request.has_path_parameter(value));
}

TEST_F(request_fixture, get_path_parameters)
{
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_EQ(value, my_request.get_path_parameter(key));
}

TEST_F(request_fixture, get_path_parameters_negative)
{
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_THROW(my_request.get_path_parameter(value), std::out_of_range);
}

TEST_F(request_fixture, get_serializer)
{
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    serializer_i const &found_serializer = my_request.get_serializer();
    EXPECT_EQ(serializer, &found_serializer);
}

TEST_F(request_fixture, get_deserializer)
{
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    deserializer_i const &found_deserializer = my_request.get_deserializer();
    EXPECT_EQ(deserializer, &found_deserializer);
}

TEST_F(request_fixture, get_include_parameters)
{
    std::vector<std::vector<string>> results;

    EXPECT_CALL(*request_mock, has_query_parameter(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*request_mock, get_query_parameter(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return("firstrelated,secondrelated.nestedrelated"));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    my_request.get_include_parameters(results);
    EXPECT_EQ(2,               results.size());
    EXPECT_EQ("firstrelated",  results[0][0]);
    EXPECT_EQ("secondrelated", results[1][0]);
    EXPECT_EQ("nestedrelated", results[1][1]);
}

TEST_F(request_fixture, get_pagination_parameters_with_given_values)
{
    unsigned page_limit  = 0;
    unsigned page_offset = 0;

    EXPECT_CALL(*request_mock, has_query_parameter(::testing::_))
        .Times(Exactly(2))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*request_mock, get_query_parameter(::testing::_, ::testing::_))
        .Times(Exactly(2))
        .WillRepeatedly(Return("10"));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    my_request.get_pagination_parameters(page_limit, page_offset);
    EXPECT_EQ(10, page_limit);
    EXPECT_EQ(10, page_offset);
}

TEST_F(request_fixture, get_result_behavior)
{

    EXPECT_CALL(*request_mock, has_query_parameter(::testing::_))
        .Times(Exactly(5))
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*request_mock, get_query_parameter(::testing::_, ::testing::_))
        .Times(Exactly(4))
        .WillOnce(Return("auto"))
        .WillOnce(Return("async"))
        .WillOnce(Return("sync"))
        .WillOnce(Return("someOtherValue"));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_EQ(wago::wdx::wda::rest::result_behavior_types::any,       my_request.get_result_behavior());
    EXPECT_EQ(wago::wdx::wda::rest::result_behavior_types::automatic, my_request.get_result_behavior());
    EXPECT_EQ(wago::wdx::wda::rest::result_behavior_types::async,     my_request.get_result_behavior());
    EXPECT_EQ(wago::wdx::wda::rest::result_behavior_types::sync,      my_request.get_result_behavior());
    EXPECT_THROW (my_request.get_result_behavior(), wago::wdx::wda::http::http_exception);
}

TEST_F(request_fixture, get_filter_queries)
{
    std::string query = "?a=true&b&x&filter[a]=123&foo&filter[other]=false";

    EXPECT_CALL(*request_mock, get_request_uri())
        .Times(1)
        .WillRepeatedly(Return(query));

    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);

    auto filters = my_request.get_filter_queries();
    EXPECT_EQ(2, filters.size());
    EXPECT_TRUE(filters.count("a"));
    EXPECT_TRUE(filters.count("other"));
    EXPECT_EQ("123", filters.at("a"));
    EXPECT_EQ("false", filters.at("other"));
}

TEST_F(request_fixture, get_filter_queries_missing_filter_name)
{
    std::string query = "?filter[a]=x&filter[]=z";

    EXPECT_CALL(*request_mock, get_request_uri())
        .Times(1)
        .WillRepeatedly(Return(query));

    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);

    auto filters = my_request.get_filter_queries();
    EXPECT_EQ(1, filters.size());
    EXPECT_TRUE(filters.count("a"));
    EXPECT_EQ("x", filters.at("a"));
}

TEST_F(request_fixture, get_filter_queries_missing_closing_square_bracket)
{
    std::string query = "?filter[abc&filter[a]=x";

    EXPECT_CALL(*request_mock, get_request_uri())
        .Times(1)
        .WillRepeatedly(Return(query));

    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);

    auto filters = my_request.get_filter_queries();
    EXPECT_EQ(1, filters.size());
    EXPECT_TRUE(filters.count("a"));
    EXPECT_EQ("x", filters.at("a"));
}

TEST_F(request_fixture, get_filter_queries_empty_query)
{
    std::string query = "";

    EXPECT_CALL(*request_mock, get_request_uri())
        .Times(1)
        .WillRepeatedly(Return(query));

    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);

    auto filters = my_request.get_filter_queries();
    EXPECT_EQ(0, filters.size());
}

TEST_F(request_fixture, get_filter_queries_missing_equal_character)
{
    std::string query = "?filter[c]&filter[a]=x&filter[b]z";

    EXPECT_CALL(*request_mock, get_request_uri())
        .Times(1)
        .WillRepeatedly(Return(query));

    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);

    auto filters = my_request.get_filter_queries();
    EXPECT_EQ(1, filters.size());
    EXPECT_TRUE(filters.count("a"));
    EXPECT_EQ("x", filters.at("a"));
}

TEST_F(request_fixture, get_pagination_parameters_default)
{
    unsigned page_limit  = 0;
    unsigned page_offset = 0;

    EXPECT_CALL(*request_mock, has_query_parameter(::testing::_))
        .Times(Exactly(2))
        .WillRepeatedly(Return(false));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    my_request.get_pagination_parameters(page_limit, page_offset);
    EXPECT_EQ(wago::wdx::wda::rest::page_limit_default,  page_limit);
    EXPECT_EQ(wago::wdx::wda::rest::page_offset_default, page_offset);
}

TEST_F(request_fixture, get_errors_as_data_attributes_parameter)
{
    EXPECT_CALL(*request_mock, has_query_parameter(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*request_mock, get_query_parameter(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return("true"));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_TRUE(my_request.get_errors_as_data_attributes_parameter());

}

TEST_F(request_fixture, get_deferred_parameters_as_errors)
{
    EXPECT_CALL(*request_mock, has_query_parameter(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*request_mock, get_query_parameter(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return("treu"));
    request my_request(std::move(request_mock), *serializer, *deserializer, path_parameters);
    EXPECT_FALSE(my_request.get_deferred_parameters_as_errors());

}

//---- End of source file ------------------------------------------------------
