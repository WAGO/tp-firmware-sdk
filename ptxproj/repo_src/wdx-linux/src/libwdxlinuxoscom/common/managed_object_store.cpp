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
///  \brief    Implementation of the managed object store.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "managed_object_store.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"
#include "definitions.hpp"

#include <atomic>

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

managed_object_store::managed_object_store(std::string const &connection_name,
                                           sender_i          &sender,
                                           driver_i          &driver)
: connection_name_m(connection_name)
, sender_m(sender)
, driver_m(driver)
{ }

bool managed_object_store::has_generated_objects()
{
    auto non_backend_object = [] (auto &obj) {
        return obj.get_id() != backend_object_id;
    };
    return find(non_backend_object) != nullptr;
}

managed_object_id managed_object_store::generate_object_id()
{
    static std::atomic<managed_object_id> next_obj_id(generated_object_ids_start);
    return next_obj_id++;
}

bool managed_object_store::has_object(managed_object_id id)
{
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    return has_object_unsafe(id);
}

managed_object & managed_object_store::get_object(managed_object_id id)
{
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    if(!has_object_unsafe(id))
    {
        throw wdx::linuxos::com::exception("Cannot get object from store: unknown id");
    }
    return *objects_m.at(id).first.get();
}

std::shared_ptr<managed_object_store::object_meta> managed_object_store::get_object_meta(managed_object_id id)
{
    std::shared_ptr<managed_object_store::object_meta> result(nullptr);
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    if(has_object_unsafe(id))
    {
        result = objects_m.at(id).second;
    }
    return result;
}

managed_object & managed_object_store::add_object(std::unique_ptr<managed_object> &&obj)
{
    auto id = obj->get_id();
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    if(has_object_unsafe(id))
    {
        throw wdx::linuxos::com::exception("Cannot add object to store: duplicate id");
    }
    auto &obj_ref = *obj;
    objects_m.emplace(id, std::make_pair(std::move(obj), std::make_shared<object_meta>()));
    return obj_ref;
}

void managed_object_store::remove_object(managed_object_id id)
{
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    objects_m.erase(id);
}

void managed_object_store::remove_objects(std::function<bool(managed_object const &obj)> predicate)
{
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    for(auto it = objects_m.cbegin(); it != objects_m.cend();)
    {
        auto &element = *it;
        if(predicate(*element.second.first))
        {
            objects_m.erase(it++);
        }
        else 
        {
            ++it;
        }
    }
}

std::string const & managed_object_store::get_connection_name() const
{
    return connection_name_m;
}

bool managed_object_store::has_object_unsafe(managed_object_id id) const
{
    return objects_m.count(id) > 0;  // parasoft-suppress CERT_C-CON43-a-3 "Explicit unsafe method"
}

sender_i & managed_object_store::get_sender() const
{
    return sender_m;
}

driver_i & managed_object_store::get_driver() const
{
    return driver_m;
}

managed_object * managed_object_store::find(std::function<bool(managed_object const &)> predicate)
{
    std::lock_guard<std::mutex> container_guard(container_mutex_m);
    for(auto &element : objects_m)
    {
        if(predicate(*element.second.first))
        {
            return element.second.first.get();
        }
    }
    return nullptr;
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
