//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Model repository loader for WAGO parameter service.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_MODEL_LOADER_HPP_
#define SRC_LIBWDXWDA_MODEL_LOADER_HPP_

#include "wago/wdx/wda/parameter_service.hpp"
#include "model_provider_i.hpp"

#include <functional>

namespace wago {
namespace wdx {
namespace wda {

class model_loader final : public model_provider_i
{
private:
    parameter_service::wdm_getter model_getter_m;
public:
    model_loader(parameter_service::wdm_getter model_getter)
    : model_getter_m(model_getter)
    { }
    ~model_loader() noexcept override = default;
    future<wdm_response> get_model_information() override
    {
        return resolved_future(wdm_response(model_getter_m()));
    };
};


} // Namespace wda
} // Namespace wdx
} // Namespace wago

#endif // SRC_LIBWDXWDA_MODEL_LOADER_HPP_
