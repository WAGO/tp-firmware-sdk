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

#ifndef SRC_LIBWDXLINUXOSFILE_CHUNK_MANAGER_HPP_
#define SRC_LIBWDXLINUXOSFILE_CHUNK_MANAGER_HPP_

#include <map>
#include <sys/types.h>

namespace wago {
namespace wdx {
namespace linuxos {
namespace file {

class chunk_manager final
{
public:
    explicit chunk_manager(size_t file_capacity);
    ~chunk_manager() noexcept = default;

    chunk_manager(chunk_manager const &)           = delete;
    chunk_manager(chunk_manager       &&) noexcept = default;

    chunk_manager & operator=(chunk_manager const &)           = delete;
    chunk_manager & operator=(chunk_manager       &&) noexcept = delete;

    // Add chunk information
    void add_chunk(size_t offset, size_t data_length);

    // Check if file is fully uploaded
    bool file_completed();

    // Function that optimizes the size in memory of the class
    // Note: Function is not optimal, needs to be called multiple times for optimal output
    bool optimize();

    // Get current amount of managed chunks
    size_t get_size() const;

    // Get current amount of managed chunks
    size_t get_file_capacity() const;

private:
    std::map<size_t, size_t>       m_chunk_map;
    size_t                   const m_file_capacity;
    size_t                         m_received_size;
    size_t                   const m_sqrt_file_capacity;

};

} // Namespace file
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago

#endif // SRC_LIBWDXLINUXOSFILE_CHUNK_MANAGER_HPP_
