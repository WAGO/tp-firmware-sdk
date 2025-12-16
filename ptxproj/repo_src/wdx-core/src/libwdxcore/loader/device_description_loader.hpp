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
#ifndef SRC_LIBWDXCORE_LOADER_DEVICE_DESCRIPTION_LOADER_HPP_
#define SRC_LIBWDXCORE_LOADER_DEVICE_DESCRIPTION_LOADER_HPP_

#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include "wdmm/device_model.hpp"
#include "wdmm/device_description.hpp"

namespace wago {
namespace wdx {

class device;

class device_description_loader
{
public:
    device_description_loader(device_model &device_model_, device &device_);

    void load(std::string &wdd_content);
    void add_description(device_description const &extension_description);
    bool was_model_incomplete() const;
    void retry_with_updated_model();

private:
    device& m_device;
    device_model& m_device_model;

    device_description m_description;
    bool m_model_incomplete = false;

    void instantiate(parameter_definition_owner           &definition_owner,
                     instance_id_t                         instance_id,
                     std::vector<value_declaration> const &values,
                     bool                                  force_status_unavailable_if_not_provided = false);
    
    static void apply_overrides(parameter_definition_owner* definition_owner, bool warn = false);
    static bool apply_override(parameter_definition_override const& override, std::vector<std::shared_ptr<parameter_definition>>& definitions);

    void load_description(std::string &wdd_content);
    void process_description();

    bool parse_device_description(nlohmann::json &node);
    bool parse_device_description_values(nlohmann::json &node);
    bool parse_value_declaration(nlohmann::json &node, value_declaration &val_decl) const;
};

}
}
#endif // SRC_LIBWDXCORE_LOADER_DEVICE_DESCRIPTION_LOADER_HPP_
