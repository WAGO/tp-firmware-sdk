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
///  \brief    Implementation of API-Test generic file provider
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "generic_file_provider.hpp"

#include <wago/wdx/linuxos/file/exception.hpp>
#include <wc/assertion.h>
#include <wc/log.h>

#include <sstream>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

generic_file_provider::generic_file_provider(std::vector<uint8_t> initial_content,
                                             bool                 writable)
: file_content_m(std::move(initial_content))
,capacity_m(file_content_m.size())
, writable_m(writable)
{ }

generic_file_provider::~generic_file_provider() noexcept = default;

future<response> generic_file_provider::create(uint64_t capacity)
{
    if(!writable_m)
    {
        wc_log(log_level_t::info, "Tried to get write access for readonly file");
        return resolved_future(response(wdx::status_codes::logic_error));
    }
    WC_STATIC_ASSERT(SSIZE_MAX >= INT32_MAX);
    if(capacity > INT32_MAX)
    {
        wc_log(log_level_t::info, (std::string("Tried to get write access for file exceeding INT32_MAX, with capacity: ") + std::to_string(capacity)).c_str());
        return resolved_future(response(wdx::status_codes::file_size_exceeded));
    }
    capacity_m = static_cast<size_t>(capacity); // Saving capacity as size_t but implementation is limited to SSIZE_MAX
    file_content_m = std::vector<uint8_t>();
    wc_log(log_level_t::info, (std::string("Created successfully write access for a file, capacity: ") + std::to_string(capacity)).c_str());
    return resolved_future(response(wdx::status_codes::success));
}

future<file_info_response> generic_file_provider::get_file_info()
{
    wc_log(log_level_t::info, "Got file info");
    return resolved_future(file_info_response(file_content_m.size()));
}

future<file_read_response> generic_file_provider::read(uint64_t offset, size_t length)
{
    if(offset > SSIZE_MAX || length > SSIZE_MAX || (offset + length) > SSIZE_MAX)
    {
        return resolved_future(file_read_response(wdx::status_codes::file_size_exceeded));
    }
    auto const offset_internal = static_cast<ssize_t>(offset);
    auto const length_internal = static_cast<ssize_t>(length);

    wc_log(log_level_t::info, (std::string("Read file data, offset: ") + std::to_string(offset) + ", length: " + std::to_string(length)).c_str());
    return resolved_future(file_read_response(std::vector<uint8_t>(
        std::min(file_content_m.end(), file_content_m.begin() + offset_internal),
        std::min(file_content_m.end(), file_content_m.begin() + offset_internal + length_internal)
    )));
}

future<response> generic_file_provider::write(uint64_t offset, std::vector<uint8_t> data)
{
    if(!writable_m)
    {
        wc_log(log_level_t::info, "Tried to write a readonly file");
        return resolved_future(response(wdx::status_codes::logic_error));
    }
    if(offset > SSIZE_MAX || (offset + data.size()) > SSIZE_MAX)
    {
        wc_log(log_level_t::info, (std::string("Tried to write file data exceeding SSIZE_MAX, offset: ") + std::to_string(offset) + ", length: " + std::to_string(data.size())).c_str());
        return resolved_future(response(wdx::status_codes::file_size_exceeded));
    }
    if((offset + data.size()) > capacity_m)
    {
        wc_log(log_level_t::info, (std::string("Tried to write file data exceeding capacity, offset: ") + std::to_string(offset) + ", length: " + std::to_string(data.size())).c_str());
        return resolved_future(response(wdx::status_codes::file_size_exceeded));
    }
    // optionally fill gap with zeroes
    for(size_t i = file_content_m.size(); i < offset; ++i)
    {
        file_content_m.push_back(0);
    }
    // set given data range
    auto internal_offset = static_cast<size_t>(offset);
    for(size_t i = internal_offset; i < static_cast<size_t>(offset + data.size()); ++i)
    {
        WC_ASSERT(i <= file_content_m.size());
        if(i == file_content_m.size())
        {
            file_content_m.push_back(data[i - internal_offset]);
        }
        else
        {
            file_content_m[i] = data[i - internal_offset];
        }
    }
    wc_log(log_level_t::info, (std::string("Wrote file data, offset: ") + std::to_string(offset) + ", length: " + std::to_string(data.size())).c_str());
    return resolved_future(response(wdx::status_codes::success));
}

future<bool> generic_file_provider::is_complete() const
{
    wc_log(log_level_t::info, (std::string("Checked file data to be complete, capacity: ") + std::to_string(capacity_m) + ", data: " + std::to_string(file_content_m.size())).c_str());
    return resolved_future(file_content_m.size() == capacity_m);
}

future<void> generic_file_provider::validate(file::file_validator validator) const
{
    try
    {
        if(!writable_m)
        {
            wc_log(log_level_t::info, "Tried to validate a readonly file");
            throw file::exception(wdx::status_codes::logic_error, "File not writable");
        }

        std::stringstream file_stream;
        WC_ASSERT(file_content_m.size() <= SSIZE_MAX);
        auto const file_content_size = static_cast<ssize_t>(file_content_m.size());
        file_stream.rdbuf()->pubsetbuf(const_cast<char*>(reinterpret_cast<char const *>(&file_content_m[0])), file_content_size); // parasoft-suppress CERT_CPP-EXP55-a-2 CERT_C-EXP39-b-3 "Const cast and reinterpret cast are necessary for stream buffer even for read"
        auto &validation_input = reinterpret_cast<std::basic_istream<uint8_t>&>(file_stream);
        if(!validator(validation_input))
        {
            throw file::exception(wdx::status_codes::invalid_value, "File validation failed");
        }
    }
    catch(std::exception const &e)
    {
        wc_log(log_level_t::info, "Failed to validate written file data");
        return resolved_future<void>(std::current_exception());
    }

    wc_log(log_level_t::info, "Successfully validated written file data");
    return resolved_future();
}

future<void> generic_file_provider::finish()
{
    try
    {
        if(!writable_m)
        {
            throw file::exception(wdx::status_codes::logic_error, "File not writable");
        }
        writable_m = false;
    }
    catch(std::exception const &e)
    {
        wc_log(log_level_t::info, "Failed to finally write file data");
        return resolved_future<void>(std::current_exception());
    }

    wc_log(log_level_t::info, "Successfully written file data to final destination");
    return resolved_future();
}

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
