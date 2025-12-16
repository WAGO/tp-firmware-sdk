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
///  \brief    Interface for a store capable of registered devices.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_BACKEND_REGISTERED_DEVICE_STORE_I_HPP_
#define SRC_LIBWDXLINUXOSCOM_BACKEND_REGISTERED_DEVICE_STORE_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/requests.hpp>

#include <wc/structuring.h>

#include <string>
#include <functional>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

/// Device management interface to manage registered devices.
class registered_device_store_i
{
    WC_INTERFACE_CLASS(registered_device_store_i)

public:
    struct device_meta
    {
        wdx::device_id  id;
        std::string     order_number;
        std::string     firmware_version;

        explicit device_meta(wdx::register_device_request const &request) noexcept
        :id(request.device_id)
        ,order_number(request.order_number)
        ,firmware_version(request.firmware_version)
        {}
    };

public:
    virtual bool has_devices() = 0;
    virtual bool has_device(wdx::device_id id) = 0;
    virtual std::shared_ptr<device_meta> get_device_meta(wdx::device_id id) = 0;
    virtual void add_device(wdx::register_device_request const &request) = 0;
    virtual void remove_device(wdx::device_id id) = 0;
    virtual void remove_devices(std::function<bool(device_meta const &meta)> predicate) = 0;
    virtual std::string const & get_connection_name() const = 0;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_BACKEND_REGISTERED_DEVICE_STORE_I_HPP_
//---- End of source file ------------------------------------------------------
