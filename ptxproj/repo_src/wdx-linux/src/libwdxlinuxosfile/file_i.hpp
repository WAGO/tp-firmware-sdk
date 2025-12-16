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
///  \brief    File abstraction interface.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSFILE_FILE_I_HPP_
#define SRC_LIBWDXLINUXOSFILE_FILE_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <vector>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {

using file_descriptor     = int;
using file_descriptor_ptr = std::shared_ptr<file_descriptor>;


class file_i
{
    WC_INTERFACE_CLASS(file_i)

public:
    /// Reads the file size.
    ///
    /// \return
    ///   File size of current productive file.
    //------------------------------------------------------------------------------
    virtual uint64_t get_file_size() = 0;

    /// Reads a portion of a file.
    ///
    /// \param offset
    ///   Offset position to start read.
    /// \param length
    ///   Requested length to read. The read data may be less than requested.
    ///
    /// \throws std::out_of_range if file size would be exceeded.
    ///
    /// \throws std::runtime_error if an error made read operation impossible to succeed.
    ///
    /// \return
    ///   Byte vector of read data.
    //------------------------------------------------------------------------------
    virtual std::vector<uint8_t> read(uint64_t offset,
                                      size_t   length) = 0;

    /// Writes a portion of a file.
    ///
    /// \param offset
    ///   Offset position to start write.
    /// \param data
    ///   Data to write.
    ///
    /// \throws std::logic_error if file is readonly.
    ///
    /// \throws std::out_of_range if file size would be exceeded.
    ///
    /// \throws std::runtime_error if an error made write operation impossible to succeed.
    //------------------------------------------------------------------------------
    virtual void write(uint64_t                    offset,
                       std::vector<uint8_t> const &data) = 0;

    /// Reads the current file size of the so far written data.
    ///
    /// \throws std::logic_error if file is readonly.
    ///
    /// \return
    ///   File size of the so far written data (e.g. of an temporary file).
    //------------------------------------------------------------------------------
    virtual uint64_t get_write_file_size() = 0;

    /// Reads a portion of so far written data (e.g. for client validation).
    ///
    /// \param offset
    ///   Offset position to start read.
    /// \param length
    ///   Requested length to read. The read data may be less than requested.
    ///
    /// \throws std::logic_error if file is readonly.
    ///
    /// \throws std::out_of_range if file size would be exceeded.
    ///
    /// \throws std::runtime_error if an error made read operation impossible to succeed.
    ///
    /// \return
    ///   Byte vector of read data.
    //------------------------------------------------------------------------------
    virtual std::vector<uint8_t> read_write_file(uint64_t offset,
                                                 size_t   length) = 0;

    /// Gets file descriptor for temporary data with so far written content (e.g. for internal validation).
    ///
    /// \throws std::logic_error if file is readonly.
    ///
    /// \return
    ///   File descriptor for temporary data.
    //------------------------------------------------------------------------------
    virtual file_descriptor_ptr get_write_file_fd() = 0;

    /// Stores the file data to the final file destination and changes mode to readonly.
    ///
    /// \throws std::logic_error if file is readonly.
    ///
    /// \throws std::logic_error if there is an outstanding open file descriptor (\see get_write_file_fd()).
    ///
    /// \throws std::runtime_error if an error made store operation impossible to succeed.
    //------------------------------------------------------------------------------
    virtual void store() = 0;
};


} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSFILE_FILE_I_HPP_
//---- End of source file ------------------------------------------------------

