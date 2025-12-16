//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Class to manage chunks and check if file is completely uploaded
///
///  \author   Röh: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

#include "chunk_manager.hpp"

#include <wc/log.h>

#include <string>
#include <cmath>
#include <stdexcept>

namespace wago {
namespace wdx {
namespace linuxos {
namespace file {
namespace {

size_t calculate_sqrt_file_capacity(size_t file_capacity)
{
    return static_cast<size_t>(sqrt(static_cast<double>(file_capacity))); // parasoft-suppress CERT_C-ERR30-a "Due to beding casted from size_t file_capacity is guaranteed to be >= 0 and  != NaN. Therefore sqrt doesn't fail."
}

} // Anonymous namespace

chunk_manager::chunk_manager(size_t file_capacity)
: m_file_capacity(file_capacity)
, m_received_size(0)
, m_sqrt_file_capacity(calculate_sqrt_file_capacity(file_capacity))
{
    WC_DEBUG_LOG("Created new chunk map");
    m_chunk_map.clear();
}

void chunk_manager::add_chunk(size_t offset, size_t data_length)
{
    m_chunk_map[offset] = data_length;
    m_received_size += data_length;
    WC_DEBUG_LOG(("Added new chunk at offset " + std::to_string(offset) + " for length " + std::to_string(data_length)).c_str());
    if(get_size() > m_sqrt_file_capacity)
    {
        bool const optimized = optimize();
        if (!optimized)
        {
            throw std::runtime_error("Failed to optimize chunk map");
        }
    }
}

bool chunk_manager::file_completed()
{
    WC_DEBUG_LOG("Check for file completion");

    //Simple check, if received data is smaller as announced file size, the file can't be completed
    if(m_received_size < m_file_capacity)
    {
        return false;
    }
    auto it = m_chunk_map.begin();
    // The list is sorted by chunk offset. If the first offset is not 0, the beginning of the file is missing
    if(it->first != 0)
    {
        return false;
    }
    size_t current_position = it->first + it->second;
    while(std::next(it) != m_chunk_map.end())
    {
        size_t position_current_chunk_ends = it->first + it->second;
        // Search for the next chunk, whose data is farther in the file
        do
        {
            it++;
        }
        while(   (it->first + it->second < position_current_chunk_ends)
              && (std::next(it) != m_chunk_map.end()));
        // Only if the offset of the found chunk is smaller or equal to the data end of the current chunk,
        // no data is missing
        if(    (it->first <= position_current_chunk_ends)
            && (it->first + it->second >= position_current_chunk_ends))
        {
            current_position = it->first + it->second;
        }
        else
        {
            break;
        }
    }
    // The current position of continuous data has to be the file size, otherwise data is missing or there is more data
    // as could be written
    bool const complete = (current_position == m_file_capacity);
    WC_DEBUG_LOG((std::string("File complete: ") + (complete ? "true" : "false")).c_str());
    return complete;
}

size_t chunk_manager::get_size() const
{
    return m_chunk_map.size();
}

size_t chunk_manager::get_file_capacity() const
{
    return m_file_capacity;
}

bool chunk_manager::optimize()
{
    WC_DEBUG_LOG("Optimize chunk map");
    bool change_made = false;
    auto it = m_chunk_map.begin();
    while(std::next(it) != m_chunk_map.end())
    {
        size_t position_current_chunk_ends = it->first + it->second;
        size_t current_offset = it->first;
        it++;
        while(   std::next(it) != m_chunk_map.end()
              && (it->first <= position_current_chunk_ends))
        {
            // If chunks offset lies in between offset and data of the last chunk, the chunk can be deleted.
            // Only if its data goes farther than the last chunk, the last chunks data length needs to updated.
            if(position_current_chunk_ends < it->first + it->second)
            {
                m_chunk_map[current_offset] = it->second + it->first - current_offset;
                position_current_chunk_ends = it->second + it->first - current_offset;
            }
            it = m_chunk_map.erase(it);
            change_made = true;
        }
    }
    return change_made;
}

} // Namespace file
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago
