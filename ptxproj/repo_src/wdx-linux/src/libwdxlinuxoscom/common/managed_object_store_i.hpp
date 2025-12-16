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
///  \brief    Interface for a store capable of managing managed objects.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_MANAGED_OBJECT_STORE_I_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_MANAGED_OBJECT_STORE_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "managed_object.hpp"

#include "abstract_proxy.hpp"
#include "abstract_stub.hpp"

#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <string>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class sender_i;
class driver_i;


/// Object management interface to manage stubs and proxies.
class managed_object_store_i
{
    WC_INTERFACE_CLASS(managed_object_store_i)

public:
    struct object_meta
    {
        std::mutex              object_mutex;
        std::condition_variable call_counter_condition;
        std::atomic<size_t>     call_counter;
        std::atomic<bool>       marked_to_destroy;

        object_meta() noexcept : call_counter(0), marked_to_destroy(false) {}
    };

public:
    virtual bool has_generated_objects() = 0;
    virtual managed_object_id generate_object_id() = 0;
    virtual bool has_object(managed_object_id id) = 0;
    virtual managed_object & get_object(managed_object_id id) = 0;
    virtual std::shared_ptr<object_meta> get_object_meta(managed_object_id id) = 0;
    virtual managed_object & add_object(std::unique_ptr<managed_object> &&obj) = 0;
    virtual void remove_object(managed_object_id id) = 0;
    virtual void remove_objects(std::function<bool(managed_object const &obj)> predicate) = 0;
    virtual std::string const & get_connection_name() const = 0;

private:
    virtual sender_i & get_sender() const = 0;
    virtual driver_i & get_driver() const = 0;
    virtual managed_object * find(std::function<bool(managed_object const &obj)> predicate) = 0;

// interface extensions
public:
    template<class Object = managed_object>
    Object & add_object(std::unique_ptr<Object> &&obj);

    template<class Object = managed_object>
    Object & get_object(managed_object_id id);

    template<class ProxyType,
             class Interface = typename ProxyType::interface,
             class ... FwdArgs,
             std::enable_if_t<std::is_base_of<abstract_proxy<Interface>, ProxyType>::value, int> = 0>
    ProxyType & create_proxy(managed_object_id const id, FwdArgs & ... args)
    {
        return add_object(std::make_unique<ProxyType>(id, get_connection_name(), get_sender(), get_driver(), args...));
    }

    template<class StubType,
             class Interface = typename StubType::interface,
             std::enable_if_t<std::is_base_of<abstract_stub<Interface>, StubType>::value, int> = 0>
    StubType & create_stub(managed_object_id const  id,
                           Interface               &target,
                           wc_trace_channels  const  channel = wc_trace_channels::invalid)
    {
        return add_object(std::make_unique<StubType>(id, get_connection_name(), get_sender(), get_driver(), target, channel));
    }

    template<class StubType,
             class Interface = typename StubType::interface,
             std::enable_if_t<std::is_base_of<abstract_stub<Interface>, StubType>::value, int> = 0>
    StubType & create_stub(managed_object_id          const  id,
                           std::unique_ptr<Interface>        target,
                           wc_trace_channels          const  channel = wc_trace_channels::invalid)
    {
        return add_object(std::make_unique<StubType>(id, get_connection_name(), get_sender(), get_driver(), std::move(target), channel));
    }

    template<class Interface,
             class StubType = abstract_stub<Interface>>
    StubType * find_stub(Interface &target);
};

template<class Interface,
         class StubType>
StubType * managed_object_store_i::find_stub(Interface &target)
{
    return static_cast<StubType *>(find([&target](managed_object const &obj) {
        auto const *object = dynamic_cast<StubType const*>(&obj);
        return ((object != nullptr) && (&object->target() == &target));
    }));
}

template<class Object>
Object & managed_object_store_i::add_object(std::unique_ptr<Object> &&obj)
{
    // add the object to the managed objects
    std::unique_ptr<managed_object> obj_base = std::move(obj);
    return static_cast<Object &>(add_object(std::move(obj_base)));
}

template<class Object>
Object & managed_object_store_i::get_object(managed_object_id id)
{
    return static_cast<Object &>(get_object(id));
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_MANAGED_OBJECT_STORE_I_HPP_
//---- End of source file ------------------------------------------------------
