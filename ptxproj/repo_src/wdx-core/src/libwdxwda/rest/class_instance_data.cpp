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
///  \brief    Implementation of class instance data class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "class_instance_data.hpp"
#include "http/http_exception.hpp"

#include "wda_ipc/representation.hpp"
#include <wc/assertion.h>
#include <wc/log.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

namespace {

wdx::parameter_instance_path class_path_from_instance_path(wdx::parameter_instance_path instance_path)
{
    return wdx::parameter_instance_path(
        instance_path.parameter_path.substr(0, instance_path.parameter_path.find_last_of('/')),
        instance_path.device_path
    );
}

wdx::instance_id_t instance_no_from_instance_path(wdx::parameter_instance_path instance_path)
{
    try 
    {
        auto const instance_no_str = instance_path.parameter_path.substr(instance_path.parameter_path.find_last_of('/') + 1);
        return wda_ipc::from_string<wdx::instance_id_t>(instance_no_str.c_str());
    }
    catch (std::exception const &e)
    {
        std::string message = "Malformed instance number in instance path.";
        wc_log(debug, message + ": " + e.what());
        throw http::http_exception(message, http::http_status_code::internal_server_error);
    }
}

}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
class_instance_data::class_instance_data(wdx::parameter_instance_path const &class_param_id, 
                                         wdx::instance_id_t                  instance_no)
: class_param_id_m(class_param_id)
, instance_no_m(instance_no)
{
    WC_ASSERT(instance_no > 0);
    if ((instance_no == 0) || (instance_no > (UINT16_MAX - 1)))
    {
        throw http::http_exception("Invalid instance number in instance path.", http::http_status_code::internal_server_error);
    }
}

class_instance_data::class_instance_data(wdx::parameter_instance_path const &instance_param_id)
: class_instance_data(class_path_from_instance_path(instance_param_id), 
                      instance_no_from_instance_path(instance_param_id))
{ }

wdx::parameter_instance_path class_instance_data::get_class_parameter() const
{
    return class_param_id_m;
}

std::string class_instance_data::get_path() const
{
    return class_param_id_m.parameter_path + "/" + wda_ipc::to_string(instance_no_m);
}

wdx::instance_id_t class_instance_data::get_instance_no() const
{
    return instance_no_m;
}

bool class_instance_data::is_dangling() const
{
    return instance_no_m == 0;
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
