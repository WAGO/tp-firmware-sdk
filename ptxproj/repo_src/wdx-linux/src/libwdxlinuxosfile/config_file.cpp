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
///  \brief    Implementation of config file abstraction class.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/file/config_file.hpp"
#include "wago/wdx/linuxos/file/exception.hpp"
#include "linux_file.hpp"

#include <wc/assertion.h>
#include <wc/log.h>

#include <stdexcept>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {

//------------------------------------------------------------------------------
// Class implementation
//------------------------------------------------------------------------------
size_t const config_file::max_size;

config_file::config_file(std::string const &readonly_file_path,
                         bool               no_empty_file_on_disk)
: m_file_path(readonly_file_path)
, m_no_empty_file_on_disk(no_empty_file_on_disk)
, m_file_size_limit(0)
, m_file_mode(0000)
{
    WC_DEBUG_LOG(("Created readonly config file instance for \"" + readonly_file_path + "\" "
                  "(no empty file on disk: " + (m_no_empty_file_on_disk ? "true" : "false") + ")").c_str());
}

config_file::config_file(std::string const &file_path,
                         bool               no_empty_file_on_disk,
                         mode_t             file_mode,
                         std::string const &file_owner)
: m_file_path(file_path)
, m_no_empty_file_on_disk(no_empty_file_on_disk)
, m_file_size_limit(config_file::max_size)
, m_file_mode(file_mode)
, m_file_owner(file_owner)
{
    WC_DEBUG_LOG(("Created writable config file instance for \"" + file_path + "\" "
                  "(no empty file on disk: " + (m_no_empty_file_on_disk ? "true" : "false") + ")").c_str());
}

config_file::~config_file() noexcept = default;

std::vector<uint8_t> config_file::load()
{
    try
    {
        std::lock_guard<std::mutex> config_lock(m_config_mutex);

        auto           file      = std::make_unique<linux_file>(m_file_path, m_no_empty_file_on_disk, false);
        uint64_t const file_size = file->get_file_size();
        WC_ASSERT(file_size <= config_file::max_size);
        WC_STATIC_ASSERT(sizeof(config_file::max_size) <= sizeof(size_t));

        return file->read(0, static_cast<size_t>(file_size));
    }
    catch(std::runtime_error const &e)
    {
        throw wdx::linuxos::file::exception(wdx::status_codes::file_not_accessible, e.what());
    }
    catch(...)
    {
        std::string const error_message = "Caught unexpected exception or value on load operation";
        wc_log(log_level_t::error, error_message.c_str());
        WC_FAIL(error_message.c_str());
        throw wdx::linuxos::file::exception(wdx::status_codes::internal_error, error_message);
    }
}

void config_file::store(const std::vector<uint8_t> &data)
{
    try
    {
        if(data.size() > config_file::max_size)
        {
            throw std::out_of_range("Data (" + std::to_string(data.size()          ) + " bytes) exceeded max file size ("
                                             + std::to_string(config_file::max_size) + " bytes)");
        }

        std::lock_guard<std::mutex> config_lock(m_config_mutex);

        auto file = std::make_unique<linux_file>(m_file_path, m_no_empty_file_on_disk, data.size(), m_file_mode, m_file_owner);
        file->write(0, data);
        file->store();
    }
    catch(std::out_of_range const &e)
    {
        throw wdx::linuxos::file::exception(wdx::status_codes::file_size_exceeded, e.what());
    }
    catch(std::logic_error const &e)
    {
        throw wdx::linuxos::file::exception(wdx::status_codes::logic_error, e.what());
    }
    catch(std::runtime_error const &e)
    {
        throw wdx::linuxos::file::exception(wdx::status_codes::file_not_accessible, e.what());
    }
    catch(...)
    {
        std::string const error_message = "Caught unexpected exception or value on store operation";
        wc_log(log_level_t::error, error_message.c_str());
        WC_FAIL(error_message.c_str());
        throw wdx::linuxos::file::exception(wdx::status_codes::internal_error, error_message);
    }
}


} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


//---- End of source file ------------------------------------------------------
