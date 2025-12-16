//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Device extension loader for WAGO parameter service.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSSERV_DEVICE_EXTENSION_LOADER_HPP_
#define SRC_LIBWDXLINUXOSSERV_DEVICE_EXTENSION_LOADER_HPP_

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

class device_extension_loader final //: public wda::device_description_provider_i
{

private:
    std::string const extension_path_m;

public:
    device_extension_loader(std::string const &extension_path);
    ~device_extension_loader() noexcept ;//override;

    future<wdd_response> get_device_information() ;//override;
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


#endif // SRC_LIBWDXLINUXOSSERV_DEVICE_EXTENSION_LOADER_HPP_
//---- End of source file ------------------------------------------------------
