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
///  \brief    Store for managed objects.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_MANAGED_OBJECT_STORE_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_MANAGED_OBJECT_STORE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "managed_object_store_i.hpp"

#include <wc/structuring.h>

#include <string>
#include <map>
#include <memory>
#include <mutex>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class driver_i;
class sender_i;

/// Object management store containing stubs and proxies.
class managed_object_store final : public managed_object_store_i
{
private:
    using store_object       = std::pair<std::unique_ptr<managed_object>, std::shared_ptr<object_meta>>;
    using managed_object_map = std::map<managed_object_id, store_object>;

    managed_object_map        objects_m;
    std::mutex                container_mutex_m;
    std::string        const &connection_name_m;
    sender_i                 &sender_m;
    driver_i                 &driver_m;

public:
    // Move constructor & assignment are implicitly deleted because of reference members.
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(managed_object_store)
    managed_object_store(std::string const &connection_name,
                         sender_i          &sender,
                         driver_i          &driver);
    ~managed_object_store() noexcept override = default;

    bool has_generated_objects() override;
    managed_object_id generate_object_id() override;
    bool has_object(managed_object_id id) override;
    managed_object & get_object(managed_object_id id) override;
    std::shared_ptr<object_meta> get_object_meta(managed_object_id id) override;
    managed_object & add_object(std::unique_ptr<managed_object> &&obj) override;
    void remove_object(managed_object_id id) override;
    void remove_objects(std::function<bool(managed_object const &obj)> predicate) override;
    std::string const & get_connection_name() const override;

private:
    bool has_object_unsafe(managed_object_id id) const;
    sender_i & get_sender() const override;
    driver_i & get_driver() const override;
    managed_object * find(std::function<bool(managed_object const &obj)> predicate) override;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_MANAGED_OBJECT_STORE_HPP_
//---- End of source file ------------------------------------------------------
