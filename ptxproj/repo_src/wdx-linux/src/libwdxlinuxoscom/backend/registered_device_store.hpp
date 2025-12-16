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
///  \brief    Store for registered devices.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_BACKEND_REGISTERED_DEVICE_STORE_HPP_
#define SRC_LIBWDXLINUXOSCOM_BACKEND_REGISTERED_DEVICE_STORE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "registered_device_store_i.hpp"

#include <wc/assertion.h>

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

class registered_device_store final : public registered_device_store_i
{
    // Move constructor & assignment are implicitly deleted because of reference members.
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(registered_device_store)

private:
    using store_object          = std::shared_ptr<device_meta>;
    using registered_device_map = std::map<wdx::device_id, store_object>;

    registered_device_map        devices_m;
    std::mutex                   container_mutex_m;
    std::string           const &connection_name_m;

public:
    explicit registered_device_store(std::string const &connection_name);
    ~registered_device_store() noexcept override;

    bool has_devices() override;
    bool has_device(wdx::device_id id) override;
    std::shared_ptr<device_meta> get_device_meta(wdx::device_id id) override;
    void add_device(wdx::register_device_request const &request) override;
    void remove_device(wdx::device_id id) override;
    void remove_devices(std::function<bool(device_meta const &meta)> predicate) override;
    std::string const & get_connection_name() const noexcept override;

private:
    bool has_object_unsafe(wdx::device_id id) const noexcept;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_BACKEND_REGISTERED_DEVICE_STORE_HPP_
//---- End of source file ------------------------------------------------------
