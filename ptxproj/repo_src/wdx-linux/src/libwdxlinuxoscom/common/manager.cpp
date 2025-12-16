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
///  \brief    Implementation of the central IPC object manager.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "manager.hpp"
#include "coder.hpp"
#include "adapter_i.hpp"

#include <wc/assertion.h>
#include <wc/log.h>

#include <limits>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

namespace {
void atomic_decrement(managed_object_store_i::object_meta *meta)
{
    std::lock_guard<std::mutex> counter_lock(meta->object_mutex);
    if(meta->call_counter.fetch_sub(1) == 1)
    {
        meta->call_counter_condition.notify_all();
    }
}

using count_guard = std::unique_ptr<managed_object_store_i::object_meta, void(*)(managed_object_store_i::object_meta*)>;
count_guard create_count_guard(managed_object_store_i::object_meta *object_meta)
{
    object_meta->call_counter.fetch_add(1);
    // atomic_decrement function notifies on a condition variable,
    // if count guard reference drops to zero
    return count_guard(object_meta, atomic_decrement); // parasoft-suppress CERT_CPP-MEM56-a-1 "This unique pointer is not intended to free memory"
}
}

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
//NOLINTNEXTLINE(modernize-pass-by-value)
manager::manager(std::string                             const &connection_name,
                 std::unique_ptr<adapter_i>                     adapter,
                 driver_i                                      &driver,
                 std::unique_ptr<managed_object_store_i>        object_store)
: connection_name_m(connection_name)
, adapter_m(std::move(adapter))
, driver_m(driver)
, object_store_m(std::move(object_store))
{}


manager::manager(std::string                const &connection_name, // NOLINT(modernize-pass-by-value")
                 std::unique_ptr<adapter_i>        adapter,
                 driver_i                         &driver)
: manager(connection_name,
          std::move(adapter),
          driver, 
          std::make_unique<managed_object_store>(connection_name_m, *this, driver))
{}

manager::manager(manager&&) noexcept = default;

manager::~manager() noexcept = default;

managed_object_store_i &manager::get_object_store()
{
    return *object_store_m;
}

void manager::send(managed_object const  &sender,
                   message_data   const &&message,
                   send_handler           handler)
{
    WC_ASSERT(message.size() <= sender_i::max_send_data);
    message_data message_stream_buffer;
    data_stream message_stream(message_stream_buffer);
    coder::encode(message_stream, sender.get_id());
    message_stream.write(message.data(), static_cast<ssize_t>(message.size())); // FIXME: this causes a copy of the bytes
    adapter_m->send(std::move(message_stream_buffer), handler);
}

void manager::receive(message_data message)
{
    // Create input stream from message
    data_input_stream message_stream(message);

    managed_object_id obj_id;
    coder::decode(message_stream, obj_id);
    auto object_meta = object_store_m->get_object_meta(obj_id);
    if(object_meta.get() != nullptr)
    {
        bool return_negative_result = false;
        if(object_meta->marked_to_destroy)
        {
            return_negative_result = true;
        }
        else
        {
            auto object_guard = create_count_guard(object_meta.get());
            if(object_meta->marked_to_destroy) // Check twice to avoid to block a destroying thread!
            {
                return_negative_result = true;
            }
            else
            {
                auto &object = object_store_m->get_object(obj_id);
                object.handle_message(message_stream);
            }
        }

        // Handle negative result out of mutex/object_guard scope to avoid to block a destroying thread!
        if(return_negative_result)
        {
            wc_log(log_level_t::info, "Not handled a received message because object is marked for destruction");
            // TODO: Handle message by return negative result
            //       Do we need static negative handlers of the particular type therefore?
            //       How do we know of what type that object was (maybe from meta data struct)?
            //       Would it maybe enough to just have an IPC return code given back instead of an typed answer?
        }
    }
    else
    {
        wc_log(log_level_t::info, "Received message for unknown or cleaned object");
        // TODO: ignore messages for unknown objects?

        // Is this case similar to "return_negative_result"?
    }
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
