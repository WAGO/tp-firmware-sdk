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
///  \brief    Model repository loader for WAGO parameter service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSSERV_MODEL_LOADER_HPP_
#define SRC_LIBWDXLINUXOSSERV_MODEL_LOADER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
//#include <wago/wdx/wda/model_provider_i.hpp>
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

class model_loader final //: public wda::model_provider_i
{

private:
    std::string const model_path_m;

public:
    explicit model_loader(std::string const &model_path);
    ~model_loader() noexcept ;//override;

    future<wdm_response> get_model_information() ;//override;
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


#endif // SRC_LIBWDXLINUXOSSERV_MODEL_LOADER_HPP_
//---- End of source file ------------------------------------------------------
