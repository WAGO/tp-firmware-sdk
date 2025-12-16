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

#ifndef INC_WAGO_WDX_REQUESTS_HPP_
#define INC_WAGO_WDX_REQUESTS_HPP_

#include <memory>
#include <set>
#include "wago/wdx/device_id.hpp"
#include "wago/wdx/parameter_instance_id.hpp"
#include "wago/wdx/parameter_value.hpp"
#include "wago/wdx/parameter_instance_path.hpp"
#include "wago/wdx/parameter_selector.hpp"

namespace wago {
namespace wdx {

/**
This request informs the `ParameterService` that a device with the given `order_number` and `firmware_version` exists at the position described by the `device_id`.
 */
struct register_device_request
{
    wago::wdx::device_id device_id;
    std::string order_number;
    std::string firmware_version;
};

inline namespace file_transfer {
    class file_provider_i;
}

/**
This request bundles the information needed to register a file_provider_i.
 */
struct register_file_provider_request
{
    file_transfer::file_provider_i* provider;
    parameter_id_t context;
    register_file_provider_request(file_transfer::file_provider_i* provider_, parameter_id_t context_)
    :provider(provider_), context(context_)
    {}
};
/**
This request bundles the information needed to reregister a file_provider_i.
 */
struct reregister_file_provider_request
{
    file_transfer::file_provider_i* provider;
    file_id id;
    parameter_id_t context;
    reregister_file_provider_request(file_transfer::file_provider_i* provider_, file_id id_, parameter_id_t context_)
    :provider(provider_), id(id_), context(context_)
    {}
};

/**
This requests that the parameter identified by `param_id` should be set to `value`.
 */
struct value_request
{
    parameter_instance_id param_id;
    std::shared_ptr<parameter_value> value;

    value_request() = default;
    value_request(parameter_instance_id id, std::shared_ptr<parameter_value> v)
        : param_id(id), value(v)
    {
    }
};

/**
This requests that the parameter identified by `param_path` should be set to `value`.
 */
struct value_path_request
{
    parameter_instance_path param_path;
    std::shared_ptr<parameter_value> value;

    value_path_request() = default;
    value_path_request(parameter_instance_path path, std::shared_ptr<parameter_value> v)
        : param_path(path), value(v)
    {
    }
};

struct parameter_filter
{
    /** Do not use directly */
    device_selector _device = device_selector::any;
    /** Do not use directly */
    bool _without_usersettings = false;
    /** Do not use directly */
    bool _only_usersettings = false;
    /** Do not use directly */
    bool _without_writeable = false;
    /** Do not use directly */
    bool _only_writeable = false;
    /** Do not use directly */
    bool _without_methods = false;
    /** Do not use directly */
    bool _only_methods = false;
    /** Do not use directly */
    bool _without_file_ids = false;
    /** Do not use directly */
    bool _only_file_ids = false;
    /** Do not use directly */
    bool _without_beta = false;
    /** Do not use directly */
    bool _only_beta = false;
    /** Do not use directly */
    bool _without_deprecated = false;
    /** Do not use directly */
    bool _only_deprecated = false;
    /** Do not use directly */
    std::set<std::string> _only_features = {};
    /** Do not use directly */
    std::string _only_subpath = "";

    /** All parameters */
    static const parameter_filter any;

    /** Only give parameters for the devices fitting given `selector` */
    static parameter_filter only_device(device_selector const &selector);
    /** Exclude parameters that are user settings */
    static parameter_filter without_usersettings();
    /** Only give parameters that are user settings */
    static parameter_filter only_usersettings();
    /** Exclude parameters that are user settings */
    static parameter_filter without_writeable();
    /** Only give parameters that are user settings */
    static parameter_filter only_writeable();
    /** Exclude methods */
    static parameter_filter without_methods();
    /** Only give methods */
    static parameter_filter only_methods();
    /** Exclude parameters with type `file_id` */
    static parameter_filter without_file_ids();
    /** Only give parameters with type `file_id` */
    static parameter_filter only_file_ids();
    /** Exclude parameters belonging to a beta feature */
    static parameter_filter without_beta();
    /** Only give parameters belonging to a beta feature */
    static parameter_filter only_beta();
    /** Exclude parameters belonging to a deprecated feature */
    static parameter_filter without_deprecated();
    /** Only give parameters belonging to a deprecated feature */
    static parameter_filter only_deprecated();
    /** Only give parameters belonging to a specific feature */
    static parameter_filter only_feature(std::string const &feature_name);
    /** Only give parameters belonging to one of given features */
    static parameter_filter only_features(std::set<std::string> const &feature_names);
    /** Only give parameters under given subpath */
    static parameter_filter only_subpath(std::string const &path);
    
    parameter_filter() = default;
    parameter_filter(parameter_filter const &other);
    parameter_filter(device_selector  const &selector);
 
    /** Combine with given filter */
    parameter_filter operator|(parameter_filter const &other);
};

bool operator==(const parameter_filter& lhs, const parameter_filter& rhs) noexcept;
bool operator!=(const parameter_filter& lhs, const parameter_filter& rhs) noexcept;

}}

#endif // INC_WAGO_WDX_REQUESTS_HPP_
