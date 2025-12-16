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
#ifndef SRC_LIBWDXCORE_WDMM_DEVICE_MODEL_HPP_
#define SRC_LIBWDXCORE_WDMM_DEVICE_MODEL_HPP_

#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include "wago/wdx/wdmm/parameter_definition.hpp"
#include "wago/wdx/wdmm/class_definition.hpp"
#include "wago/wdx/wdmm/feature_definition.hpp"
#include "wago/wdx/wdmm/enum_definition.hpp"


namespace wago {
namespace wdx {
inline namespace wdmm {

struct model_version
{
    uint16_t major;
    uint16_t minor;
    uint16_t bugfix;
};

class device_model
{
public:
    static void check_model_version(std::string const version);
    static model_version const                       implemented_wdmm;

    std::string                                      name;
    std::vector<std::shared_ptr<feature_definition>> features;
    std::vector<std::shared_ptr<class_definition>>   classes;
    std::vector<std::shared_ptr<enum_definition>>    enums;
    
    /// Collects all parameter_definitions, regardless where they are contained in, for easy access later
    std::vector<std::shared_ptr<parameter_definition>> definitions; // these will be filled by DeviceModelLoader. NOTE: They do not contain overrides

    ~device_model() noexcept;

    std::shared_ptr<parameter_definition> find_definition(parameter_id_t const &definition_id) const;
    std::shared_ptr<enum_definition>      find_enum_definition(name_t const &enum_name) const;
    std::shared_ptr<class_definition>     find_class_definition(name_t const &name) const;
    std::shared_ptr<feature_definition>   find_feature_definition(name_t const &name) const;

    static bool names_equal(name_t const &n1, name_t const &n2);

    // TODO: https://en.cppreference.com/w/cpp/language/rule_of_three
};

}
}
}
#endif // SRC_LIBWDXCORE_WDMM_DEVICE_MODEL_HPP_
