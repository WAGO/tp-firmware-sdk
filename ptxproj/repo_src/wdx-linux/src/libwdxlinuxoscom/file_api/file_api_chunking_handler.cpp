//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of a chunking file API handler.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_api_chunking_handler.hpp"

#include <wc/assertion.h>

#include <limits>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
file_api_chunking_handler::file_api_chunking_handler(parameter_service_file_api_i &original_interface,
                                                     ssize_t                       max_read_chunk_size,
                                                     ssize_t                       max_write_chunk_size)
: original_interface_m(original_interface)
, max_read_chunk_size_m(max_read_chunk_size)
, max_write_chunk_size_m(max_write_chunk_size)
{
    WC_ASSERT(max_read_chunk_size_m  > 0);
    WC_ASSERT(max_write_chunk_size_m > 0);
}

future<wdx::file_read_response> file_api_chunking_handler::file_read(wdx::file_id id,
                                                                     uint64_t     offset,
                                                                     size_t       length)
{
    if(length <= static_cast<size_t>(max_read_chunk_size_m))
    {
        return original_interface_m.file_read(id, offset, length);
    }

    //return original_interface_m.file_read(id, offset, max_chunk_size_m);

    auto complete_promise = std::make_shared<promise<wdx::file_read_response>>();
    data_collector data = std::make_shared<std::vector<uint8_t>>();
    auto read_future = read_more(id, offset, length, data);
    read_future.set_notifier([data, promise = complete_promise](wdx::response part_response){
        WC_ASSERT(part_response.is_determined());
        if(part_response.has_error())
        {
            promise->set_value(wdx::file_read_response(part_response.status));
        }
        else
        {
            promise->set_value(wdx::file_read_response(std::move(*data)));
        }
    });
    read_future.set_exception_notifier([promise = complete_promise](std::exception_ptr e_ptr){
        if(e_ptr)
        {
            promise->set_exception(e_ptr);
        }
    });
    return complete_promise->get_future();
}

future<wdx::response> file_api_chunking_handler::read_more(wdx::file_id   id,
                                                           uint64_t       offset,
                                                           size_t         length,
                                                           data_collector data)
{
    auto part_promise = std::make_shared<promise<wdx::response>>();
    size_t part_size  = std::min(length, static_cast<size_t>(max_read_chunk_size_m));
    future<wdx::file_read_response> part_future = original_interface_m.file_read(id, offset, part_size);
    part_future.set_notifier([this, promise = part_promise, part_size, id = std::move(id), offset, length, data = std::move(data)](wdx::file_read_response part_response) mutable {
        WC_ASSERT(part_response.is_determined());
        if(part_response.has_error())
        {
            promise->set_value(wdx::response(part_response.status));
        }
        else
        {
            data->insert(data->end(), part_response.data.begin(), part_response.data.end());
            if(part_response.data.size() == length || part_response.data.size() < part_size)
            {
                promise->set_value(wdx::response(part_response.status));
            }
            else
            {
                // Read next part
                auto next_offset = offset + part_response.data.size();
                auto next_length = length - std::min(length, part_response.data.size());
                future<wdx::response> next_future = read_more(id, next_offset, next_length, data);
                next_future.set_notifier([last_promise = promise](wdx::response next_response){
                    last_promise->set_value(wdx::response(next_response.status));
                });
                next_future.set_exception_notifier([last_promise = promise](std::exception_ptr e_ptr){
                    if(e_ptr)
                    {
                        last_promise->set_exception(e_ptr);
                    }
                });
            }
        }
    });
    part_future.set_exception_notifier([promise = part_promise](std::exception_ptr e_ptr){
        if(e_ptr)
        {
            promise->set_exception(e_ptr);
        }
    });
    return part_promise->get_future();
}

future<wdx::response> file_api_chunking_handler::file_write(wdx::file_id         id,
                                                            uint64_t             offset,
                                                            std::vector<uint8_t> data)
{
    if(data.size() <= static_cast<size_t>(max_write_chunk_size_m))
    {
        return original_interface_m.file_write(id, offset, std::move(data));
    }

    std::stack<std::vector<uint8_t>> stack_data;
    while(!data.empty())
    {
        size_t const part_size  = std::min(data.size(), static_cast<size_t>(max_write_chunk_size_m));
        WC_ASSERT(part_size <= std::numeric_limits<ssize_t>::max());
        std::vector<uint8_t> part_data(data.end() - static_cast<ssize_t>(part_size), data.end());
        stack_data.push(std::move(part_data));
        data.erase(data.end() - static_cast<ssize_t>(part_size), data.end());
    }

    return write_more(id, offset, std::move(stack_data));
}

future<wdx::response> file_api_chunking_handler::write_more(wdx::file_id id,
                                                            uint64_t     offset,
                                                            data_stack   data)
{
    // Prepare parts
    auto part_promise = std::make_shared<promise<wdx::response>>();
    std::vector<uint8_t> part_data = std::move(data.top());
    size_t part_size  = part_data.size();
    data.pop();

    // Remove part data from source vector
    future<wdx::response> part_future = original_interface_m.file_write(id, offset, std::move(part_data));
    part_future.set_notifier([this, promise = part_promise, id = std::move(id), next_offset = offset + part_size, data = std::move(data)](wdx::response part_response) mutable {
        WC_ASSERT(part_response.is_determined());
        if(part_response.has_error() || (data.empty()))
        {
            promise->set_value(wdx::response(part_response.status));
        }
        else
        {
            // Write next part
            future<wdx::response> next_future = write_more(id, next_offset, std::move(data));
            next_future.set_notifier([last_promise = promise](wdx::response next_response){
                last_promise->set_value(wdx::response(next_response.status));
            });
            next_future.set_exception_notifier([last_promise = promise](std::exception_ptr e_ptr){
                if(e_ptr)
                {
                    last_promise->set_exception(e_ptr);
                }
            });
        }
    });
    part_future.set_exception_notifier([promise = part_promise](std::exception_ptr e_ptr){
        if(e_ptr)
        {
            promise->set_exception(e_ptr);
        }
    });
    return part_promise->get_future();
}

future<wdx::file_info_response> file_api_chunking_handler::file_get_info(wdx::file_id id)
{
    return original_interface_m.file_get_info(id);
}

future<wdx::response> file_api_chunking_handler::file_create(wdx::file_id id,
                                                             uint64_t     capacity)
{
    return original_interface_m.file_create(id, capacity);
}

future<wdx::register_file_provider_response> file_api_chunking_handler::create_parameter_upload_id(wdx::parameter_instance_path context, 
                                                                                                   uint16_t timeout_seconds)
{
    return original_interface_m.create_parameter_upload_id(context, timeout_seconds);
}

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
