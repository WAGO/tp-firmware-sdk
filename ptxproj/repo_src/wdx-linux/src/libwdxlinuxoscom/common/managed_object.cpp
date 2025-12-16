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
///  \brief    Implementation of common base for proxies and stubs.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "managed_object.hpp"
#include "manager.hpp"

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
// class implementation
//------------------------------------------------------------------------------
managed_object::managed_object(managed_object_id  id,
                               std::string const &connection_name,
                               sender_i          &sender,
                               driver_i          &driver)
: id_m(id)
, connection_name_m(connection_name)
, sender_m(sender)
, driver_m(driver)
{}

managed_object_id managed_object::get_id() const
{
    return id_m;
}

std::string const & managed_object::get_connection_name() const
{
    return connection_name_m;
}

sender_i & managed_object::get_sender() const
{
    return sender_m;
}

driver_i & managed_object::get_driver() const
{
    return driver_m;
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
