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
#include "wago/wdx/requests.hpp"
#include <algorithm>
#include <wdmm/device_model.hpp>

namespace wago {
namespace wdx {

parameter_filter::parameter_filter(device_selector const &selector) :_device(selector) {}

parameter_filter::parameter_filter(parameter_filter const &other) = default;

parameter_filter parameter_filter::operator|(parameter_filter const &other)
{
    parameter_filter p = *this;
    if(other._device != device_selector::any)
    {
        p._device = other._device;
    }
    if(other._without_usersettings)
    {
        p._without_usersettings = true;
    }
    if(other._only_usersettings)
    {
        p._only_usersettings = true;
    }
    if(other._without_writeable)
    {
        p._without_writeable = true;
    }
    if(other._only_writeable)
    {
        p._only_writeable = true;
    }
    if(other._without_methods)
    {
        p._without_methods = true;
    }
    if(other._only_methods)
    {
        p._only_methods = true;
    }
    if(other._without_file_ids)
    {
        p._without_file_ids = true;
    }
    if(other._only_file_ids)
    {
        p._only_file_ids = true;
    }
    if(other._without_beta)
    {
        p._without_beta = true;
    }
    if(other._only_beta)
    {
        p._only_beta = true;
    }
    if(other._without_deprecated)
    {
        p._without_deprecated = true;
    }
    if(other._only_deprecated)
    {
        p._only_deprecated = true;
    }
    if(!other._only_features.empty())
    {
        if(p._only_features.empty()) 
        {
            p._only_features = other._only_features;
        }
        else
        {
            std::set<std::string> common_features;
            // remove all features from p that are not in other as well
            for(auto &feature : p._only_features)
            {
                auto found_in_other = other._only_features.end() != std::find_if(
                    other._only_features.begin(), other._only_features.end(), 
                    [&feature](auto const &other_feature) {
                        return device_model::names_equal(feature, other_feature);
                    }
                );
                if(found_in_other)
                {
                    common_features.emplace(std::move(feature));
                }
            }
            p._only_features = common_features;

            constexpr char const no_feature_allowed_by_filter[] = "no_feature_allowed_by_filter";
            if (common_features.size() == 0)
            {
                // no common features found, so we have to insert one that never matches
                p._only_features.emplace(no_feature_allowed_by_filter);
            }
            else if (common_features.count(no_feature_allowed_by_filter) > 0)
            {
                // if the common features contain the "no_feature_allowed_by_filter" feature, we have to remove it
                // as it is not a valid feature name
                p._only_features.erase(no_feature_allowed_by_filter);
            }
        }
    }
    if(!other._only_subpath.empty())
    {
        p._only_subpath = other._only_subpath;
    }
    return p;
}

parameter_filter parameter_filter::only_device(device_selector const &selector)
{
    return parameter_filter(selector);
}

parameter_filter parameter_filter::without_usersettings()
{
    parameter_filter p;
    p._without_usersettings = true;
    return p;
}

parameter_filter parameter_filter::only_usersettings()
{
    parameter_filter p;
    p._only_usersettings = true;
    return p;
}

parameter_filter parameter_filter::without_writeable()
{
    parameter_filter p;
    p._without_writeable = true;
    return p;
}

parameter_filter parameter_filter::only_writeable()
{
    parameter_filter p;
    p._only_writeable = true;
    return p;
}

parameter_filter parameter_filter::without_methods()
{
    parameter_filter p;
    p._without_methods = true;
    return p;
}

parameter_filter parameter_filter::only_methods()
{
    parameter_filter p;
    p._only_methods = true;
    return p;
}

parameter_filter parameter_filter::without_file_ids()
{
    parameter_filter p;
    p._without_file_ids = true;
    return p;
}

parameter_filter parameter_filter::only_file_ids()
{
    parameter_filter p;
    p._only_file_ids = true;
    return p;
}

parameter_filter parameter_filter::without_beta()
{
    parameter_filter p;
    p._without_beta = true;
    return p;
}

parameter_filter parameter_filter::only_beta()
{
    parameter_filter p;
    p._only_beta = true;
    return p;
}

parameter_filter parameter_filter::without_deprecated()
{
    parameter_filter p;
    p._without_deprecated = true;
    return p;
}

parameter_filter parameter_filter::only_deprecated()
{
    parameter_filter p;
    p._only_deprecated = true;
    return p;
}

parameter_filter parameter_filter::only_feature(std::string const &name)
{
    parameter_filter p;
    p._only_features = { name };
    return p;
}

parameter_filter parameter_filter::only_features(std::set<std::string> const &names)
{
    parameter_filter p;
    p._only_features = names;
    return p;
}

parameter_filter parameter_filter::only_subpath(std::string const &path)
{
    parameter_filter p;
    p._only_subpath = path;
    return p;
}

const parameter_filter parameter_filter::any = parameter_filter(); // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "bad_alloc exception is not thrown on Linux, parameter_filter constructor/destructor is not known to throw"

bool operator==(const parameter_filter& lhs, const parameter_filter& rhs) noexcept {
  return    (lhs._device               == rhs._device)
         && (lhs._without_usersettings == rhs._without_usersettings)
         && (lhs._only_usersettings    == rhs._only_usersettings)
         && (lhs._without_writeable    == rhs._without_writeable)
         && (lhs._only_writeable       == rhs._only_writeable)
         && (lhs._without_methods      == rhs._without_methods)
         && (lhs._only_methods         == rhs._only_methods)
         && (lhs._without_file_ids     == rhs._without_file_ids)
         && (lhs._only_file_ids        == rhs._only_file_ids)
         && (lhs._without_beta         == rhs._without_beta)
         && (lhs._only_beta            == rhs._only_beta)
         && (lhs._without_deprecated   == rhs._without_deprecated)
         && (lhs._only_deprecated      == rhs._only_deprecated)
         && (lhs._only_features.size() == rhs._only_features.size())
         // no mismatching element pair => container elements are equal (https://en.cppreference.com/w/cpp/algorithm/mismatch)
         && (std::mismatch(lhs._only_features.begin(), lhs._only_features.end(),
                           rhs._only_features.begin(), rhs._only_features.end()).first == lhs._only_features.end())
         && (lhs._only_subpath         == rhs._only_subpath);
}

bool operator!=(const parameter_filter& lhs, const parameter_filter& rhs) noexcept {
  return !(lhs == rhs);
}

}}
