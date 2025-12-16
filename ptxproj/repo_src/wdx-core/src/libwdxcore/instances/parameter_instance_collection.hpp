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
#ifndef SRC_LIBWDXCORE_INSTANCES_PARAMETER_INSTANCE_COLLECTION_HPP_
#define SRC_LIBWDXCORE_INSTANCES_PARAMETER_INSTANCE_COLLECTION_HPP_

#include <vector>
#include "parameter_instance.hpp"
#include "wago/wdx/parameter_instance_id.hpp"
#include "wago/wdx/wdmm/parameter_definition.hpp"
#include "utils/path.hpp"
#include "utils/path_tree_node.hpp"
#include "wdmm/value_declaration.hpp"

namespace wago {
namespace wdx {

class parameter_instance_collection
{
public:

    parameter_instance_collection();

    ~parameter_instance_collection() noexcept;

    void add_instances(device_id                                                 device,
                       instance_id_t                                             instance_id,
                       std::vector<std::shared_ptr<parameter_definition>> const &definitions,
                       std::vector<value_declaration>                     const &values,
                       bool                                                      force_status_unavailable_if_not_provided = false);
    void add_class_instance(device_id                   device,
                            path                        base_path,
                            parameter_definition const &definition,
                            value_declaration    const &force_value_decl);

    parameter_instance* get_instance(parameter_instance_id id);

    parameter_instance* get_instance(path const &parameter_path);

    std::vector<parameter_instance*> const & get_all();
    std::vector<parameter_instance*> get_all_under_subpath(path subpath);

    static path build_parameter_instance_path(parameter_instance* instance);

private:
    std::vector<std::unique_ptr<parameter_instance>> m_parameter_instances;
    std::vector<parameter_instance*> m_parameter_instance_ptrs;
    path_tree_node<parameter_instance*> m_paths;
    std::vector<std::unique_ptr<parameter_instance>> m_dynamic_parameter_instances;
    path_tree_node<parameter_instance*> m_dynamic_paths;
    static path build_internal_path(parameter_instance* instance);

    parameter_instance* get_dynamic_instance(parameter_instance_id id, parameter_instance* placeholder_instance);
    parameter_instance* get_dynamic_instance(path const &parameter_path, parameter_instance* placeholder_instance);
};

bool find_value_declaration(const std::vector<value_declaration>& values, parameter_instance_id id, value_declaration& out);

}
}
#endif // SRC_LIBWDXCORE_INSTANCES_PARAMETER_INSTANCE_COLLECTION_HPP_
