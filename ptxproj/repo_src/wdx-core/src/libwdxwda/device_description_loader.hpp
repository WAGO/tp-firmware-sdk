//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Device description repository loader for WAGO parameter service.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_DEVICE_DESCRIPTION_LOADER_HPP_
#define SRC_LIBWDXWDA_DEVICE_DESCRIPTION_LOADER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/parameter_service.hpp"
#include "device_description_provider_i.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

class device_description_loader final : public device_description_provider_i
{

private:
    device_selector               selected_devices_m;
    parameter_service::wdd_getter description_getter_m;

public:
    device_description_loader(device_selector               selected_devices,
                              parameter_service::wdd_getter description_getter)
    : selected_devices_m(selected_devices)
    , description_getter_m(description_getter)
    { }
    ~device_description_loader() noexcept override = default;

    inline device_selector_response get_provided_devices() override
    { 
        return device_selector_response({selected_devices_m});
    }
    inline future<wdd_response> get_device_information(std::string order_number,
                                                       std::string firmware_version) override
    {
        return resolved_future(wdd_response(
            description_getter_m(std::move(order_number), std::move(firmware_version))
        ));
    }
};

} // Namespace wda
} // Namespace wdx
} // Namespace wago

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


#endif // SRC_LIBWDXWDA_DEVICE_DESCRIPTION_LOADER_HPP_
//---- End of source file ------------------------------------------------------
