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
///  \brief    Config file class to read and write small (configuration) files.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   Rhö:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_FILE_CONFIG_FILE_HPP_
#define INC_WAGO_WDX_LINUXOS_FILE_CONFIG_FILE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <string>
#include <vector>
#include <mutex>
#include <sys/types.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {

//------------------------------------------------------------------------------
/// \brief Class for abstract file access.
///
/// \remark This class is thread-safe.
///
/// This class represents small regular (config) files in Linux files systems.
//------------------------------------------------------------------------------
class WDXLINUXOSFILE_API config_file
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(config_file)

public:
    //------------------------------------------------------------------------------
    /// Constructs a class, representing a readonly (config) file.
    ///
    /// \param readonly_file_path
    ///   File path to readonly file.
    /// \param no_empty_file_on_disk
    ///   An empty file is not stored on disk.
    ///   Read operations for a non-existing file will successfully deliver empty content.
    ///   A write operation for an empty file will remove the target file.
    //------------------------------------------------------------------------------
    explicit config_file(std::string const &readonly_file_path,
                         bool               no_empty_file_on_disk = false);

    //------------------------------------------------------------------------------
    /// Constructs a class, representing a writable (config) file.
    ///
    /// \param file_path
    ///   File path for the final file in Linux file system.
    /// \param no_empty_file_on_disk
    ///   An empty file is not stored on disk.
    ///   Read operations for a non-existing file will successfully deliver empty content.
    ///   A write operation for an empty file will remove the target file.
    /// \param file_mode
    ///   File mode for the final file in Linux file system.
    ///   Mode "0000" means use of directory default.
    /// \param file_owner
    ///   File owner for the final file in Linux file system.
    ///   Will only be applied if not empty.
    //------------------------------------------------------------------------------
    config_file(std::string const &file_path,
                bool               no_empty_file_on_disk,
                mode_t             file_mode,
                std::string const &file_owner = "");

    //------------------------------------------------------------------------------
    /// Destructs class, representing a (config) file.
    //------------------------------------------------------------------------------
    virtual ~config_file() noexcept;

    //------------------------------------------------------------------------------
    /// Loads the whole file at once.
    ///
    /// \throws wdx::linuxos::file::exception if an error made read operation impossible to succeed.
    ///
    /// \return
    ///   Byte vector with file content.
    //------------------------------------------------------------------------------
    virtual std::vector<uint8_t> load();

    //------------------------------------------------------------------------------
    /// Stores the content as file.
    ///
    /// \param data
    ///   Data, representing the whole file content.
    ///
    /// \throws wdx::linuxos::file::exception
    ///   if file is readonly.
    ///   if data size exceeds \ref config_file::max_size.
    ///   if an error made write/store operation impossible to succeed.
    //------------------------------------------------------------------------------
    virtual void store(const std::vector<uint8_t> &data);

    /// Maximum supported config file size
    static constexpr size_t const max_size = 1024*1024;

    /// Configured file path for config file
    std::string             const m_file_path;

    /// Configured marker to avoid (and also not expect) an empty file on disk
    bool                    const m_no_empty_file_on_disk;

    /// Configured file size limit for config file
    uint64_t                const m_file_size_limit;

    /// Configured file mode for config file
    mode_t                  const m_file_mode;

    /// Configured file owner for config file
    std::string             const m_file_owner;

private:
    std::mutex                    m_config_mutex;
};


} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago

#endif // INC_WAGO_WDX_LINUXOS_FILE_CONFIG_FILE_HPP_
//---- End of source file ------------------------------------------------------
