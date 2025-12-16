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
///
///  \brief    Implementation of monitoring list data class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "monitoring_list_data.hpp"
#include "basic_relationship.hpp"
#include "basic_resource.hpp"
#include "relationship_error.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

monitoring_list_data::monitoring_list_data(monitoring_list_id_t const id,
                                           uint16_t             const timeout)
: id_m(id)
, timeout_m(timeout)
, has_included_parameters_m(false)
, errors_in_resource_attributes_m(false)
{ }

monitoring_list_id_t monitoring_list_data::get_monitoring_list_id() const
{
    return id_m;
}

uint16_t monitoring_list_data::get_timeout() const
{
    return timeout_m;
}

bool monitoring_list_data::has_related_data(std::string const &relationship_name) const
{
    if(relationship_name != "parameters")
    {
        return false;
    }

    return has_included_parameters_m;
}

vector<related_resource> monitoring_list_data::get_related_data(std::string const &relationship_name) const
{
    if(!has_related_data(relationship_name))
    { 
        throw std::runtime_error("no related data for \"" + relationship_name + "\"");
    }

    vector<related_resource> related;
    for(auto &parameter : included_parameters_m)
    {
        related.push_back(related_resource(parameter.get_id(), parameter.get_type()));
    }

    return related;
}

void monitoring_list_data::set_related_data(std::string                     const &relationship_name,
                                            vector<wdx::parameter_response>        included_parameters,
                                            bool                                   errors_in_resource_attributes)
{
    if(relationship_name != "parameters")
    {
        throw std::runtime_error("unknown relationship name \"" + relationship_name + "\"");
    }
    included_parameters_m.clear();
    for(wdx::parameter_response parameter : included_parameters)
    {
        included_parameters_m.emplace_back(parameter);
    }
    has_included_parameters_m = true;
    errors_in_resource_attributes_m = errors_in_resource_attributes;
}

bool monitoring_list_data::has_related_resources(std::string const &relationship_name) const
{
    if(relationship_name != "parameters")
    {
        return false;
    }
    return !included_parameters_m.empty();
}

vector<parameter_resource> monitoring_list_data::get_related_resources(std::string const &relationship_name) const
{
    if(!has_related_data(relationship_name))
    { 
        throw std::runtime_error("no related resources for \"" + relationship_name + "\"");
    }
    vector<parameter_resource> non_error_included_parameters;
    for(parameter_resource const & parameter : included_parameters_m)
    {
        if (!parameter.has_errors() || errors_in_resource_attributes_m)
        {
            non_error_included_parameters.push_back(parameter);
        }
    }
    return non_error_included_parameters;;
}


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
