//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Helper class for file creation notifications.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_ASIO_FILE_NOTIFIER_HPP_
#define SRC_LIBWDXLINUXOSCOM_ASIO_FILE_NOTIFIER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction_com.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

#include <string>
#include <functional>
#include <memory>
#include <mutex>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
struct inotify_event;

namespace wago {
namespace wdx {
namespace linuxos {
namespace com {


class file_notifier final
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(file_notifier)

public:
    using available_handler = std::function<void(bool success)>;
    enum class type           { directory, socket };

private:
    using stream_descriptor = boost::asio::posix::stream_descriptor;
    using event_buffer      = std::array<char, 1024 * 16>;

    boost::asio::io_context        &context_m;
    std::mutex                      mutex_m;
    type                            file_type_m;
    std::string                     file_path_m;
    std::string                     file_dir_m;
    std::string                     file_name_m;
    sal::file_descriptor            inotify_fd_m;
    sal::watch_handle               watch_handle_m;
    stream_descriptor               stream_m;
    event_buffer                    buffer_m;
    available_handler               handler_m;
    std::unique_ptr<file_notifier>  nested_notifier_m;

public:
    file_notifier(boost::asio::io_context &context,
                  type                     file_type,
                  std::string              file_path);
    ~file_notifier() noexcept;

    void async_wait_for_file(available_handler handler);

private:
    bool is_file_existing();
    void read_inotify_watch(char   *buffer_pos,
                            size_t  buffer_size);
    bool process_event(inotify_event const *event);
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_ASIO_FILE_NOTIFIER_HPP_
//---- End of source file ------------------------------------------------------
