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
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXCORE_INSTANCES_DEVICE_HPP_
#define SRC_LIBWDXCORE_INSTANCES_DEVICE_HPP_

#include <vector>
#include "wdmm/class_instantiation_internal.hpp"
#include "wdmm/device_model.hpp"
#include "wdmm/device_description.hpp"
#include "parameter_instance_collection.hpp"


namespace wago {
namespace wdx {

class device_description_loader;

class device
{
public:
    device_id id;
    
    /// Collection for value and attribute requests
    parameter_instance_collection parameter_instances;

    /// Collection for parameter model requests e. g. Frontend: GetFeatures, GetInstance, GetInstanceClasses.
    /// This will include all features the device has, for easy access to all parameters/classes etc.
    feature_definition collected_features;

    // TODO: Are this the static classes?
    std::vector<class_instantiation_internal> class_instantiations;
    bool has_beta_parameters       = false;
    bool has_deprecated_parameters = false;

    device(device_id const &id, std::string const &order_number_, std::string const &firmware_version);
    std::string get_order_number() const;
    std::string get_firmware_version() const;

    void add_description(device_description const &extension_description, device_model& device_model);
    void add_wdd(std::string wdd_content, device_model& device_model);
    void retry_unfinished_device_informations();

private:
    std::string m_order_number;
    std::string m_firmware_version;
    std::vector<std::unique_ptr<device_description_loader>> m_incomplete_loaders;
};

}
}
#endif // SRC_LIBWDXCORE_INSTANCES_DEVICE_HPP_
