//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    File operations for WAGO Parameter Service file API.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_operation.hpp"
#include "status.hpp"
#include "definitions.hpp"
#include "wago/wdx/wda/http/request_i.hpp"
#include "wago/wdx/wda/trace_routes.hpp"
#include "wda_ipc/representation.hpp"

#include <wc/trace.h>
#include <wc/log.h>

#include <regex>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace files {

// File API content type definitions
#define DEFAULT_FILE_CONTENT_TYPE_PART1                          "application"
#define DEFAULT_FILE_CONTENT_TYPE_PART2                          "octet-stream"
static constexpr char const default_file_content_type_part1[]  = DEFAULT_FILE_CONTENT_TYPE_PART1;
static constexpr char const default_file_content_type_part2[]  = DEFAULT_FILE_CONTENT_TYPE_PART2;
static constexpr char const default_file_content_type[]        = DEFAULT_FILE_CONTENT_TYPE_PART1 "/" DEFAULT_FILE_CONTENT_TYPE_PART2;

#define MULTIPART_CONTENT_TYPE_PART1                             "multipart"
#define MULTIPART_CONTENT_TYPE_PART2                             "byteranges"
static constexpr char const multipart_content_type_part1[]     = MULTIPART_CONTENT_TYPE_PART1;
static constexpr char const multipart_content_type_part2[]     = MULTIPART_CONTENT_TYPE_PART2;
static constexpr char const multipart_content_type[]           = MULTIPART_CONTENT_TYPE_PART1 "/" MULTIPART_CONTENT_TYPE_PART2;
static constexpr char const multipart_content_type_boundary[]  = "; boundary=";

// Byte range definitions used for "Accept-Ranges", "Range" and "Content-Range" header fields
// (details see https://tools.ietf.org/html/rfc7233)
#define ACCEPT_RANGES_UNIT                                       "bytes"
static constexpr char       const accept_ranges_unit[]         = ACCEPT_RANGES_UNIT;

#define BYTE_RANGE_VALUE_PATTERN                                 "(\\d*)-(\\d*)(, (\\d*)-(\\d*))*"
static constexpr char       const byte_range_request_pattern[] = ACCEPT_RANGES_UNIT "=" BYTE_RANGE_VALUE_PATTERN;
static           std::regex const byte_range_regex(byte_range_request_pattern); // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Any unexcpected exception in the init code will be noticed in the correspondig unit-tests."
static constexpr uint64_t   const suffix_range_request_value   = UINT64_MAX;

// Content-Range header see https://tools.ietf.org/html/rfc7233#section-4.2
// Example usage in multipart/byteranges see https://tools.ietf.org/html/rfc7233#section-4.1
#define CONTENT_RANGE_VALUE_PATTERN                              "(\\d+)-(\\d+)(/(\\d+|\\*))"
static constexpr char       const content_range_pattern[]      = ACCEPT_RANGES_UNIT " " CONTENT_RANGE_VALUE_PATTERN;
static           std::regex const content_range_regex(content_range_pattern); // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Any unexcpected exception in the init code will be noticed in the correspondig unit-tests."


struct request_range
{
    bool             valid            = true;
    bool             has_range_header = false;
    core_status_code core_code        = core_status_code::no_error_yet;
    uint64_t         offset           = 0;
    uint64_t         length           = UINT64_MAX;
    uint64_t         file_size        = 0;
};

using std::shared_ptr;


//------------------------------------------------------------------------------
// private function declaration
//------------------------------------------------------------------------------
static bool extract_byte_range_request(std::string  byte_range_header,
                                       uint64_t    &offset,
                                       uint64_t    &length);

static bool parse_content_range(std::string  content_range_header,
                                uint64_t    &offset,
                                uint64_t    &length,
                                uint64_t    &target_file_size);

static http::http_status_code parse_content_length(std::shared_ptr<request_i>  request,
                                                   uint64_t                   &content_length);

static http::http_status_code parse_content_length(std::string const &content_length_raw,
                                                   uint64_t          &content_length);

static wago::future<request_range> compute_range(wdx::wdmm::file_id                            const &file_id,
                                                 shared_ptr<request_i>                                request,
                                                 shared_ptr<wdx::parameter_service_file_api_i>        core_file_api);

static std::string create_content_range(uint64_t range_from,
                                        uint64_t range_to,
                                        uint64_t total_size);

static http::head_response get_error_response(wdx::status_codes core_error);

static future<bool> get_file_chunks_async(shared_ptr<wdx::parameter_service_file_api_i> file_api,
                                          shared_ptr<request_i>                         request,
                                          wdx::wdmm::file_id                            file_id,
                                          uint64_t                                      total_file_size,
                                          uint64_t                                      offset,
                                          uint64_t                                      length);

static void start_file_parts_async(shared_ptr<wdx::parameter_service_file_api_i>  file_api,
                                   shared_ptr<request_i>                          request,
                                   std::istream                                  &content_stream,
                                   wdx::wdmm::file_id                             file_id,
                                   std::string                                    boundary,
                                   uint64_t                                       target_file_size,
                                   uint64_t                                       offset,
                                   uint64_t                                       length,
                                   shared_ptr<promise<bool>>                      prom);

static future<bool> set_file_parts_async(shared_ptr<wdx::parameter_service_file_api_i>  file_api,
                                         shared_ptr<request_i>                          request,
                                         std::istream                                  &content_stream,
                                         wdx::wdmm::file_id                             file_id,
                                         std::string                                    boundary,
                                         uint64_t                                       target_file_size);

static future<bool> set_file_chunks_async(shared_ptr<wdx::parameter_service_file_api_i>  file_api,
                                          shared_ptr<request_i>                          request,
                                          std::istream                                  &content_stream,
                                          wdx::wdmm::file_id                             file_id,
                                          uint64_t                                       offset,
                                          uint64_t                                       length);

static void get_content_line(std::istream &content_stream,
                             std::string  &buffer);

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
file_operation::file_operation(std::shared_ptr<wdx::parameter_service_file_api_i> const &frontend)
: core_file_api_m(frontend)
{ }

file_operation::~file_operation() noexcept= default;

void file_operation::get_file_info(wdx::wdmm::file_id         file_id,
                                   std::unique_ptr<request_i> request)
{
    WC_DEBUG_LOG(std::string("Operation called: ") + __func__);
    std::shared_ptr<request_i> shared_request = std::move(request);
    auto range_computation = compute_range(file_id, shared_request, core_file_api_m);
    range_computation.set_notifier([shared_request](request_range &&range) {
        if(!range.valid)
        {
            // Server should send a 416 if supported ranges are invalid (see https://tools.ietf.org/html/rfc7233#section-3.1)
            shared_request->respond(http::head_response(http::http_status_code::range_not_satisfiable));
        }
        else if(wago::wdx::has_error(range.core_code))
        {
            shared_request->respond(get_error_response(range.core_code));
        }
        else
        {
            http::head_response response(range.has_range_header ? http::http_status_code::partial_content
                                                                : http::http_status_code::ok);
            response.set_content_type(default_file_content_type);
            response.set_content_length(std::to_string(range.length));
            response.set_header("Accept-Ranges", accept_ranges_unit);
            if(range.has_range_header)
            {
                response.set_header("Content-Range", create_content_range(range.offset, range.offset + range.length - 1, range.file_size));
            }
            shared_request->respond(response);
        }
        return;
    });
}

void file_operation::get_file_content(wdx::wdmm::file_id         file_id,
                                      std::unique_ptr<request_i> request)
{
    WC_DEBUG_LOG(std::string("Operation called: ") + __func__);
    std::shared_ptr<request_i> shared_request = std::move(request);
    if(!shared_request->is_response_content_type_accepted(default_file_content_type_part1, default_file_content_type_part2))
    {
        WC_DEBUG_LOG(std::string("Client does not accept default file content type \"") + default_file_content_type + "\"");
        shared_request->respond(http::head_response(http::http_status_code::not_acceptable));
        return;
    }
    auto range_computation = compute_range(file_id, shared_request, core_file_api_m);
    range_computation.set_notifier([core_file_api=core_file_api_m, file_id=std::move(file_id), shared_request](request_range &&range) {
        if(!range.valid)
        {
            // Server should send a 416 if supported ranges are invalid (see https://tools.ietf.org/html/rfc7233#section-3.1)
            shared_request->respond(http::head_response(http::http_status_code::range_not_satisfiable));
        }
        else if(wago::wdx::has_error(range.core_code))
        {
            shared_request->respond(get_error_response(range.core_code));
        }
        else if(range.length > max_download_data_length)
        {
            wc_log(log_level_t::error, "Requested range length (" + std::to_string(range.length) + " bytes) " +
                                       "for file ID \"" + file_id + "\" " +
                                       "is above internal maximum of " + std::to_string(max_download_data_length) + " bytes");
            shared_request->respond(http::head_response(http::http_status_code::range_not_satisfiable));
        }
        else
        {
            get_file_chunks_async(core_file_api, shared_request, std::move(file_id), range.file_size, range.offset, range.length);
        }
    });
}

void file_operation::create_upload_id(std::unique_ptr<request_i> request, uint16_t timeout)
{
    WC_DEBUG_LOG(std::string("Operation called: ") + __func__);
    std::string context_param;
    try 
    {
        context_param = request->get_query_parameter(context_query_param_name);
    }
    catch(...)
    {
        context_param = "";
    }

    // context MUST be set
    if (context_param.empty())
    {
        request->respond(http::head_response(http::http_status_code::bad_request));
        return;
    }

    wdx::parameter_instance_path context;
    try 
    {
        context = wda_ipc::from_string<wdx::parameter_instance_path>(context_param);
    }
    catch(...)
    {
        // the only exception to be thrown is when the context string 
        // has not been valid -> client error
        request->respond(http::head_response(http::http_status_code::bad_request));
        return;
    }

    std::shared_ptr<http::request_i> shared_request = std::move(request);
    auto pending_core_response = core_file_api_m->create_parameter_upload_id(context, timeout);
    pending_core_response.set_notifier([shared_request](auto &&core_response) {

        if (core_response.has_error()) {
            shared_request->respond(get_error_response(core_response.status));
            return;
        }

        // when new file id has been retrieved successfully, sent it back to client
        // via "Location" HTTP header
        // See https://en.wikipedia.org/w/index.php?title=HTTP_location&oldid=1063890423
        // and https://datatracker.ietf.org/doc/html/rfc7231#section-7.1.2
        // for formatting details of "Location" header
        auto const new_file_id = core_response.registered_file_id;
        WC_DEBUG_LOG(std::string("Created new file with ID: ") + new_file_id);
        http::head_response response(http::http_status_code::created);
        std::string location_header = shared_request->get_request_uri().get_path();
        if (location_header.back() != '/') {
            location_header += '/';
        }
        location_header += new_file_id;
        response.set_header("Location", location_header);

        // done, send response
        shared_request->respond(response);
    });
    pending_core_response.set_exception_notifier([shared_request](auto &&) {
        shared_request->respond(http::head_response(http::http_status_code::internal_server_error));
    });
}

void file_operation::set_file_content(wdx::wdmm::file_id         file_id,
                                      std::unique_ptr<request_i> request)
{
    WC_DEBUG_LOG(std::string("Operation called: ") + __func__);
    std::shared_ptr<request_i> shared_request = std::move(request);
    uint64_t content_length;
    http::http_status_code const parse_status = parse_content_length(shared_request, content_length);
    if(parse_status != http::http_status_code::ok)
    {
        shared_request->respond(http::head_response(parse_status));
        return;
    }
    if(shared_request->is_content_type_matching(default_file_content_type_part1, default_file_content_type_part2))
    {
        if(shared_request->get_method() != http::http_method::put)
        {
            // Wrong content type for PATCH
            WC_ASSERT(shared_request->get_method() == http::http_method::patch);
            shared_request->respond(http::head_response(http::http_status_code::unsupported_media_type));
            return;
        }
        if(shared_request->has_http_header("Content-Range"))
        {
            // Content-Range headers explicitly forbidden on PUT (see https://tools.ietf.org/html/rfc7231#section-4.3.4)
            wc_log(log_level_t::notice, "Got forbidden PUT request with Content-Range header "
                                        "for file ID \"" + file_id + "\"");
            shared_request->respond(http::head_response(http::http_status_code::bad_request));
            return;
        }
        WC_TRACE_SET_MARKER(for_route(trace_route::file_api_call), "Core operation: file create");
        auto prepare = core_file_api_m->file_create(file_id, content_length);
        prepare.set_notifier([core_file_api=core_file_api_m, file_id=std::move(file_id), shared_request, content_length] (auto prepare_response) {
            WC_TRACE_SET_MARKER(for_route(trace_route::file_api_call), "Core operation done");
            if(prepare_response.has_error())
            {
                wc_log(log_level_t::warning, "File create operation failed in write request for file ID \"" + 
                                             file_id + "\" (" + wdx::to_string(prepare_response.status) + ")");
                shared_request->respond(get_error_response(prepare_response.status));
            }
            else
            {
                auto process = set_file_chunks_async(core_file_api, shared_request, shared_request->get_content_stream(), file_id, 0, content_length);
                process.set_notifier([file_id=std::move(file_id), shared_request] (auto ok) {
                    if(ok)
                    {
                        shared_request->respond(http::head_response(http::http_status_code::no_content));
                    }
                    else
                    {
                        wc_log(log_level_t::error, "Failed to set file chunks");
                    }
                });
            }
        });
    }
    else if(shared_request->is_content_type_matching(multipart_content_type_part1, multipart_content_type_part2))
    {
        if(shared_request->get_method() != http::http_method::patch)
        {
            // Wrong content type for PUT
            WC_ASSERT(shared_request->get_method() == http::http_method::put);
            shared_request->respond(http::head_response(http::http_status_code::unsupported_media_type));
            return;
        }
        std::string raw_content_type = shared_request->get_content_type();
        size_t const boundary_pos = raw_content_type.find(multipart_content_type_boundary);
        if(boundary_pos == std::string::npos)
        {
            wc_log(log_level_t::notice, "Got PATCH request without multipart content boundary definition "
                                        "for file ID \"" + file_id + "\"");
            shared_request->respond(http::head_response(http::http_status_code::bad_request));
        }
        else
        {
            std::istream &content_stream = shared_request->get_content_stream();
            std::string   boundary = raw_content_type.substr(boundary_pos + sizeof(multipart_content_type_boundary) - 1);
            std::string   buff;
            get_content_line(content_stream, buff);
            if(buff != ("--" + boundary))
            {
                wc_log(log_level_t::notice, "Got PATCH request without valid multipart content boundary begin in body "
                                            "for file ID \"" + file_id + "\"");
                shared_request->respond(http::head_response(http::http_status_code::bad_request));
            }
            else
            {
                set_file_parts_async(core_file_api_m, shared_request, content_stream, std::move(file_id), boundary, 0);
            }
        }
    }
    else
    {
        shared_request->respond(http::head_response(http::http_status_code::unsupported_media_type));
    }
}


//------------------------------------------------------------------------------
// private function implementation
//------------------------------------------------------------------------------
static bool extract_byte_range_request(std::string  byte_range_header,
                                       uint64_t    &offset,
                                       uint64_t    &length)
{
    std::smatch matches;
    if(std::regex_match(byte_range_header, matches, byte_range_regex))
    {
        uint64_t range_from = 0;
        uint64_t range_to   = UINT64_MAX - 1;
        if(!matches[1].str().empty())
        {
            range_from = std::stoull(matches[1]);
        }
        if(!matches[2].str().empty())
        {
            range_to   = std::stoull(matches[2]);
        }
        if((range_to >= range_from) && (range_to < UINT64_MAX))
        {
            if(matches[1].str().empty()) // Suffix byte range spec (details see https://tools.ietf.org/html/rfc7233#section-2.1)
            {
                // We should send the last "range_to" bytes only
                offset = suffix_range_request_value;
                length = range_to;
            }
            else
            {
                offset = range_from;
                length = 1 + range_to - range_from;
            }
            return true;
        }
        // Ignoring further ranges according to https://tools.ietf.org/html/rfc7233#section-4.4
    }
    return false;
}

static bool parse_content_range(std::string  content_range_header,
                                uint64_t    &offset,
                                uint64_t    &length,
                                uint64_t    &target_file_size)
{
    std::smatch matches;
    if(std::regex_match(content_range_header, matches, content_range_regex))
    {
        uint64_t range_from = 0;
        uint64_t range_to   = UINT64_MAX - 1;
        range_from = std::stoull(matches[1]);
        range_to   = std::stoull(matches[2]);
        if((range_to >= range_from) && (range_to < UINT64_MAX))
        {
            offset = range_from;
            length = 1 + range_to - range_from;
            if(matches[3].str() == "/*")
            {
                target_file_size = 0;
            }
            else
            {
                target_file_size = std::stoull(matches[3].str().substr(1));
            }
            return true;
        }
    }
    return false;
}

static http::http_status_code parse_content_length(std::string const &content_length_raw,
                                                   uint64_t          &content_length)
{
    http::http_status_code result = http::http_status_code::ok;

    if(content_length_raw.empty())
    {
        result = http::http_status_code::length_required;
    }
    else
    {
        try
        {
            std::size_t pos = 0;
            content_length = std::stoull(content_length_raw, &pos);
            if(pos != content_length_raw.length())
            {
                result = http::http_status_code::bad_request;
            }
            else if(content_length > max_upload_request_length)
            {
                result = http::http_status_code::payload_too_large;
            }
        }
        catch(...)
        {
            result = http::http_status_code::bad_request;
        }
    }

    return result;
}

static http::http_status_code parse_content_length(std::shared_ptr<request_i>  request,
                                                   uint64_t                   &content_length)
{
    std::string content_length_raw = request->get_content_length();
    return parse_content_length(content_length_raw, content_length);
}

static wago::future<request_range> compute_range(wdx::wdmm::file_id                            const &file_id,
                                                 shared_ptr<request_i>                                request,
                                                 shared_ptr<wdx::parameter_service_file_api_i>        core_file_api)
{
    request_range result;

    
    result.has_range_header = request->has_http_header("Range");
    if(result.has_range_header)
    {
        auto range_header = request->get_http_header("Range");
        if(!extract_byte_range_request(range_header, result.offset, result.length))
        {
            result.valid = false;
            return resolved_future(std::move(result));
        }
    }
    auto prom = std::make_shared<promise<request_range>>();
    WC_TRACE_SET_MARKER(for_route(trace_route::file_api_call), "Core operation: get file info");
    auto file_info_coming_soon = core_file_api->file_get_info(file_id);
    file_info_coming_soon.set_notifier([=](wdx::file_info_response &&file_info) {
        WC_TRACE_SET_MARKER(for_route(trace_route::file_api_call), "Core operation done");
        auto new_result = result;
        if(file_info.has_error())
        {
            wc_log(log_level_t::error, "Failed to get file information for file ID \"" + file_id +
                                       "\" (" + wdx::to_string(file_info.status) + "): " +
                                       file_info.get_message());
            new_result.core_code = file_info.status;
        }
        else
        {
            new_result.file_size = file_info.file_size;
            if(new_result.offset == suffix_range_request_value)
            {
                new_result.offset = new_result.file_size - std::min(new_result.length, new_result.file_size);
            }
            if(new_result.offset > new_result.file_size)
            {
                new_result.valid = false;
            }
            else
            {
                new_result.length = std::min(new_result.length, new_result.file_size - new_result.offset);
            }
        }
        prom->set_value(std::move(new_result));
        return;
    });

    return prom->get_future();
}

static std::string create_content_range(uint64_t range_from,
                                        uint64_t range_to,
                                        uint64_t total_size)
{
    std::string const content_range = std::string(accept_ranges_unit) + " "
                                    + std::to_string(range_from) + "-" + std::to_string(range_to)
                                    + "/" + std::to_string(total_size);
    WC_ASSERT(std::regex_match(content_range, content_range_regex));
    return content_range;
}

static http::head_response get_error_response(wdx::status_codes core_error)
{
    http::http_status_code status;
    switch(core_error)
    {
        // these status codes should only occur on POST /files?context=...
        case core_status_code::unknown_device_collection:
        case core_status_code::unknown_device:
        case core_status_code::unknown_parameter_path:
        case core_status_code::unknown_class_instance_path: // TODO: really?
        case core_status_code::not_a_file_id:
        case core_status_code::logic_error:
            status = http::http_status_code::bad_request;
            break;
        // well get this only when POST /files failed because of too many existing file ids
        // earlier retrieved by POSTs
        case core_status_code::upload_id_max_exceeded:
            status = http::http_status_code::internal_server_error;
            break;

        // rest of status codes should only occur when operating on a file via
        // /files/<file_id>
        case core_status_code::invalid_value:
        case core_status_code::file_size_exceeded:
            status = http::http_status_code::bad_request;
            break;
        case core_status_code::unknown_file_id:
            status = http::http_status_code::not_found;
            break;

        // if a user cannot access a file context, he cannot access the file
        case core_status_code::unauthorized:
            status = http::http_status_code::forbidden;
            break;

        // this case can mean a lot of things. However, because we're missing status code to 
        // differentiate, we'll use 500 Server Error for every error possible here.
        case core_status_code::file_not_accessible:
            status = http::http_status_code::internal_server_error;
            break;

        default:
            status = http::http_status_code::internal_server_error;
    }
    return http::head_response(status);
}

static future<bool> get_file_chunks_async(shared_ptr<wdx::parameter_service_file_api_i> file_api,
                                          shared_ptr<request_i>                         request,
                                          wdx::wdmm::file_id                            file_id,
                                          uint64_t                                      total_file_size,
                                          uint64_t                                      offset,
                                          uint64_t                                      length)
{
    WC_DEBUG_LOG(std::string("Function called: ") + __func__);
    WC_ASSERT((offset + length) >= offset);
    WC_ASSERT((offset + length) >= length);
    WC_ASSERT((offset + length) <= total_file_size);

    auto prom = std::make_shared<promise<bool>>();
    WC_ASSERT(download_chunk_size <= SIZE_MAX);
    auto chunk_length = static_cast<size_t>(std::min(download_chunk_size, length));
    WC_TRACE_SET_MARKER(for_route(trace_route::file_api_call), "Core operation: read file chunk");
    auto incoming_chunk = file_api->file_read(file_id, offset, chunk_length);
    incoming_chunk.set_notifier([=](wdx::file_read_response &&chunk) {
        WC_TRACE_SET_MARKER(for_route(trace_route::file_api_call), "Core operation done");
        if(chunk.has_error())
        {
            wc_log(log_level_t::error, "Failed to read file chunk for file ID \"" + file_id +
                                       "\" (" + wdx::to_string(chunk.status) + ")");
            if(!request->is_responded())
            {
                request->respond(get_error_response(chunk.status));
            }
            else
            {
                // We cannot do anything about this. skip and stop further processing
                // The client should manually continue by using a Range header GET request
            }
            prom->set_value(true);
        }
        else
        {
            // respond on first chunk
            if(!request->is_responded())
            {
                http::http_status_code http_code;
                std::string content_range_header;
                auto range_from = offset;
                auto range_to   = std::min(offset + length - 1, total_file_size - 1);
                if(total_file_size == length)
                {
                    http_code = http::http_status_code::ok;
                }
                else
                {
                    http_code = http::http_status_code::partial_content;
                    content_range_header = create_content_range(range_from, range_to, total_file_size);
                }
                http::head_response response(http_code);
                response.set_content_type(default_file_content_type);
                response.set_content_length(std::to_string(1 + range_to - range_from));
                response.set_header("Accept-Ranges", accept_ranges_unit);
                if(!content_range_header.empty())
                {
                    response.set_header("Content-Range", content_range_header);
                }
                request->respond(response);
            }
            // send chunk data
            request->send_data(reinterpret_cast<char *>(chunk.data.data()), chunk.data.size());
            if(chunk_length < length)
            {
                uint64_t next_offset = offset + chunk_length;
                auto next_length = length - chunk_length;
                auto get_next_chunk = get_file_chunks_async(file_api, request, std::move(file_id), total_file_size, next_offset, next_length);
                get_next_chunk.set_notifier([prom] (bool) {
                    prom->set_value(true);
                });
            }
            else
            {
                prom->set_value(true);
            }
        }
        // just abort processing on error?
    });
    return prom->get_future();
}

static void start_file_parts_async(shared_ptr<wdx::parameter_service_file_api_i> file_api,
                                   shared_ptr<request_i>                         request,
                                   std::istream                                 &content_stream,
                                   wdx::wdmm::file_id                            file_id,
                                   std::string                                   boundary,
                                   uint64_t                                      target_file_size,
                                   uint64_t                                      offset,
                                   uint64_t                                      length,
                                   shared_ptr<promise<bool>>                     prom)
{

    auto set_file_part = set_file_chunks_async(file_api, request, content_stream, file_id, offset, length);
    set_file_part.set_notifier([prom, file_api, request, &content_stream, file_id, boundary, target_file_size] (bool ok) {
        if(!ok)
        {
            prom->set_value(true);
        }
        else
        {
            // expect boundary
            std::string next_line;
            // skip one line seperator
            get_content_line(content_stream, next_line);
            if(next_line.empty())
            {
                get_content_line(content_stream, next_line);
                if(next_line == ("--" + boundary + "--"))
                {
                    // we're done
                    request->respond(http::head_response(http::http_status_code::no_content));
                    prom->set_value(true);
                }
                else if(next_line == ("--" + boundary))
                {
                    // we need to process another part
                    auto recursive_set_part = set_file_parts_async(file_api, request, content_stream, file_id, boundary, target_file_size);
                    recursive_set_part.set_notifier([prom](bool) {
                        prom->set_value(true);
                    });
                }
                else
                {
                    // something is wrong with this body...
                    wc_log(log_level_t::notice, "Got PATCH request with wrong body format (boundary expected) "
                                                "for file ID \"" + file_id + "\"");
                    request->respond(http::head_response(http::http_status_code::bad_request));
                    prom->set_value(true);
                }
            }
            else
            {
                // something is wrong with this body...
                wc_log(log_level_t::notice, "Got PATCH request with wrong body format (empty line expected) "
                                            "for file ID \"" + file_id + "\"");
                request->respond(http::head_response(http::http_status_code::bad_request));
                prom->set_value(true);
            }
        }
    });
}

static future<bool> set_file_parts_async(shared_ptr<wdx::parameter_service_file_api_i>  file_api,
                                         shared_ptr<request_i>                          request,
                                         std::istream                                  &content_stream,
                                         wdx::wdmm::file_id                             file_id,
                                         std::string                                    boundary,
                                         uint64_t                                       target_file_size)
{
    auto prom = std::make_shared<promise<bool>>();
    // get part headers
    std::map<std::string, std::string> part_headers;
    std::string buff;
    get_content_line(content_stream, buff);
    while(!buff.empty()) {
        auto header_seperator_pos = buff.find(": ");
        if(header_seperator_pos == std::string::npos)
        {
            wc_log(log_level_t::notice, "Got PATCH request with wrong body format (header line expected) "
                                        "for file ID \"" + file_id + "\"");
            request->respond(http::head_response(http::http_status_code::bad_request));
            prom->set_value(true);
            return prom->get_future();
        }
        part_headers.emplace(buff.substr(0, header_seperator_pos), buff.substr(header_seperator_pos + 2));
        get_content_line(content_stream, buff);
    }

    // again: check headers like it was a default request
    if(   (part_headers.count("Content-Type")  == 0)
       || (part_headers.count("Content-Range") == 0))
    {
        wc_log(log_level_t::notice, "Got PATCH request with wrong body format (header \"Content-Type\" & \"Content-Range\" expected) "
                                    "for file ID \"" + file_id + "\"");
        request->respond(http::head_response(http::http_status_code::bad_request));
        prom->set_value(true);
    }
    else
    {
        std::string content_type   = part_headers.at("Content-Type");
        std::string content_range  = part_headers.at("Content-Range");
        if(default_file_content_type != content_type)
        {
            request->respond(http::head_response(http::http_status_code::unsupported_media_type));
            prom->set_value(true);
        }
        else
        {
            uint64_t offset         = 0;
            uint64_t length         = 0;
            uint64_t part_file_size = 0;
            if(!parse_content_range(content_range, offset, length, part_file_size))
            {
                wc_log(log_level_t::notice, "Got PATCH request with wrong body format (unexpected content range format) "
                                            "for file ID \"" + file_id + "\"");
                request->respond(http::head_response(http::http_status_code::bad_request));
                prom->set_value(true);
            }
            else if(target_file_size == 0)
            {
                WC_ASSERT(length < max_upload_request_length);
                // First range, check if we need a create call
                if(part_file_size == 0)
                {
                    request->respond(http::head_response(http::http_status_code::length_required));
                    prom->set_value(true);
                    return prom->get_future();
                }
                target_file_size = part_file_size;
                WC_TRACE_SET_MARKER(for_route(trace_route::file_api_call), "Core operation: get file info");
                auto file_size_info = file_api->file_get_info(file_id);
                file_size_info.set_notifier([file_api, request, &content_stream=content_stream, file_id, boundary, target_file_size, offset, length, prom] (auto file_size_info_response) {
                    WC_TRACE_SET_MARKER(for_route(trace_route::file_api_call), "Core operation done");
                    if(file_size_info_response.has_error())
                    {
                        wc_log(log_level_t::error, "Failed to get file information for file ID \"" + file_id +
                                                   "\" (" + wdx::to_string(file_size_info_response.status) + "): " +
                                                   file_size_info_response.get_message());
                        request->respond(get_error_response(file_size_info_response.status));
                    }
                    else
                    {
                        // If file does not contain any content we have to invoke an create call
                        if(file_size_info_response.file_size == 0)
                        {
                            WC_TRACE_SET_MARKER(for_route(trace_route::file_api_call), "Core operation: file create");
                            auto prepare = file_api->file_create(file_id, target_file_size);
                            prepare.set_notifier([file_api, request, &content_stream=content_stream, file_id, boundary, target_file_size, offset, length, prom] (auto prepare_response) {
                                WC_TRACE_SET_MARKER(for_route(trace_route::file_api_call), "Core operation done");
                                if(prepare_response.has_error())
                                {
                                    wc_log(log_level_t::warning, "File create operation failed in write request for file ID \"" +
                                                                 file_id + "\" (" + wdx::to_string(prepare_response.status) + ")");
                                    request->respond(get_error_response(prepare_response.status));
                                }
                                else
                                {
                                    start_file_parts_async(file_api, request, content_stream, file_id, boundary, target_file_size, offset, length, prom);
                                }
                            });
                        }
                        else
                        {
                            start_file_parts_async(file_api, request, content_stream, file_id, boundary, target_file_size, offset, length, prom);
                        }
                    }
                });
            }
            else
            {
                WC_ASSERT(length < max_upload_request_length);
                start_file_parts_async(file_api, request, content_stream, file_id, boundary, target_file_size, offset, length, prom);
            }
        }
    }
    return prom->get_future();
}

static future<bool> set_file_chunks_async(shared_ptr<wdx::parameter_service_file_api_i>  file_api,
                                          shared_ptr<request_i>                          request,
                                          std::istream                                  &content_stream,
                                          wdx::wdmm::file_id                             file_id,
                                          uint64_t                                       offset,
                                          uint64_t                                       length)
{
    WC_DEBUG_LOG(std::string("Function called: ") + __func__);
    WC_ASSERT((offset + length) >= offset);
    WC_ASSERT((offset + length) >= length);

    WC_STATIC_ASSERT(upload_chunk_size <= std::numeric_limits<std::streamsize>::max());
    auto chunk_length = static_cast<std::streamsize>(std::min(upload_chunk_size, length));

    WC_STATIC_ASSERT(std::numeric_limits<size_t>::max() >= std::numeric_limits<decltype(chunk_length)>::max());
    std::vector<uint8_t> chunk(static_cast<size_t>(chunk_length));

    content_stream.read(reinterpret_cast<char *>(chunk.data()), chunk_length); // parasoft-suppress CERT_C-EXP39-b-3 "File API works with uint8_t while stream works on char. In Both cases content is interpreted as binary data."
    if(!content_stream.good())
    {
        if(content_stream.gcount() > 0)
        {
            chunk_length = content_stream.gcount();
        }
        else
        {
            wc_log(log_level_t::notice, "Found erroneous request body on try to write a file chunk "
                                        "for file ID \"" + file_id + "\"");
            request->respond(http::head_response(http::http_status_code::bad_request));
            return resolved_future(false);
        }
    }
    auto prom = std::make_shared<promise<bool>>();
    WC_TRACE_SET_MARKER(for_route(trace_route::file_api_call), "Core operation: write file chunk");
    auto outgoing_chunk = file_api->file_write(file_id, offset, chunk);
    outgoing_chunk.set_notifier([=, &content_stream](wdx::response &&write_response) {
        WC_TRACE_SET_MARKER(for_route(trace_route::file_api_call), "Core operation done");
        if(write_response.has_error())
        {
            wc_log(log_level_t::warning, "File write operation failed for file ID \"" +
                                         file_id + "\" (" + wdx::to_string(write_response.status) + ")");
            // send error immediately when it occurs. Success as soon as all chunks have been written.
            if(!request->is_responded())
            {
                request->respond(get_error_response(write_response.status));
                prom->set_value(false);
            }
            else
            {
                // We cannot do anything about this. skip and stop further processing
                // The client should manually continue by using a Range header GET request
                prom->set_value(false);
            }
        }
        else
        {
            // send chunk data
            WC_STATIC_ASSERT(std::numeric_limits<uint64_t>::max() >= std::numeric_limits<decltype(chunk_length)>::max());
            uint64_t const next_offset = offset + static_cast<uint64_t>(chunk_length);
            uint64_t const next_length = length - static_cast<uint64_t>(chunk_length);
            if(next_length > 0)
            {
                auto recursive_chunk = set_file_chunks_async(file_api, request, content_stream, file_id, next_offset, next_length);
                recursive_chunk.set_notifier([prom](bool ok) {
                    prom->set_value(std::move(ok));
                });
            }
            else
            {
                // done
                prom->set_value(true);
            }
        }
    });
    return prom->get_future();
}

static void get_content_line(std::istream &content_stream, std::string &buffer)
{
    buffer.clear();

    bool carriage_return_found = false;
    bool new_line_found        = false;
    int single_char;
    while((single_char = content_stream.get()) != EOF) // parasoft-suppress CERT_C-EXP45-d-2 "Compliant solution - Intentional Assignment. Parasoft has a bug here."
    {
        char const candidate = static_cast<char>(single_char);
        if(!carriage_return_found && (candidate == '\r'))
        {
            carriage_return_found = true;
        }
        else if(!new_line_found && (candidate == '\n'))
        {
            new_line_found = true;
        }
        else if(carriage_return_found || new_line_found)
        {
            content_stream.unget();
            break;
        }
        else
        {
            buffer.push_back(candidate);
        }
    }
}


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
