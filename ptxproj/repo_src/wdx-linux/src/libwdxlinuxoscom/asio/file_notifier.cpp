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
///  \brief    Implementation of the file notifier.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_notifier.hpp"
#include "common/definitions.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"

#include <wc/assertion.h>
#include <wc/compiler.h>
#include <wc/structuring.h>
#include <wc/log.h>

#include <sys/inotify.h>

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
file_notifier::file_notifier(boost::asio::io_context &context,
                             type                     file_type,
                             std::string              file_path)
: context_m(context)
, file_type_m(file_type)
, file_path_m(std::move(file_path))
, file_dir_m(file_path_m.substr(0, file_path_m.find_last_of('/')))
, file_name_m(file_path_m.substr(file_path_m.find_last_of('/') + 1))
, inotify_fd_m(sal::inotify::get_instance().init1(IN_NONBLOCK))
, watch_handle_m(-1)
, stream_m(context_m)
, buffer_m({})
{
    if(inotify_fd_m < 0)
    {
        throw wdx::linuxos::com::exception("Failed to setup file notifier: Init failed");
    }
    if(file_dir_m.empty())
    {
        file_dir_m = "/";
    }
}

file_notifier::~file_notifier() noexcept
{
    if(inotify_fd_m >= 0)
    {
        try
        {
            std::lock_guard<std::mutex> lock(mutex_m);
            if(watch_handle_m >= 0)
            {
                sal::inotify::get_instance().rm_watch(inotify_fd_m, watch_handle_m);
            }
            sal::inotify::get_instance().close(inotify_fd_m);
        }
        catch(...)
        {
            // Catch all exceptions to avoid throwing from destructor.
            // - std::lock_guard throws when recursivly locking the same mutex, which
            //   is most certainly not the case.
            // - close does not throw
            WC_FAIL("Unexpected exception caught in file_notifier destructor");
        }
    }
}

void file_notifier::async_wait_for_file(available_handler handler)
{
    // Store handler
    WC_ASSERT(handler != nullptr);
    {
        std::lock_guard<std::mutex> lock(mutex_m);
        handler_m = std::move(handler);
    }

    // Prepare watch setup
    available_handler setup_watch = [this](bool const success){
        if(!success)
        {
            // Job is failed, notifier may no longer valid after handler call
            handler_m(false);
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_m);
            uint32_t const watch_events = IN_CREATE | IN_MOVED_TO | IN_DELETE_SELF | IN_MOVE_SELF;
            watch_handle_m              = sal::inotify::get_instance().add_watch(inotify_fd_m, file_dir_m.c_str(), watch_events);
            if(watch_handle_m < 0)
            {
                throw wdx::linuxos::com::exception("Failed to setup file notifier: Failed to add event watch");
            }
        }

        // Check for race-condition: s.o. may have added the file already
        if(is_file_existing())
        {
            // Job is done, notifier may no longer valid after handler call
            handler_m(true);
        }
        else
        {
            char   * buffer_pos;
            size_t   buffer_size;
            {
                std::lock_guard<std::mutex> lock(mutex_m);
                buffer_pos  = buffer_m.data();
                buffer_size = buffer_m.size();
            }
            stream_m.assign(inotify_fd_m);
            read_inotify_watch(buffer_pos, buffer_size);
        }
    };

    // Check parent, needed to register watch events
    if(linuxos::sal::filesystem::get_instance().is_directory_existing(file_dir_m))
    {
        setup_watch(true);
    }
    else
    {
        if(file_dir_m == "/")
        {
            // Stop recursion on failure for root directory
            throw wdx::linuxos::com::exception("Failed to setup file notifier: Unable to get any directory for watches");
        }
        // Parent needed for watch is also not present, setup a notifier for it
        {
            std::lock_guard<std::mutex> lock(mutex_m);
            nested_notifier_m = std::make_unique<file_notifier>(context_m, type::directory, file_dir_m);
        }
        nested_notifier_m->async_wait_for_file(setup_watch);
    }
}

bool file_notifier::is_file_existing()
{
    switch(file_type_m)
    {
        case type::directory:
            return linuxos::sal::filesystem::get_instance().is_directory_existing(file_path_m);
            break;

        case type::socket:
            return linuxos::sal::filesystem::get_instance().is_socket_existing(file_path_m);
            break;

        default:
            WC_FAIL("Unhandled switch case for some file type");
    }

    return false;
}

void file_notifier::read_inotify_watch(char   *buffer_pos,
                                       size_t  buffer_size)
{
    auto asio_buffer = boost::asio::buffer(buffer_pos, buffer_size);
    stream_m.async_read_some(asio_buffer, [this] (boost::system::error_code const code, size_t const length) {
        if(code)
        {
            wc_log(log_level_t::error, (SERVICE_LOG_PREFIX "Failed to read inotify events: " + code.message()).c_str());
            // Job is done, notifier may no longer valid after handler call
            handler_m(false);
            return;
        }

        try
        {
            WC_DEBUG_LOG(("Got event data: " + std::to_string(length) + " bytes").c_str());
            auto remaining_length = length;
            constexpr auto event_size_without_len = offsetof(inotify_event, name);

            inotify_event * event;
            {
                std::lock_guard<std::mutex> lock(mutex_m);
                GNUC_DIAGNOSTIC_PUSH
                GNUC_DIAGNOSTIC_IGNORE("-Wcast-align")
                event = reinterpret_cast<inotify_event *>(buffer_m.data());
                GNUC_DIAGNOSTIC_POP
            }
            while(event_size_without_len <= remaining_length)
            {
                WC_DEBUG_LOG(("Event meta data (" + std::to_string(event_size_without_len) + " bytes) complete").c_str());
                auto event_size = event_size_without_len + event->len;
                WC_DEBUG_LOG(("Event size is " + std::to_string(event_size) + " bytes").c_str());

                // Event not complete
                if(remaining_length < event_size)
                {
                    break;
                }

                WC_DEBUG_LOG(("Event data is complete, process event \"" + std::string(WC_ARRAY_TO_PTR(event->name)) + "\"").c_str());
                if(process_event(event))
                {
                    // Job is done, notifier may no longer valid after handler call
                    handler_m(true);
                    return;
                }

                // Set pointer to next event
                GNUC_DIAGNOSTIC_PUSH
                GNUC_DIAGNOSTIC_IGNORE("-Wcast-align")
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                event = reinterpret_cast<inotify_event *>(reinterpret_cast<char *>(event) + event_size);
                GNUC_DIAGNOSTIC_POP
                remaining_length -= event_size;
                WC_DEBUG_LOG(("Remaining event data: " + std::to_string(remaining_length) + " bytes").c_str());
            }

            // Move remaining data to start of buffer
            char   * new_buffer_pos;
            size_t   new_buffer_size;
            {
                std::lock_guard<std::mutex> lock(mutex_m);
                ::memmove(buffer_m.data(), event, remaining_length);
                // Set read position to after remaining length
                new_buffer_pos  = buffer_m.data() + remaining_length;
                new_buffer_size = buffer_m.size() - remaining_length;
            }
            read_inotify_watch(new_buffer_pos, new_buffer_size);
        }
        catch(std::exception &e)
        {
            wc_log(log_level_t::error, (std::string(SERVICE_LOG_PREFIX "Watch for file failed: ") + e.what()).c_str());
            // Inform about failed watch
            handler_m(false);
        }
    });
}

inline bool is_directory_event(uint32_t const event_flags)
{
    return (event_flags & IN_ISDIR) != 0;
}

bool file_notifier::process_event(inotify_event const *event)
{
    bool result = false;

    uint32_t const event_type  = event->mask &  static_cast<uint32_t>(IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED);
    uint32_t const event_flags = event->mask & ~static_cast<uint32_t>(IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED);
    switch(event_type)
    {
        case IN_MOVED_TO:
        case IN_CREATE:
            if(    (event->len > 0)                               // Has a filename
                && (file_name_m == WC_ARRAY_TO_PTR(event->name))  // Filename matches
                && ((file_type_m == type::directory) ?            // Check file type
                          is_directory_event(event_flags) :
                        (!is_directory_event(event_flags) && (is_file_existing()))))
            {
                WC_DEBUG_LOG((SERVICE_LOG_PREFIX "Got event (matching requested file): " + std::to_string(event_type)).c_str());
                result = true;
            }
            else
            {
                WC_DEBUG_LOG((SERVICE_LOG_PREFIX "Got event (no match for requested file): " + std::to_string(event_type)).c_str());
            }
            break;

        case IN_DELETE_SELF:
        case IN_MOVE_SELF:
        case IN_IGNORED:
            WC_DEBUG_LOG((SERVICE_LOG_PREFIX "Got event making this watch obsolete: " + std::to_string(event_type)).c_str());
            // Watched directory is no longer the place where the expected file will pop up
            throw wdx::linuxos::com::exception("Watch for file \"" + file_path_m + "\" is no longer valid (event " + std::to_string(event_type) + ")");

        default:
            WC_DEBUG_LOG((SERVICE_LOG_PREFIX "Got uninteresting event: " + std::to_string(event_type)).c_str());
    }

    return result;
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
