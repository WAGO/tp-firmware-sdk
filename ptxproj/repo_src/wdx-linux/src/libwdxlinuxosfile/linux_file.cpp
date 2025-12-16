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
///  \brief    Implementation of Linux file abstraction class.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "linux_file.hpp"
#include "system_abstraction_file.hpp"
#include "errno_utils.hpp"

#include <wc/assertion.h>
#include <wc/structuring.h>
#include <wc/log.h>

#include <stdexcept>
#include <cstring>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {

using c_string_ptr    = std::unique_ptr<char,            void(*)(char*)>;
using unique_fd_ptr   = std::unique_ptr<file_descriptor, void(*)(file_descriptor*)>;
using unique_fmem_ptr = std::unique_ptr<uint8_t,         void(*)(uint8_t*)>;

//------------------------------------------------------------------------------
// Internal function prototypes
//------------------------------------------------------------------------------
namespace {
void free_c_string(char *str);
void change_file_owner(file_descriptor const fd,
                       uid_t           const user,
                       gid_t           const group);
uint64_t get_file_size_internal(std::string const &file_path,
                                bool        const  allow_missing_file);
std::vector<uint8_t> read_internal(file_descriptor const fd,
                                   uint64_t        const file_size,
                                   uint64_t        const offset,
                                   size_t          const length);
}

//------------------------------------------------------------------------------
// Class implementation
//------------------------------------------------------------------------------
linux_file::linux_file(std::string const &file_path,
                       bool               no_empty_file_on_disk,
                       bool               keep_file_open)
: m_file_path(file_path)
, m_no_empty_file_on_disk(no_empty_file_on_disk)
, m_file_capacity(0)
, m_keep_file_open(keep_file_open)
, m_readonly(true)
{
    WC_ASSERT(!m_file_path.empty());
}

linux_file::linux_file(std::string const &file_path,
                       bool               no_empty_file_on_disk,
                       uint64_t           file_capacity,
                       mode_t      const  file_mode,
                       std::string const &file_owner)
: linux_file(file_path, no_empty_file_on_disk, file_capacity, file_mode)
{
    if(!m_no_empty_file_on_disk || (m_file_capacity > 0))
    {
        // Change file owner if requested
        if(!file_owner.empty())
        {
            WC_ASSERT(m_open_file.get() != nullptr);
            if (m_open_file.get() == nullptr) {
                // This case should not happen, because file is opened by
                // other constructor but it is better to be safe than sorry 
                throw std::runtime_error("Failed to open file.");
            }
            uid_t uid;
            gid_t gid;
            sal::filemanagement::get_instance().get_user_info(file_owner, uid, gid);
            change_file_owner(*m_open_file, uid, gid);
        }
    }
}

linux_file::linux_file(std::string const &file_path,
                       bool               no_empty_file_on_disk,
                       uint64_t           file_capacity,
                       mode_t      const  file_mode,
                       uid_t       const  file_owner_id,
                       gid_t       const  file_group_id)
: linux_file(file_path, no_empty_file_on_disk, file_capacity, file_mode)
{
    if(!m_no_empty_file_on_disk || (m_file_capacity > 0))
    {
        WC_ASSERT(m_open_file.get() != nullptr);
        if (m_open_file.get() == nullptr) {
            // This case should not happen, because file is opened by
            // other constructor but it is better to be safe than sorry 
            throw std::runtime_error("Failed to open file.");
        }
        change_file_owner(*m_open_file, file_owner_id, file_group_id);
    }
}

linux_file::linux_file(std::string const &file_path,
                       bool               no_empty_file_on_disk,
                       uint64_t           file_capacity,
                       mode_t      const  file_mode)
: m_file_path(file_path)
, m_no_empty_file_on_disk(no_empty_file_on_disk)
, m_file_capacity(file_capacity)
, m_keep_file_open(true)
, m_readonly(false)
{
    WC_ASSERT(!m_file_path.empty());
    WC_ASSERT(file_mode <= 0777);

    auto last_slash_pos = m_file_path.find_last_of('/');
    m_directory_path = last_slash_pos != std::string::npos 
                     ? m_file_path.substr(0, last_slash_pos)
                     :  ".";
    if(!m_no_empty_file_on_disk || (m_file_capacity > 0))
    {
        // Check free space
        uint64_t free_space = 0;
        if(sal::filemanagement::get_instance().get_file_system_space(m_directory_path, free_space) != 0)
        {
            std::string error_message = std::string("Failed to check file system space for write operations: ")
                                      + errno_to_string(errno);
            throw std::runtime_error(error_message);
        }
        if(free_space < m_file_capacity)
        {
            throw std::runtime_error(("Not enough space available in \"" + m_directory_path + "\"").c_str());
        }

        // Delete old temporary files
        sal::filemanagement::get_instance().delete_old_temp_files(m_file_path, ".tmp");
        // Create temporary file
        file_descriptor fd = sal::filemanagement::get_instance().create_and_open_temp_file(m_file_path, ".tmp", m_temp_file_path);
        if(fd < 0)
        {
            std::string error_message = std::string("Failed to create temporarily file for write operations: ")
                                      + errno_to_string(errno);
            throw std::runtime_error(error_message);
        }
        m_open_file = std::make_shared<file_descriptor>(fd);

        // Memory map the file to write may be an option to enhance performance,
        // but writing to a memory mapped file forces an application to deal with SIGSEGV and SIGBUS,
        // refer to: https://man7.org/linux/man-pages/man2/mmap.2.html (heading "ERRORS")

        wc_log(log_level_t::info, (std::string("Opened file \"") + m_temp_file_path + "\" for write operations").c_str());

        // Change file mode (access rights) if requested
        if(file_mode > 0000)
        {
            if(sal::filemanagement::get_instance().fchmod(*m_open_file, file_mode) < 0)
            {
                std::string error_message = std::string("Failed to change file mode for \"") + m_temp_file_path + "\": "
                                          + errno_to_string(errno);
                throw std::runtime_error(error_message);
            }
            wc_log(log_level_t::info, (std::string("Changed file mode for \"") + m_temp_file_path + "\" to " + std::to_string(file_mode)).c_str());
        }
    }
}

linux_file::~linux_file() noexcept
{
    if(m_open_file.get() != nullptr)
    {
        sal::filemanagement::get_instance().close(*m_open_file);
        wc_log(log_level_t::info, (std::string("Closed file \"") + m_file_path + "\"").c_str()); // parasoft-suppress CERT_CPP-DCL57-a CERT_CPP-ERR50-b "bad_alloc exception is not thrown on Linux"
    }

    if(!m_temp_file_path.empty())
    {
        WC_ASSERT(!m_readonly);
        if(sal::filemanagement::get_instance().unlink(m_temp_file_path) != 0)
        {
            std::string error_message = std::string("Failed to cleanup temporary file \"") + m_temp_file_path + "\": " // parasoft-suppress CERT_CPP-DCL57-a CERT_CPP-ERR50-b "std::string ctor does not throw bad_alloc exceptions on a linux system"
                                      + errno_to_string(errno);
            wc_log(log_level_t::error, error_message.c_str());
        }
        else
        {
            wc_log(log_level_t::info, (std::string("Cleaned temporary file \"") + m_temp_file_path + "\"").c_str()); // parasoft-suppress CERT_CPP-DCL57-a CERT_CPP-ERR50-b "std::string ctor does not throw bad_alloc exceptions on a linux system"
        }
    }
}

linux_file::linux_file(linux_file &&src) noexcept
:m_file_path(src.m_file_path)
,m_no_empty_file_on_disk(src.m_no_empty_file_on_disk)
,m_file_capacity(src.m_file_capacity)
,m_directory_path(std::move(src.m_directory_path))
,m_temp_file_path(std::move(src.m_temp_file_path))
,m_open_file(src.m_open_file)
,m_keep_file_open(src.m_keep_file_open)
,m_readonly(src.m_readonly)
{ }

uint64_t linux_file::get_file_size()
{
    return get_file_size_internal(m_file_path, m_no_empty_file_on_disk);
}

// Reads a portion of a file
std::vector<uint8_t> linux_file::read(uint64_t offset,
                                      size_t   length)
{
    // Open file permanently if requested
    if(m_readonly && m_keep_file_open && (m_open_file.get() == nullptr))
    {
        file_descriptor fd = sal::filemanagement::get_instance().open_readonly(m_file_path);
        if(fd < 0)
        {
            if(m_no_empty_file_on_disk && (errno == ENOENT))
            {
                if (offset == 0)
                {
                    return std::vector<uint8_t>();
                }
                else
                {
                    throw std::out_of_range("Failed to read from empty file with offset > 0");
                }
            }
            else
            {
                std::string error_message = std::string("Failed to open file for read operations: ")
                                          + errno_to_string(errno);
                throw std::runtime_error(error_message);
            }
        }
        m_open_file = std::make_shared<file_descriptor>(fd);

        wc_log(log_level_t::info, (std::string("Opened file \"") + m_file_path + "\" for read operations").c_str());
    }

    // Open original file if currently a temporary file is open for write operations
    file_descriptor read_fd;
    unique_fd_ptr   read_fd_ptr(nullptr, [](file_descriptor*){});
    if(m_readonly && m_keep_file_open)
    {
        read_fd     = *m_open_file;
        read_fd_ptr = unique_fd_ptr(&read_fd, [](file_descriptor* ptr) // parasoft-suppress CERT_CPP-MEM56-a "This unique pointer is not intended to free memory"
        {
            WC_UNUSED_DATA(ptr); // Do NOT close
        });
        WC_DEBUG_LOG((std::string("Opened temporary data file \"") + m_temp_file_path + "\" for read operations").c_str());
    }
    else
    {
        read_fd = sal::filemanagement::get_instance().open_readonly(m_file_path);
        if(read_fd < 0)
        {
            if(m_no_empty_file_on_disk && (errno == ENOENT))
            {
                return std::vector<uint8_t>();
            }
            else
            {
                std::string error_message = std::string("Failed to open file for read operations: ")
                                          + errno_to_string(errno);
                throw std::runtime_error(error_message);
            }
        }
        read_fd_ptr = unique_fd_ptr(&read_fd, [](file_descriptor* ptr)
        {
            sal::filemanagement::get_instance().close(*ptr);
        });
        WC_DEBUG_LOG((std::string("Opened file \"") + m_file_path + "\" for read operations").c_str());
    }

    return read_internal(*read_fd_ptr, get_file_size_internal(m_file_path, true), offset, length);
}

void linux_file::write(uint64_t                    offset,
                       std::vector<uint8_t> const &data)
{
    if(m_readonly)
    {
        throw std::logic_error("Tried to write a readonly file");
    }

    size_t const length = data.size();
    if(    ((offset + length) < offset)       // overflow
        || ((offset + length) < length)       // overflow
        || ((offset + length) > m_file_capacity))
    {
        throw std::out_of_range("Result of offset + length is out of file range");
    }

    // Implementation is limited to size_t
    if((offset + length) > SIZE_MAX)
    {
        throw std::out_of_range("Result of offset + length is out of " + std::to_string(sizeof(size_t) * 8) + " bit range");
    }
    WC_ASSERT(offset < SIZE_MAX);
    auto const internal_offset = static_cast<size_t>(offset);


    if(data.empty())
    {
        wc_log(log_level_t::debug, "Skip file write for write length of 0");
    }
    else
    {
        // TODO: Check if keeping track of local seek variable increases performance
        wc_log(log_level_t::debug, ("Seek for write operation to offset: " + std::to_string(internal_offset)).c_str());
        if(sal::filemanagement::get_instance().seek_to_offset(*m_open_file, internal_offset) < 0)
        {
            std::string error_message = std::string("Failed to seek file for write operation: ")
                                      + errno_to_string(errno);
            throw std::runtime_error(error_message);
        }

        if(sal::filemanagement::get_instance().write(*m_open_file, data) < 0)
        {
            std::string error_message = std::string("Failed to write data to file: ")
                                      + errno_to_string(errno);
            throw std::runtime_error(error_message);
        }
    }
}

uint64_t linux_file::get_write_file_size()
{
    if(m_readonly)
    {
        throw std::logic_error("Tried to get written data file size of a readonly file");
    }

    if(m_no_empty_file_on_disk && (m_file_capacity == 0))
    {
        return 0;
    }
    else
    {
        return get_file_size_internal(m_temp_file_path, true);
    }
}

// Reads a portion of so far written data from temporary file (e.g. for client validation)
std::vector<uint8_t> linux_file::read_write_file(uint64_t offset,
                                                 size_t   length)
{
    if(m_readonly)
    {
        throw std::logic_error("Tried to get written data of a readonly file");
    }

    if(m_no_empty_file_on_disk && (m_file_capacity == 0))
    {
        return std::vector<uint8_t>();
    }
    else
    {
        unique_fd_ptr read_fd_ptr(m_open_file.get(), [](file_descriptor* ptr) // parasoft-suppress CERT_CPP-MEM56-a "This unique pointer is not intended to free memory"
        {
            WC_UNUSED_DATA(ptr); // Do NOT close
        });

        return read_internal(*read_fd_ptr, get_file_size_internal(m_temp_file_path, true), offset, length);
    }
}

// Gets file descriptor for temporary file with so far written data (e.g. for internal validation)
file_descriptor_ptr linux_file::get_write_file_fd()
{
    if(m_readonly)
    {
        throw std::logic_error("Tried to get write file descriptor of a readonly file");
    }

    return m_open_file;
}

// Stores the file data to the final file destination
void linux_file::store()
{
    if(m_readonly)
    {
        throw std::logic_error("Tried to store written data for a readonly file");
    }

    if(m_no_empty_file_on_disk && (m_file_capacity == 0))
    {
        // Unlink existing file (if any)
        if(sal::filemanagement::get_instance().unlink(m_file_path) != 0)
        {
            if(errno != ENOENT)
            {
                std::string error_message = std::string("Failed to remove file \"") + m_file_path + "\": "
                                          + errno_to_string(errno);
                wc_log(log_level_t::error, error_message.c_str());
            }
        }
        else
        {
            wc_log(log_level_t::info, (std::string("Removed file \"") + m_file_path + "\"").c_str());
        }
    }
    else
    {
        // Destroy external file descriptor pointer to ensure no further external access
        if(!m_open_file.unique())
        {
            throw std::logic_error("Outstanding open file descriptor");
        }

        // Store written data to final file destination
        if(sal::filemanagement::get_instance().fsync(*m_open_file) < 0)
        {
            std::string error_message = std::string("Failed to synchronize written file data: ")
                                      + errno_to_string(errno);
            throw std::runtime_error(error_message);
        }
        int const close_result = sal::filemanagement::get_instance().close(*m_open_file);
        m_open_file.reset(); // The close operation invalidates the file descriptor, even when returned with an error
        if(close_result < 0)
        {
            std::string error_message = std::string("Failed to close written file, data may be lost: ")
                                      + errno_to_string(errno);
            throw std::runtime_error(error_message);
        }
        if(sal::filemanagement::get_instance().rename(m_temp_file_path, m_file_path) < 0)
        {
            std::string error_message = std::string("Failed to move written file to final destination: ")
                                      + errno_to_string(errno);
            throw std::runtime_error(error_message);
        }
    }

    // Move or remove succeeded. From now on we consider the operation succeeded
    m_temp_file_path = "";
    m_keep_file_open = false;
    m_readonly       = true;

    // Synchronize directory to ensure updated file entry
    file_descriptor dir_fd = sal::filemanagement::get_instance().open_readonly(m_directory_path);
    bool const dir_fd_is_valid = (dir_fd >= 0);
    if(    (!dir_fd_is_valid)
        || (sal::filemanagement::get_instance().fsync(dir_fd) < 0))
    {
        std::string error_message = "Failed to synchronize directory \"" + m_directory_path + "\" of written file: "
                                  + errno_to_string(errno);
        wc_log(log_level_t::warning, error_message.c_str());
    }
    if(    (dir_fd_is_valid) 
        && (sal::filemanagement::get_instance().close(dir_fd) < 0))
    {
        std::string error_message = "Failed to close directory \"" + m_directory_path + "\" after sync of written file: "
                                  + errno_to_string(errno);
        wc_log(log_level_t::warning, error_message.c_str());
    }
}

//------------------------------------------------------------------------------
// Internal function implementation
//------------------------------------------------------------------------------
namespace {
void free_c_string(char *str)
{
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc)
    free(reinterpret_cast<void *>(str));
}

void change_file_owner(file_descriptor const fd,
                       uid_t           const user,
                       gid_t           const group)
{
    if(sal::filemanagement::get_instance().fchown(fd, user, group) != 0)
    {
        std::string error_message = std::string("Failed to change file owner for temporary write file: ")
                                  + errno_to_string(errno);
        throw std::runtime_error(error_message);
    }
}

uint64_t get_file_size_internal(std::string const &file_path,
                                bool        const  allow_missing_file)
{
    uint64_t file_size;
    if(sal::filemanagement::get_instance().get_file_size(file_path, file_size) != 0)
    {
        if(allow_missing_file && (errno == ENOENT))
        {
            return 0;
        }
        else
        {
            std::string error_message = std::string("Failed to determine file stats for \"") + file_path + "\": "
                                      + errno_to_string(errno);
            throw std::runtime_error(error_message);
        }
    }
    return file_size;
}

std::vector<uint8_t> read_internal(file_descriptor const fd,
                                   uint64_t        const file_size,
                                   uint64_t        const offset,
                                   size_t          const length)
{
    if(    ((offset + length) < offset)     // overflow
        || ((offset + length) < length)     // overflow
        || ((offset + length) > file_size))
    {
        throw std::out_of_range("Result of offset + length is out of file range");
    }

    // Implementation is limited to size_t
    if((offset + length) > SIZE_MAX)
    {
        throw std::out_of_range("Result of offset + length is out of " + std::to_string(sizeof(size_t) * 8) + " bit range");
    }
    WC_ASSERT(offset < SIZE_MAX);
    auto const internal_offset = static_cast<size_t>(offset);

    std::vector<uint8_t> result(length);
    result.resize(length);
    if(length != 0)
    {
        // TODO: Check if mmap gives a better performance
        if(sal::filemanagement::get_instance().seek_to_offset(fd, internal_offset) < 0)
        {
            std::string error_message = std::string("Failed to seek file for read operation: ")
                                      + errno_to_string(errno);
            throw std::runtime_error(error_message);
        }
        ssize_t const read_data = sal::filemanagement::get_instance().read(fd, result);
        if(read_data < 0)
        {
            std::string error_message = std::string("Failed to read data from file: ")
                                      + errno_to_string(errno);
            throw std::runtime_error(error_message);
        }
        WC_STATIC_ASSERT(SSIZE_MAX <= SIZE_MAX);
        if(static_cast<size_t>(read_data) < length)
        {
            result.resize(static_cast<size_t>(read_data));
        }
    }

    if(result.size() < length)
    {
        wc_log(log_level_t::info, "Read was interrupted before all requested data was read, return less data");
    }

    return result;
}
}


} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


//---- End of source file ------------------------------------------------------
