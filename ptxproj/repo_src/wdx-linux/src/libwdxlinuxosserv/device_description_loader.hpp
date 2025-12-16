//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Device description repository loader for WAGO parameter service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSSERV_DEVICE_DESCRIPTION_LOADER_HPP_
#define SRC_LIBWDXLINUXOSSERV_DEVICE_DESCRIPTION_LOADER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
//#include <wago/wdx/wda/device_description_provider_i.hpp>
#include <wago/future.hpp>
#include <wago/wdx/responses.hpp>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {

using device_selection = std::vector<device_selector>;

class device_description_loader final //: public wda::device_description_provider_i
{

private:
    std::string      const repository_path_m;
    device_selection const selected_devices_m;

public:
    device_description_loader(std::string      const &repository_path,
                              device_selection const &selected_devices);
    ~device_description_loader() noexcept ;//override;

    device_selector_response get_provided_devices() ;//override;
    future<wdd_response> get_device_information(std::string order_number,
                                                std::string firmware_version) ;//override;
};


} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


#endif // SRC_LIBWDXLINUXOSSERV_DEVICE_DESCRIPTION_LOADER_HPP_
//---- End of source file ------------------------------------------------------
