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
//------------------------------------------------------------------------------

#ifndef INC_WAGO_WDX_PARAMETER_INSTANCE_PATH_HPP_
#define INC_WAGO_WDX_PARAMETER_INSTANCE_PATH_HPP_

#include <string>

namespace wago {
namespace wdx {

/// \copydetails parameter_instance_path::device_path
using device_path_t = std::string;

/// \copydetails parameter_instance_path::parameter_path
using parameter_instance_path_t = std::string;

/**
The human-readable alternative to `parameter_instance_id`, to address a specific parameter(-instance) in a specific device.
 */
struct parameter_instance_path
{
    
    /**
    The path of the ParameterInstance. Its segments are delimited by '/'.
    If the ParameterInstance is defined in a Class, the full path is {basePath of class}/{instanceId}/{path of ParameterDefinition}.
    Matching is case-insensitive.
     */
    parameter_instance_path_t parameter_path;

    /**
     * {collectionID}-{slot}, e.g. "2-3" for the third module of the second
     * device collection.
     * The headstation can be addressed with "0-0", or with an empty string.
     */
    device_path_t device_path;

    /**
     * Creates a `parameter_instance_path`, with device defaulting to headstation.
     * \param parameter \copydetails parameter_instance_path::parameter_path
     * \param device \copydetails parameter_instance_path::device_path
     */
    parameter_instance_path(parameter_instance_path_t parameter, device_path_t device = "");

    /** Creates an empty `parameter_instance_path`.
     */
    parameter_instance_path() = default;
};

bool operator==(const parameter_instance_path& lhs, const parameter_instance_path& rhs) noexcept;
bool operator!=(const parameter_instance_path& lhs, const parameter_instance_path& rhs) noexcept;

}
}
#endif // INC_WAGO_WDX_PARAMETER_INSTANCE_PATH_HPP_
