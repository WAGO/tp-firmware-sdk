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
///  \brief    Implementation of protected IPC adapter wrapper.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "protected_adapter.hpp"
#include "asio/asio_adapter.hpp"

#include <wc/assertion.h>

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

protected_adapter::protected_adapter(std::weak_ptr<asio_adapter> adapter,
                                     std::shared_ptr<std::mutex> mutex)
: adapter_m(std::move(adapter))
, mutex_m(std::move(mutex))
{ }

void protected_adapter::send(message_data &&message,
                             send_handler   handler)
{
    std::lock_guard<std::mutex> lock_guard(*mutex_m);
    auto adapter = adapter_m.lock();
    if((adapter.get() == nullptr) || adapter->is_closed())
    {
        handler("Disconnected");
    }
    else
    {
        adapter->send(std::move(message), handler);
    }
}

void protected_adapter::receive(receive_handler handler)
{
    auto adapter = adapter_m.lock();
    if((adapter.get() == nullptr) || adapter->is_closed()) // This case should not occur!
    {
        WC_ASSERT(adapter.get() != nullptr);
        WC_ASSERT(!adapter->is_closed()); // parasoft-suppress CERT_CPP-STR51-a-1 CERT_C-EXP34-a-1 "Assertion above already checks for nullptr"
        handler(message_data(), "Disconnected");
    }
    else
    {
        adapter->receive(handler);
    }
}

connection_info protected_adapter::get_connection_info() const
{
    auto adapter = adapter_m.lock();
    if(adapter.get() == nullptr)
    {
        return { "unknown_user" };
    }
    return adapter->get_connection_info();
}

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
