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
#include "wago/wdx/linuxos/file/regular_file_provider.hpp"
#include "chunk_manager.hpp"
#include "file_factory.hpp"
#include "system_abstraction_file.hpp"
#include "errno_utils.hpp"

#include <wc/assertion.h>
#include <wc/log.h>

#include <memory>
#include <mutex>
#include <istream>
#include <streambuf>
#include <stdexcept>
#include <cstring>

#include <sys/stat.h>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {

enum class file_state { prepared, created, completed, readonly };

namespace {
constexpr mode_t const g_auto_file_mode = 9999;
}

//------------------------------------------------------------------------------
// Internal class definitions and function prototypes
//------------------------------------------------------------------------------
class file_provider_impl
{
public:
    file_provider_impl(std::string const &file_path,
                       bool               no_empty_file_on_disk,
                       bool               keep_file_open);
    file_provider_impl(std::string const &file_path,
                       uint64_t           file_size_limit);
    file_provider_impl(std::string const &file_path,
                       uint64_t           file_size_limit,
                       bool               no_empty_file_on_disk,
                       mode_t      const  file_mode,
                       std::string const &file_owner);
    ~file_provider_impl() noexcept = default;
    file_provider_impl(file_provider_impl &);
    file_provider_impl(file_provider_impl &&) noexcept;
    file_provider_impl & operator=(file_provider_impl const &);
    file_provider_impl & operator=(file_provider_impl       &&) noexcept;

    // Internal state
    std::mutex                           m_file_state_mutex;
    file_state                           m_state;

    // Member for write access
    std::string                    const m_file_path;
    uint64_t                       const m_file_size_limit;
    bool                           const m_no_empty_file_on_disk;
    mode_t                         const m_file_mode;
    std::string                    const m_file_owner;

    // File abstraction
    std::unique_ptr<file_i>              m_file;
    std::unique_ptr<chunk_manager>       m_chunks;
};

bool is_complete_unsafe(file_provider_impl *impl);


class fd_input_stream : public std::basic_istream<uint8_t>
{
private:
    class fd_buffer : public std::basic_streambuf<uint8_t>
    {
    private:
        std::vector<uint8_t> buffer;
        size_t               data_length;
        file_descriptor_ptr  fd;

    public:
        explicit fd_buffer(file_descriptor_ptr open_read_fd);
        fd_buffer::int_type underflow() override;
    };

public:
    explicit fd_input_stream(file_descriptor_ptr open_read_fd);

private:
    fd_buffer buffer;
};

//------------------------------------------------------------------------------
// Class implementation
//------------------------------------------------------------------------------
regular_file_provider::regular_file_provider(std::string const &readonly_file_path,
                                             bool               no_empty_file_on_disk,
                                             bool               keep_file_open)
: impl(std::make_unique<file_provider_impl>(readonly_file_path, no_empty_file_on_disk, keep_file_open))
{ }

regular_file_provider::regular_file_provider(std::string const &replaced_file_path,
                                             uint64_t           file_size_limit)
: impl(std::make_unique<file_provider_impl>(replaced_file_path, file_size_limit))
{ }

regular_file_provider::regular_file_provider(std::string const &new_file_path,
                                             uint64_t           file_size_limit,
                                             bool               no_empty_file_on_disk,
                                             mode_t      const  file_mode,
                                             std::string const &file_owner)
: impl(std::make_unique<file_provider_impl>(new_file_path, file_size_limit, no_empty_file_on_disk, file_mode, file_owner))
{ }

regular_file_provider::~regular_file_provider() noexcept = default;

regular_file_provider::regular_file_provider(regular_file_provider &&src) noexcept = default;

regular_file_provider & regular_file_provider::operator=(regular_file_provider &&src) noexcept = default;

std::string const &regular_file_provider::get_file_path()
{
    return impl->m_file_path;
}

future<wdx::response> regular_file_provider::create(uint64_t const capacity)
{
    if(impl->m_state == file_state::readonly)
    {
        return resolved_future(wdx::response(wdx::status_codes::logic_error));
    }

    if(capacity > impl->m_file_size_limit)
    {
        return resolved_future(wdx::response(wdx::status_codes::file_size_exceeded));
    }

    try
    {
        std::lock_guard<std::mutex> file_state_lock(impl->m_file_state_mutex);

        if(impl->m_file_mode == g_auto_file_mode)
        {
            mode_t mode;
            uid_t  uid;
            gid_t  gid;
            if(sal::filemanagement::get_instance().get_file_permissions(impl->m_file_path, mode, uid, gid) != 0)
            {
                std::string error_message = std::string("Failed to determine file permissions for \"") + impl->m_file_path + "\": "
                                          + errno_to_string(errno);
                throw std::runtime_error(error_message);
            }
            WC_DEBUG_LOG(("Determined mode " + std::to_string(mode) + " for new writable file (\"" + impl->m_file_path + "\")").c_str());
            impl->m_file = sal::file_factory::create_for_write_access(impl->m_file_path, impl->m_no_empty_file_on_disk, capacity, mode, uid, gid);
        }
        else
        {
            impl->m_file = sal::file_factory::create_for_write_access(impl->m_file_path, impl->m_no_empty_file_on_disk, capacity, impl->m_file_mode, impl->m_file_owner);
        }
        WC_ASSERT(capacity <= SIZE_MAX); // Implementation of linux_file checked 32 bit border (SIZE_MAX)
        impl->m_chunks = std::make_unique<chunk_manager>(static_cast<size_t>(capacity));
        impl->m_state = capacity == 0 ? file_state::completed : file_state::created;
        WC_DEBUG_LOG(("Created new writable file for \"" + impl->m_file_path + "\"").c_str());
        return resolved_future(wdx::response(wdx::status_codes::success));
    }
    catch(std::exception &e)
    {
        wc_log(log_level_t::error, ("Failed to create new writable file for \"" + impl->m_file_path + "\": " + e.what()).c_str());
        return resolved_future(wdx::response(wdx::status_codes::file_not_accessible));
    }
}

future<wdx::response> regular_file_provider::write(uint64_t             offset,
                                                   std::vector<uint8_t> data)
{
    try
    {
        std::lock_guard<std::mutex> file_state_lock(impl->m_file_state_mutex);

        if(impl->m_state == file_state::readonly)
        {
            throw std::logic_error("File is readonly");
        }
        if(impl->m_state < file_state::created)
        {
            throw std::logic_error("No file created previously");
        }
        WC_ASSERT(impl->m_file.get() != nullptr);
        if(impl->m_file.get() == nullptr)
        {
            // This case should not happen, because m_state has been checked already
            // but it is better to be safe than sorry 
            throw std::logic_error("No file created previously");
        }

        impl->m_file->write(offset, data);
        WC_ASSERT((offset + data.size()) <= SIZE_MAX); // Implementation of linux_file checked 32 bit border (SIZE_MAX)
        impl->m_chunks->add_chunk(static_cast<size_t>(offset), data.size());
        WC_DEBUG_LOG(("Successfully written " + std::to_string(data.size()) + " bytes of data for \"" + impl->m_file_path + "\"").c_str());
        return resolved_future(wdx::response(wdx::status_codes::success));
    }
    catch(std::out_of_range &e)
    {
        wc_log(log_level_t::error, ("Failed write operation: File size of \"" + impl->m_file_path + "\" would be exceeded").c_str());
        return resolved_future(wdx::response(wdx::status_codes::file_size_exceeded));
    }
    catch(std::logic_error &e)
    {
        wc_log(log_level_t::error, ("Failed write operation: Logic error for \"" + impl->m_file_path + "\": " + e.what()).c_str());
        return resolved_future(wdx::response(wdx::status_codes::logic_error));
    }
    catch(std::exception &e)
    {
        wc_log(log_level_t::error, ("Failed write operation: Temporary data file for \"" + impl->m_file_path + "\" not accessible: " + e.what()).c_str());
        return resolved_future(wdx::response(wdx::status_codes::file_not_accessible));
    }
}

future<wdx::file_info_response> regular_file_provider::get_file_info()
{
    try
    {
        std::lock_guard<std::mutex> file_state_lock(impl->m_file_state_mutex);

        uint64_t     file_size;
        char const * log_extension = "";
        // only for write file which no create call has happened for, yet
        // therefore we can just deliver '0' as the current file size
        if(impl->m_file.get() == nullptr)
        {
            file_size = 0;
        }
        else if(impl->m_state != file_state::readonly)
        {
            log_extension = " (temporary file)";
            file_size = impl->m_file->get_write_file_size();
        }
        else
        {
            file_size = impl->m_file->get_file_size();
        }
        WC_DEBUG_LOG(("Determined file size of " + std::to_string(file_size) + " for \"" + impl->m_file_path + "\"" + log_extension).c_str());
        return resolved_future(wdx::file_info_response(file_size));
    }
    catch(std::exception &e)
    {
        wc_log(log_level_t::error, ("File \"" + impl->m_file_path + "\" not accessible: " + e.what()).c_str());
        return resolved_future(wdx::file_info_response(wdx::status_codes::file_not_accessible));
    }
}

future<wdx::file_read_response> regular_file_provider::read(uint64_t offset,
                                                            size_t   length)
{
    try
    {
        std::lock_guard<std::mutex> file_state_lock(impl->m_file_state_mutex);

        if(impl->m_state < file_state::created)
        {
            throw std::logic_error("No file created previously");
        }
        WC_ASSERT(impl->m_file.get() != nullptr);
        if(impl->m_file.get() == nullptr)
        {
            // This case should not happen, because m_state has been checked already
            // but it is better to be safe than sorry 
            throw std::logic_error("No file created previously");
        }

        if(impl->m_state != file_state::readonly)
        {
            auto result = impl->m_file->read_write_file(offset, length);
            WC_DEBUG_LOG(("Successfully read back some temporary data for \"" + impl->m_file_path + "\"").c_str());
            return resolved_future(wdx::file_read_response(result));
        }
        else
        {
            auto result = impl->m_file->read(offset, length);
            WC_DEBUG_LOG(("Successfully read some data for \"" + impl->m_file_path + "\"").c_str());
            return resolved_future(wdx::file_read_response(result));
        }
    }
    catch(std::out_of_range &e)
    {
        wc_log(log_level_t::error, ("Failed read operation: File size of \"" + impl->m_file_path + "\" would be exceeded").c_str());
        return resolved_future(wdx::file_read_response(wdx::status_codes::file_size_exceeded));
    }
    catch(std::logic_error &e)
    {
        wc_log(log_level_t::error, ("Failed read operation: Logic error for \"" + impl->m_file_path + "\": " + e.what()).c_str());
        return resolved_future(wdx::file_read_response(wdx::status_codes::logic_error));
    }
    catch(std::exception &e)
    {
        wc_log(log_level_t::error, ("Failed read operation: Temporary data file for \"" + impl->m_file_path + "\" not accessible: " + e.what()).c_str());
        return resolved_future(wdx::file_read_response(wdx::status_codes::file_not_accessible));
    }
}

bool is_complete_unsafe(file_provider_impl *impl)
{
    if(impl->m_state < file_state::created)
    {
        throw std::logic_error("No file created previously");
    }

    if(impl->m_state >= file_state::completed)
    {
        return true;
    }
    else
    {
        WC_DEBUG_LOG(("Data for \"" + impl->m_file_path + "\" is complete").c_str());
        if (impl->m_chunks->file_completed())
        {
            impl->m_state = file_state::completed;
            return true;
        }
        else
        {
            return false;
        }
    }
}

future<bool> regular_file_provider::is_complete() const
{
    promise<bool> complete_promise;
    try
    {
        std::lock_guard<std::mutex> file_state_lock(impl->m_file_state_mutex);
        complete_promise.set_value(is_complete_unsafe(impl.get()));
    }
    catch(std::exception const &)
    {
        complete_promise.set_exception(std::current_exception());
    }

    return complete_promise.get_future();
}

future<void> regular_file_provider::validate(file_validator validator) const
{
    promise<void> validate_promise;
    try
    {
        std::lock_guard<std::mutex> file_state_lock(impl->m_file_state_mutex);

        if(impl->m_state < file_state::created)
        {
            throw std::logic_error("No file created previously");
        }
        else if(!is_complete_unsafe(impl.get()))
        {
            throw std::logic_error("File incomplete");
        }
        else if(impl->m_state == file_state::readonly)
        {
            WC_DEBUG_LOG(("Skip validation of readonly file for \"" + impl->m_file_path + "\"").c_str());
            validate_promise.set_value();
        }
        else if(impl->m_no_empty_file_on_disk && (impl->m_chunks->get_file_capacity() == 0))
        {
            WC_DEBUG_LOG(("Skip validation of zero data for \"" + impl->m_file_path + "\" (no empty file on disk)").c_str());
            validate_promise.set_value();
        }
        else
        {
            fd_input_stream fd_stream(impl->m_file->get_write_file_fd());
            if(!validator(fd_stream))
            {
                throw std::runtime_error("Validation failed");
            }
            else
            {
                WC_DEBUG_LOG(("Data for \"" + impl->m_file_path + "\" successfully validated").c_str());
                validate_promise.set_value();
            }
        }
    }
    catch(std::exception const &)
    {
        validate_promise.set_exception(std::current_exception());
    }

    return validate_promise.get_future();
}

future<void> regular_file_provider::finish()
{
    promise<void> finish_promise;
    try
    {
        std::lock_guard<std::mutex> file_state_lock(impl->m_file_state_mutex);

        if(impl->m_state == file_state::readonly)
        {
            throw std::logic_error("File is readonly");
        }
        if(impl->m_state < file_state::created)
        {
            throw std::logic_error("No file created previously");
        }
        WC_ASSERT(impl->m_file.get() != nullptr);

        impl->m_file->store();
        impl->m_state = file_state::readonly;
        WC_DEBUG_LOG(("Temporary data for \"" + impl->m_file_path + "\" successfully move to final destination").c_str());
        finish_promise.set_value();
    }
    catch(std::exception const &)
    {
        finish_promise.set_exception(std::current_exception());
    }

    return finish_promise.get_future();
}

//------------------------------------------------------------------------------
// Internal class and function implementation
//------------------------------------------------------------------------------
file_provider_impl::file_provider_impl(std::string const &file_path,
                                       bool               no_empty_file_on_disk,
                                       bool               keep_file_open)
: m_state(file_state::readonly)
, m_file_path(file_path)
, m_file_size_limit(0)
, m_no_empty_file_on_disk(no_empty_file_on_disk)
, m_file_mode(0000)
, m_file(sal::file_factory::create_for_read_access(m_file_path, m_no_empty_file_on_disk, keep_file_open))
{ }

file_provider_impl::file_provider_impl(std::string const &file_path,
                                       uint64_t           file_size_limit)
// File mode g_auto_file_mode is used as marker to determine file mode from existing file
: file_provider_impl(file_path, file_size_limit, false, g_auto_file_mode, "")
{ }

file_provider_impl::file_provider_impl(std::string const &file_path,
                                       uint64_t           file_size_limit,
                                       bool               no_empty_file_on_disk,
                                       mode_t      const  file_mode,
                                       std::string const &file_owner)
: m_state(file_state::prepared)
, m_file_path(file_path)
, m_file_size_limit(file_size_limit)
, m_no_empty_file_on_disk(no_empty_file_on_disk)
, m_file_mode(file_mode)
, m_file_owner(file_owner)
{ }

fd_input_stream::fd_buffer::fd_buffer(file_descriptor_ptr open_read_fd)
: data_length(0)
, fd(open_read_fd)
{
    buffer.resize(4096);
    setg(buffer.data(), buffer.data(), buffer.data() + data_length);
    if(sal::filemanagement::get_instance().seek_to_offset(*fd, 0) < 0)
    {
        std::string error_message = std::string("Failed to seek file for read operation: ")
                                  + errno_to_string(errno);
        WC_DEBUG_LOG(error_message.c_str());
        throw std::runtime_error(error_message);
    }
}

fd_input_stream::fd_buffer::int_type fd_input_stream::fd_buffer::underflow()
{
    // Read chunk from file descriptor
    ssize_t const read_data = sal::filemanagement::get_instance().read(*fd, buffer);
    if(read_data < 0)
    {
        setg(buffer.data(), buffer.data(), buffer.data());
        std::string error_message = std::string("Failed to read data from file: ")
                                  + errno_to_string(errno);
        WC_DEBUG_LOG(error_message.c_str());
        throw std::runtime_error(error_message);
    }

    // Set results
    WC_ASSERT(read_data >= 0);
    WC_STATIC_ASSERT(sizeof(read_data) <= sizeof(data_length));
    data_length = static_cast<size_t>(read_data);
    WC_DEBUG_LOG(("Read " + std::to_string(read_data) + " new data from file descriptor").c_str());
    setg(buffer.data(), buffer.data(), buffer.data() + data_length);
    if(read_data == 0)
    {
        return traits_type::eof();
    }
    else
    {
        return traits_type::to_int_type(*gptr());
    }
}

fd_input_stream::fd_input_stream(file_descriptor_ptr open_read_fd)
: std::basic_istream<uint8_t>(&buffer)
, buffer(open_read_fd)
{
    rdbuf(&buffer);
}


} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


//---- End of source file ------------------------------------------------------
