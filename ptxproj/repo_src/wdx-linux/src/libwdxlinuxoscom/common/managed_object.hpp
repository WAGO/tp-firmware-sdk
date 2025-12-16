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
///  \brief    A common base for proxies and stubs.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_MANAGED_OBJECT_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_MANAGED_OBJECT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "data_stream.hpp"

#include <wc/std_includes.h>
#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

/// Used for identification of an object
using managed_object_id = uint32_t;
static constexpr managed_object_id unknown_managed_object_id = UINT32_MAX;

class sender_i;
class driver_i;

/// Managed object able to send/receive data.
class managed_object
{
private:
    managed_object_id  id_m;
    std::string const &connection_name_m;
    sender_i          &sender_m;
    driver_i          &driver_m;

public:
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(managed_object)

    /// Create a managed object with the provided id. 
    /// \param id   The id to uniquely identify the managed object
    /// \param mngr The manager which this object is managed by
    managed_object(managed_object_id  id,
                   std::string const &connection_name,
                   sender_i          &sender,
                   driver_i          &driver);

    managed_object(managed_object&&) noexcept = default;

    // Move assignment is implicitly deleted because of reference members.
    managed_object& operator=(managed_object&&) noexcept = delete;

    virtual ~managed_object() noexcept = default;

    /// Get the id of the managed object
    managed_object_id get_id() const;

    /// Get the connection name of the managed object
    std::string const & get_connection_name() const;

    /// Get sender interface
    sender_i & get_sender() const;

    /// Get driver interface
    driver_i & get_driver() const;

    /// This method will be responsible to handle an incoming
    /// message addressed to the id of this object.
    virtual void handle_message(data_istream &received) = 0;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_MANAGED_OBJECT_HPP_
//---- End of source file ------------------------------------------------------
