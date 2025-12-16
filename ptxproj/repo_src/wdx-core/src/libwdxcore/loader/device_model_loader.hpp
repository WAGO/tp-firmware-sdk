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
#ifndef SRC_LIBWDXCORE_LOADER_DEVICE_MODEL_LOADER_HPP_
#define SRC_LIBWDXCORE_LOADER_DEVICE_MODEL_LOADER_HPP_

#include <memory>
#include <iostream>
#include "wdmm/device_model.hpp"
#include "wago/wdx/wdmm/method_definition.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace std;
using namespace wago::wdx::wdmm;

namespace wago {
namespace wdx {
namespace wdmm {

class device_model_loader
{
public:
    void load(string& wdm_artifact, device_model& model);

private:
    static method_argument_definition parse_method_argument_definition(json const &node); // used as callback therefore static
    std::shared_ptr<parameter_definition> create_parameter_definition(
        json                                const &node,
        std::shared_ptr<feature_definition> const &owner_feature,
        std::shared_ptr<class_definition>   const &owner_class);
    void fill_definition(std::shared_ptr<parameter_definition_owner> const &definition_owner,
                         json                                        const &node,
                         std::shared_ptr<feature_definition>         const &owner_feature,
                         std::shared_ptr<class_definition>           const &owner_class);
    std::shared_ptr<class_definition> create_class_definition(json const&node);
    std::shared_ptr<feature_definition> create_feature_definition(json const &node);
    std::shared_ptr<enum_definition> create_enum_definition(json const &node);
    bool parse_device_model(json const &node, device_model& model, vector<std::shared_ptr<parameter_definition>>& added_definitions);
};

}
}
}
#endif // SRC_LIBWDXCORE_LOADER_DEVICE_MODEL_LOADER_HPP_
