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
///  \brief    Test JSON:API serialization.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/json_api.hpp"
#include "rest/definitions.hpp"
#include "rest/core_exception.hpp"
#include "rest/collection_document.hpp"
#include "wago/wdx/wda/http/http_status_code.hpp"

#include "wago/wdx/parameter_service_frontend_i.hpp"
#include <wda_ipc/representation.hpp>
#include <wc/assertion.h>

#include <nlohmann/json.hpp>
#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <map>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda;
using namespace wago::wdx::wda::http;
using namespace wago::wdx::wda::rest;
using namespace wago::wdx;
using nlohmann::json;
using std::string;
using std::vector;
using std::map;
using std::make_shared;


#define EXPECT_HTTP_EXCEPTION(status_code, execution_code, error_text) \
    try \
    { \
        execution_code; \
        FAIL() << (error_text); \
    } \
    catch(wago::wdx::wda::rest::http_exception const &e) \
    { \
        EXPECT_EQ(status_code, e.get_http_status_code()) << "Expected status code: " << static_cast<unsigned>(status_code) << std::endl \
                                                         << "Actual status code:   " << static_cast<unsigned>(e.get_http_status_code()); \
    } \
    catch(...) \
    { \
        FAIL() << "Not expected to throw other things than wago::wdx::wda::rest::http_exception"; \
    }

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
std::string build_monitoring_list_request_string(std::string const &json_timeout,
                                                 std::string const &parameter_1,
                                                 std::string const &parameter_2);

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(json_api, provides_json_api_content_type)
{
    char const expected_content_type[] = "application/vnd.api+json";

    json_api serializer;
    EXPECT_STREQ(expected_content_type, serializer.get_content_type().c_str());
}

TEST(json_api, serialize_error)
{
    json_api               serializer;
    string           const base_path   = "/base/path";
    http_status_code const http_status = http_status_code::forbidden;
    string           const title       = get_http_status_code_text(http_status);
    string           const message     = "Error message";

    auto const test_error = make_shared<http_exception>(message, http_status);

    string result;
    serializer.serialize(result, test_error);
    auto expect = json::object({
        { "jsonapi", {
            { "version", "1.0" }
        }},
        { "meta", {
            { "version", REST_API_VERSION }
        }},
        { "errors", {{
            { "status", to_string(http_status)},
            { "title",  title},
            { "detail", message}
        }}}
    });
    auto actual = json::parse(result);
    EXPECT_TRUE(expect == actual) << "Expected JSON: " << expect.dump() << std::endl
                                  << "Actual JSON:   " << actual.dump();
}

TEST(json_api, serialize_core_error)
{
    json_api                 serializer;
    string             const base_path      = "/base/path";
    http_status_code   const http_status    = http_status_code::internal_server_error;
    core_status_code   const core_code      = core_status_code::internal_error;
    domain_status_code const domain_code    = 42;
    string             const core_code_text = "Internal Error";
    string             const message        = "Error message";
    auto const test_error = make_shared<core_exception>(core_error(core_code, message, domain_code, "mytype"));

    string result;    
    serializer.serialize(result, test_error);
    auto expect = json::object({
        { "jsonapi", {
            { "version", "1.0" }
        }},
        { "meta", {
            { "version", REST_API_VERSION }
        }},
        { "errors", {{
            { "status", to_string(http_status)},
            { "code",   wago::wda_ipc::to_string(core_code)},
            { "title",  "An internal error occurred"},
            { "detail", "An internal error occurred on mytype. " + message + ". (" + core_code_text + ")"},
            { "meta", {
                { "domainSpecificStatusCode", "42" }
            }}
        }}}
    });
    auto actual = json::parse(result);
    EXPECT_TRUE(expect == actual) << "Expected JSON: " << expect.dump() << std::endl
                                  << "Actual JSON:   " << actual.dump();
}

TEST(json_api, serialize_empty_collection)
{
    json_api                  serializer;
    string              const base_path   = "/base/path";
    map<string, string> const meta        = {};
    string              const query       = "";
    unsigned            const page_limit  = 5;
    unsigned            const page_offset = 0;

    device_collection_document const collection(base_path, query, meta, vector<device_response>(0), page_offset, page_limit, 0);

    string result;
    serializer.serialize(result, collection);
    string const paging_query = std::string(page_limit_query_key)  + "=" + std::to_string(page_limit) + "&" +
                                std::string(page_offset_query_key) + "=" + std::to_string(0);
    auto expect = json::object({
        { "jsonapi", {
            { "version", "1.0" }
        } },
        { "meta", {
            { "version", REST_API_VERSION }
        } },
        { "links", {
            { self_link_name,       base_path + "?" + paging_query },
            { first_page_link_name, base_path + "?" + paging_query },
            { last_page_link_name,  base_path + "?" + paging_query }
        }},
        { "data", json::array({}) }
    });
    auto actual = json::parse(result);
    EXPECT_TRUE(expect == actual) << "Expected JSON: " << expect.dump() << std::endl
                                  << "Actual JSON:   " << actual.dump();
}

TEST(json_api, serialize_empty_collection_with_query)
{
    json_api                  serializer;
    string              const base_path      = "/base/path";
    map<string, string> const meta           = {};
    unsigned            const page_limit     = 6;
    unsigned            const page_offset    = 0;
    string              const query          = "?first-param=3x3&page[limit]=" + std::to_string(page_limit) + "&third_param=something";
    string              const query_filtered = "?first-param=3x3&third_param=something";

    device_collection_document const collection(base_path, query, meta, vector<device_response>(0), page_offset, page_limit, 0);

    string result;
    serializer.serialize(result, collection);
    string const paging_query = std::string(page_limit_query_key)  + "=" + std::to_string(page_limit) + "&" +
                                std::string(page_offset_query_key) + "=" + std::to_string(0);
    auto expect = json::object({
        { "jsonapi", {
            { "version", "1.0" }
        } },
        { "meta", {
            { "version", REST_API_VERSION }
        } },
        { "links", {
            { self_link_name,       base_path + query_filtered + "&" + paging_query },
            { first_page_link_name, base_path + query_filtered + "&" + paging_query },
            { last_page_link_name,  base_path + query_filtered + "&" + paging_query }
        }},
        { "data", json::array({}) }
    });
    auto actual = json::parse(result);
    EXPECT_TRUE(expect == actual) << "Expected JSON: " << expect.dump() << std::endl
                                  << "Actual JSON:   " << actual.dump();
}

TEST(json_api, serialize_empty_collection_with_page_offset)
{
    json_api                  serializer;
    string              const base_path   = "/base/path";
    unsigned            const page_limit  = 6;
    unsigned            const page_offset = 1;
    string              const query       = "?page[offset]=" + std::to_string(page_offset);
    map<string, string> const meta        = {};

    device_collection_document const collection(base_path, query, meta, vector<device_response>(0), page_offset, page_limit, 0);

    string result;
    serializer.serialize(result, collection);
    string const paging_query = std::string(page_limit_query_key)  + "=" + std::to_string(page_limit) + "&" +
                                std::string(page_offset_query_key) + "=" + std::to_string(0);
    auto expect = json::object({
        { "jsonapi", {
            { "version", "1.0" }
        } },
        { "meta", {
            { "version", REST_API_VERSION }
        } },
        { "links", {
            { self_link_name,          base_path + "?" + std::string(page_limit_query_key)  + "=" + std::to_string(page_limit) + "&" +
                                                         std::string(page_offset_query_key) + "=" + std::to_string(page_offset) },
            { first_page_link_name,    base_path + "?" + paging_query },
            { last_page_link_name,     base_path + "?" + paging_query },
            { previous_page_link_name, base_path + "?" + paging_query }
        }},
        { "data", json::array({}) }
    });
    auto actual = json::parse(result);
    EXPECT_TRUE(expect == actual) << "Expected JSON: " << expect.dump() << std::endl
                                  << "Actual JSON:   " << actual.dump();
}

TEST(json_api, serialize_collection)
{
    json_api                      serializer;
    string                  const base_path   = "/base/path";
    string                  const query       = "";
    map<string, string>     const meta        = {};
    unsigned                const page_limit  = 5;
    unsigned                const page_offset = 0;
    device_response               dev1;
                                  dev1.id           = { 1, 1 };
                                  dev1.order_number = "123";
                                  dev1.firmware_version = "0.1.0";
    device_response               dev2;
                                  dev2.id           = { 2, 1 };
                                  dev2.order_number = "456";
                                  dev2.firmware_version = "2.0.18";
    vector<device_response> const device_responses = { dev1, dev2 };

    device_collection_document const collection(base_path, query, meta, vector<device_response>(device_responses), page_offset, page_limit, device_responses.size());

    WC_STATIC_ASSERT(sizeof(unsigned) >= sizeof(device_response::id.slot));
    WC_STATIC_ASSERT(sizeof(unsigned) >= sizeof(device_response::id.device_collection_id));
    string result;
    serializer.serialize(result, collection);
    string const paging_query = std::string(page_limit_query_key)  + "=" + std::to_string(page_limit) + "&" +
                                std::string(page_offset_query_key) + "=" + std::to_string(0);
    auto expect = json::object({
        { "jsonapi", {
            { "version", "1.0" }
        } },
        { "meta", {
            { "version", REST_API_VERSION }
        } },
        { "links", {
            { self_link_name,       base_path + "?" + paging_query },
            { first_page_link_name, base_path + "?" + paging_query },
            { last_page_link_name,  base_path + "?" + paging_query }
        }},
        { "data", {
           
                {
                    { "id", wago::wda_ipc::to_string(dev1.id) },
                    { "relationships", {
                        { "features", {
                            { "links", {
                                { "related", "/wda/devices/" + wago::wda_ipc::to_string(dev1.id) + "/features" }
                            }}
                        }}
                    }},
                    { "type", "devices" },
                    { "links", {
                        { self_link_name, "/wda/devices/" + wago::wda_ipc::to_string(dev1.id) }
                    }},
                    { "attributes", {
                        { "orderNumber",     dev1.order_number },
                        { "firmwareVersion", dev1.firmware_version },
                        { "busPosition",     dev1.id.slot }
                    } }
                },
                {
                    { "id", wago::wda_ipc::to_string(dev2.id) },
                    { "relationships", {
                        { "features", {
                            { "links", {
                                { "related", "/wda/devices/" + wago::wda_ipc::to_string(dev2.id) + "/features" }
                            }}
                        }}
                    }},
                    { "type", "devices" },
                    { "links", {
                        { self_link_name, "/wda/devices/" + wago::wda_ipc::to_string(dev2.id) }
                    }},
                    { "attributes", {
                        { "orderNumber",     dev2.order_number },
                        { "firmwareVersion", dev2.firmware_version },
                        { "busPosition",     dev2.id.slot }
                    } }
                }
           
        } }
    });
    auto actual = json::parse(result);
    EXPECT_TRUE(expect == actual) << "Expected JSON: " << expect.dump() << std::endl
                                  << "Actual JSON:   " << actual.dump();
}

TEST(json_api, serialize_collection_with_paging_query)
{
    json_api                      serializer;
    string                  const base_path      = "/base/path";
    unsigned                const page_limit     = 2;
    unsigned                const page_offset    = 1;
    string                  const query          = "?page[limit]=" + std::to_string(page_limit) + "&second-param=19"
                                                   "&third_param=something&page[offset]=" + std::to_string(page_offset);
    map<string, string>     const meta           = { { "test", "meta" } };
    string                  const query_filtered = "?second-param=19&third_param=something";
    device_response               dev0;
                                  dev0.id           = { 1, 1 };
                                  dev0.order_number = "000";
                                  dev0.firmware_version = "1.15.3";
    device_response               dev1;
                                  dev1.id           = { 2, 1 };
                                  dev1.order_number = "123";
                                  dev1.firmware_version = "0.1.0";
    device_response               dev2;
                                  dev2.id           = { 3, 1 };
                                  dev2.order_number = "456";
                                  dev2.firmware_version = "2.0.18";
    vector<device_response> const device_responses = { dev0, dev1, dev2 };
    vector<device_response> const device_responses_paged = { dev1, dev2 };

    device_collection_document const collection(base_path, query, meta, vector<device_response>(device_responses_paged), page_offset, page_limit, device_responses.size());

    WC_STATIC_ASSERT(sizeof(unsigned) >= sizeof(device_response::id.slot));
    WC_STATIC_ASSERT(sizeof(unsigned) >= sizeof(device_response::id.device_collection_id));
    string result;
    serializer.serialize(result, collection);
    string const paging_query = std::string(page_limit_query_key)  + "=" + std::to_string(page_limit) + "&" +
                                std::string(page_offset_query_key) + "=" + std::to_string(0);
    auto expect = json::object({
        { "jsonapi", {
            { "version", "1.0" }
        } },
        { "meta", {
            { "version", REST_API_VERSION },
            { "test", "meta" }
        } },
        { "links", {
            { self_link_name,          base_path + query_filtered + "&" +
                                       std::string(page_limit_query_key)  + "=" + std::to_string(page_limit) + "&" +
                                       std::string(page_offset_query_key) + "=" + std::to_string(page_offset) },
            { first_page_link_name,    base_path + query_filtered + "&" + paging_query },
            { last_page_link_name,     base_path + query_filtered + "&" +
                                       std::string(page_limit_query_key)  + "=" + std::to_string(page_limit) + "&" +
                                       std::string(page_offset_query_key) + "=" + std::to_string(1)},
            { previous_page_link_name, base_path + query_filtered + "&" + paging_query }
        }},
        { "data", {

                {
                    { "id", wago::wda_ipc::to_string(dev1.id) },
                    { "relationships", {
                        { "features", {
                            { "links", {
                                { "related", "/wda/devices/" + wago::wda_ipc::to_string(dev1.id) + "/features" }
                            }}
                        }}
                    }},
                    { "type", "devices" },
                    { "links", {
                        { self_link_name, "/wda/devices/" + wago::wda_ipc::to_string(dev1.id) }
                    }},
                    { "attributes", {
                        { "orderNumber", dev1.order_number },
                        { "firmwareVersion", dev1.firmware_version },
                        { "busPosition", dev1.id.slot }
                    } }
                },
                {
                    { "id", wago::wda_ipc::to_string(dev2.id) },
                    { "relationships", {
                        { "features", {
                            { "links", {
                                { "related", "/wda/devices/" + wago::wda_ipc::to_string(dev2.id) + "/features" }
                            }}
                        }}
                    }},
                    { "type", "devices" },
                    { "links", {
                        { self_link_name, "/wda/devices/" + wago::wda_ipc::to_string(dev2.id) }
                    }},
                    { "attributes", {
                        { "orderNumber", dev2.order_number },
                        { "firmwareVersion", dev2.firmware_version },
                        { "busPosition", dev2.id.slot }
                    } }
                }

        } }
    });
    auto actual = json::parse(result);
    EXPECT_TRUE(expect == actual) << "Expected JSON: " << expect.dump() << std::endl
                                  << "Actual JSON:   " << actual.dump();
}

std::string build_monitoring_list_request_string(std::string const &json_timeout,
                                                 std::string const &parameter_1,
                                                 std::string const &parameter_2)
{
    return "{"
           "  \"data\": {"
           "    \"type\": \"monitoring-lists\","
           "    \"attributes\": {"
                  + (json_timeout.empty() ? "" :
           "      \"timeout\": " + json_timeout)
                  +
           "    },"
           "    \"relationships\": {"
           "      \"parameters\": {"
           "        \"data\": ["
                      + (parameter_1.empty() ? "" :
           "          {"
           "            \"id\": \"" + parameter_1 + "\","
           "            \"type\": \"parameters\""
           "          }")
                      + (parameter_2.empty() ? "" : ","
           "          {"
           "            \"id\": \"" + parameter_2 + "\","
           "            \"type\": \"parameters\""
           "          }")
                      +
           "        ]"
           "      }"
           "    }"
           "  }"
           "}";
}

TEST(json_api, deserialize_create_monitoring_list_request)
{
    json_api          deserializer;

    uint16_t    const test_timeout = 5;
    std::string const parameter_1  = "0-0-some-value";
    std::string const parameter_2  = "0-0-some-other-value";
    std::string const json_timeout = std::to_string(static_cast<uint64_t>(test_timeout));
    std::string const json_string  = build_monitoring_list_request_string(json_timeout, parameter_1, parameter_2);

    wago::wdx::wda::rest::create_monitoring_list_request request;
    deserializer.deserialize(request, json_string);
    EXPECT_EQ(test_timeout, request.timeout);
    EXPECT_EQ(2, request.parameters.size());
}

TEST(json_api, deserialize_create_monitoring_list_request_without_parameters)
{
    json_api          deserializer;

    uint16_t    const test_timeout = 5;
    std::string const json_timeout = std::to_string(static_cast<uint64_t>(test_timeout));
    std::string const json_string  = build_monitoring_list_request_string(json_timeout, "", "");

    wago::wdx::wda::rest::create_monitoring_list_request request;
    deserializer.deserialize(request, json_string);
    EXPECT_EQ(test_timeout, request.timeout);
    EXPECT_EQ(0, request.parameters.size());
}

TEST(json_api, deserialize_create_monitoring_list_request_without_timeout_value)
{
    json_api          deserializer;

    std::string const parameter_1  = "0-0-some-value";
    std::string const parameter_2  = "0-0-some-other-value";
    std::string const json_string  = build_monitoring_list_request_string("", parameter_1, parameter_2);

    wago::wdx::wda::rest::create_monitoring_list_request request;
    EXPECT_HTTP_EXCEPTION(wago::wdx::wda::http::http_status_code::bad_request,
                          deserializer.deserialize(request, json_string),
                          "Request without timeout value should throw");
}

TEST(json_api, deserialize_create_monitoring_list_request_with_overflow_value)
{
    json_api          deserializer;

    uint64_t    const test_timeout = UINT16_MAX + 1ULL;
    std::string const parameter_1  = "0-0-some-value";
    std::string const parameter_2  = "0-0-some-other-value";
    std::string const json_timeout = std::to_string(test_timeout);
    std::string const json_string  = build_monitoring_list_request_string(json_timeout, parameter_1, parameter_2);

    wago::wdx::wda::rest::create_monitoring_list_request request;
    EXPECT_HTTP_EXCEPTION(wago::wdx::wda::http::http_status_code::bad_request,
                          deserializer.deserialize(request, json_string),
                          "Overflow value should throw");
}

TEST(json_api, deserialize_create_monitoring_list_request_with_negative_value)
{
    json_api          deserializer;

    int64_t     const test_timeout = -1;
    std::string const parameter_1  = "0-0-some-value";
    std::string const parameter_2  = "0-0-some-other-value";
    std::string const json_timeout = std::to_string(test_timeout);
    std::string const json_string  = build_monitoring_list_request_string(json_timeout, parameter_1, parameter_2);

    wago::wdx::wda::rest::create_monitoring_list_request request;
    EXPECT_HTTP_EXCEPTION(wago::wdx::wda::http::http_status_code::bad_request,
                          deserializer.deserialize(request, json_string),
                          "Negative value should throw");
}

TEST(json_api, deserialize_create_monitoring_list_request_with_dot_timeout)
{
    json_api          deserializer;

    std::string const parameter_1  = "0-0-some-value";
    std::string const parameter_2  = "0-0-some-other-value";
    std::string const json_timeout = ".";
    std::string const json_string  = build_monitoring_list_request_string(json_timeout, parameter_1, parameter_2);

    wago::wdx::wda::rest::create_monitoring_list_request request;
    EXPECT_HTTP_EXCEPTION(wago::wdx::wda::http::http_status_code::bad_request,
                          deserializer.deserialize(request, json_string),
                          "Non-numeric values should throw");
}

TEST(json_api, deserialize_create_monitoring_list_request_with_real_value_timeout)
{
    json_api          deserializer;

    std::string const parameter_1  = "0-0-some-value";
    std::string const parameter_2  = "0-0-some-other-value";
    std::string const json_timeout = "2.5";
    std::string const json_string  = build_monitoring_list_request_string(json_timeout, parameter_1, parameter_2);

    wago::wdx::wda::rest::create_monitoring_list_request request;
    EXPECT_HTTP_EXCEPTION(wago::wdx::wda::http::http_status_code::bad_request,
                          deserializer.deserialize(request, json_string),
                          "Real values should throw");
}

TEST(json_api, deserialize_create_monitoring_list_request_with_timeout_value_postfix)
{
    json_api          deserializer;

    std::string const parameter_1  = "0-0-some-value";
    std::string const parameter_2  = "0-0-some-other-value";
    std::string const json_timeout = "5k";
    std::string const json_string  = build_monitoring_list_request_string(json_timeout, parameter_1, parameter_2);

    wago::wdx::wda::rest::create_monitoring_list_request request;
    EXPECT_HTTP_EXCEPTION(wago::wdx::wda::http::http_status_code::bad_request,
                          deserializer.deserialize(request, json_string),
                          "Values with postfix should throw");
}

TEST(json_api, deserialize_create_monitoring_list_request_with_empty_string_timeout)
{
    json_api          deserializer;

    std::string const parameter_1  = "0-0-some-value";
    std::string const parameter_2  = "0-0-some-other-value";
    std::string const json_timeout = "\"\"";
    std::string const json_string  = build_monitoring_list_request_string(json_timeout, parameter_1, parameter_2);

    wago::wdx::wda::rest::create_monitoring_list_request request;
    EXPECT_HTTP_EXCEPTION(wago::wdx::wda::http::http_status_code::bad_request,
                          deserializer.deserialize(request, json_string),
                          "String value should throw");
}


//---- End of source file ------------------------------------------------------
