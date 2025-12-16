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
///  \brief    Implementation of the registered device store.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "registered_device_store.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"

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
registered_device_store::registered_device_store(std::string const &connection_name)
: connection_name_m(connection_name)
{ }

registered_device_store::~registered_device_store() noexcept = default;

bool registered_device_store::has_devices()
{
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    return !devices_m.empty();
}

bool registered_device_store::has_device(wdx::device_id id)
{
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    return has_object_unsafe(id);
}

std::shared_ptr<registered_device_store::device_meta> registered_device_store::get_device_meta(wdx::device_id id)
{
    std::shared_ptr<registered_device_store::device_meta> result(nullptr);
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    if(has_object_unsafe(id))
    {
        result = devices_m.at(id);
    }
    return result;
}

void registered_device_store::add_device(wdx::register_device_request const &request)
{
    auto id = request.device_id;
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    if(has_object_unsafe(id))
    {
        throw wdx::linuxos::com::exception("Cannot add device to store: duplicate id");
    }
    devices_m.emplace(id, std::make_shared<device_meta>(request));
}

void registered_device_store::remove_device(wdx::device_id id)
{
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    devices_m.erase(id);
}

void registered_device_store::remove_devices(std::function<bool(device_meta const &meta)> predicate)
{
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    for(auto it = devices_m.cbegin(); it != devices_m.cend();)
    {
        auto &element = *it;
        if(predicate(*element.second))
        {
            devices_m.erase(it++);
        }
        else 
        {
            ++it;
        }
    }
}

std::string const & registered_device_store::get_connection_name() const noexcept
{
    return connection_name_m;
}

bool registered_device_store::has_object_unsafe(wdx::device_id id) const noexcept
{
    return devices_m.count(id) > 0; // parasoft-suppress CERT_C-CON43-a "Explicit unsafe declared function"
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
