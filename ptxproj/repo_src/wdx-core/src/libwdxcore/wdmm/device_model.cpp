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
#include <algorithm>
#include <regex>
#include "device_model.hpp"
#include "wago/wdx/wdmm/method_definition.hpp"
#include "utils/string_util.hpp"
#include "wago/wdx/parameter_exception.hpp"
#include "wago/wdx/status_codes.hpp"

using namespace std;

namespace wago {
namespace wdx {
namespace wdmm {

std::regex    const version_regex("^(\\d+)\\.(\\d+)\\.(\\d+)$", std::regex::optimize); // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Does only throw if pattern is invalid, but pattern is constant"
model_version const device_model::implemented_wdmm = { 1, 3, 0 };


inline uint16_t extract_version_no(std::string const &version_no)
{
    unsigned long const raw_no = std::stoul(version_no);
    if(raw_no > UINT16_MAX)
    {
        throw std::out_of_range("Version number exceeds " + std::to_string(UINT16_MAX));
    }
    return static_cast<uint16_t>(raw_no);
}

void device_model::check_model_version(std::string const version)
{
    std::smatch version_match;
    if(!std::regex_match(version, version_match, version_regex))
    {
        throw parameter_exception(status_codes::wdmm_version_not_supported, "Invalid model version");
    }
    try
    {
        uint16_t const device_model_major  = extract_version_no(version_match[1]);
        uint16_t const device_model_minor  = extract_version_no(version_match[2]);
        uint16_t const device_model_bugfix = extract_version_no(version_match[3]);
        if((device_model_major           > device_model::implemented_wdmm.major)
            || ((device_model_major     == device_model::implemented_wdmm.major)
                && (device_model_minor   > device_model::implemented_wdmm.minor))
                /* Bugfix version does not matter, model has to be compatible */ )
        {
            throw parameter_exception(status_codes::wdmm_version_not_supported);
        }
    }
    catch(...)
    {
        throw parameter_exception(status_codes::wdmm_version_not_supported, "Error parsing model version");
    }
}

device_model::~device_model() noexcept
{}

shared_ptr<parameter_definition> device_model::find_definition(parameter_id_t const &definition_id) const
{
    const auto it = find_if(definitions.begin(), definitions.end(), [definition_id](auto& definition)
    {
        return definition->id == definition_id;
    });
    if (it != definitions.end())
    {
        return *it;
    }
    else
    {
        return nullptr;
    }
}

shared_ptr<enum_definition> device_model::find_enum_definition(name_t const &enum_name) const
{
    auto ed = std::find_if(enums.begin(), enums.end(), [&](shared_ptr<enum_definition> const &def) { return names_equal(def->name, enum_name); });
    if(ed != enums.end())
    {
        return *ed;
    }
    else
    {
        return nullptr;
    }
}

shared_ptr<class_definition> device_model::find_class_definition(name_t const &cls_name) const
{
    auto ed = std::find_if(classes.begin(), classes.end(), [&](shared_ptr<class_definition> const &def) { return names_equal(def->name, cls_name); });
    if(ed != classes.end())
    {
        return *ed;
    }
    else
    {
        return nullptr;
    }
}

shared_ptr<feature_definition> device_model::find_feature_definition(name_t const &f_name) const
{
    auto ed = std::find_if(features.begin(), features.end(), [&](shared_ptr<feature_definition> const &def) { return names_equal(def->name, f_name); });
    if(ed != features.end())
    {
        return *ed;
    }
    else
    {
        return nullptr;
    }
}

bool device_model::names_equal(name_t const &n1, name_t const &n2)
{
    return to_lower_copy(n1) == to_lower_copy(n2);
}

bool feature_definition::is_part_of(feature_definition const &upper)
{
    if(device_model::names_equal(name, upper.name)) { return true; }
    for(auto const &included_name : upper.resolved_includes)
    {
        if(device_model::names_equal(name, included_name)) { return true; }
    }
    return false;
}

bool class_definition::is_part_of(class_definition const &upper)
{
    if(device_model::names_equal(name, upper.name)) { return true; }
    for(auto const &included_name : upper.resolved_includes)
    {
        if(device_model::names_equal(name, included_name)) { return true; }
    }
    return false;
}

}
}
}
