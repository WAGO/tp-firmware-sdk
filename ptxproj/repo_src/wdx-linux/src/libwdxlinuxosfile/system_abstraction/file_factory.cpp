//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2022-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    System abstraction implementation for file factory.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_factory.hpp"
#include "linux_file.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {
namespace sal {

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
std::unique_ptr<file_i> file_factory::create_for_read_access(std::string const &file_path,
                                                             bool               no_empty_file_on_disk,
                                                             bool               keep_file_open)
{
    return std::make_unique<linux_file>(file_path, no_empty_file_on_disk, keep_file_open);
}

std::unique_ptr<file_i> file_factory::create_for_write_access(std::string const &file_path,
                                                              bool               no_empty_file_on_disk,
                                                              uint64_t           file_capacity,
                                                              mode_t      const  file_mode,
                                                              std::string const &file_owner)
{
    return std::make_unique<linux_file>(file_path, no_empty_file_on_disk, file_capacity, file_mode, file_owner);
}

std::unique_ptr<file_i> file_factory::create_for_write_access(std::string const &file_path,
                                                              bool               no_empty_file_on_disk,
                                                              uint64_t           file_capacity,
                                                              mode_t      const  file_mode,
                                                              uid_t       const  file_owner_id,
                                                              gid_t       const  file_group_id)
{
    return std::make_unique<linux_file>(file_path, no_empty_file_on_disk, file_capacity, file_mode, file_owner_id, file_group_id);
}

} // Namespace sal
} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


//---- End of source file ------------------------------------------------------
