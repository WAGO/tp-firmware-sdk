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
///  \brief    The central IPC object manager.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_MANAGER_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_MANAGER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "managed_object.hpp"
#include "managed_object_store.hpp"
#include "sender_i.hpp"
#include "receiver_i.hpp"
#include "type_helpers.hpp"

#include <memory>
#include <map>
#include <type_traits>

#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

// Unit test implementation is friended in some cases
class manager_fixture;

namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class adapter_i;
class driver_i;

/// The manager is responsible to manage stubs and proxies which are using the 
/// same IPC connection.
class manager final : public sender_i, public receiver_i
{
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(manager)
private:
    std::string                                 connection_name_m;
    std::unique_ptr<adapter_i>                  adapter_m;
    driver_i                                   &driver_m;
    std::unique_ptr<managed_object_store_i>     object_store_m;

    friend manager_fixture;
    manager(std::string                             const &connection_name,
            std::unique_ptr<adapter_i>                     adapter,
            driver_i                                      &driver,
            std::unique_ptr<managed_object_store_i>        object_store);

public:
    /// Create a manager which manages connections and communication using 
    /// the provided adapter
    /// \param connection_name Give the connection a name of your choice
    /// \param adapter The adapter to be used, which must be an adapter_i
    manager(std::string                const &connection_name,
            std::unique_ptr<adapter_i>        adapter,
            driver_i                         &driver);

    manager(manager&&) noexcept;

    // Move assignment is implicitly deleted because of limitations in object store member.
    manager& operator=(manager&&) noexcept = delete;

    ~manager() noexcept override;

    managed_object_store_i &get_object_store();

    // sender_i
    void send(managed_object const  &sender,
              message_data   const &&message,
              send_handler           handler) override;

    // receiver_i
    void receive(message_data message) override;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_MANAGER_HPP_
//---- End of source file ------------------------------------------------------
