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
///  \brief    Wrapper for monitoring list responses from the core API
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_MONITORING_LIST_DATA_HPP_
#define SRC_LIBWDXWDA_REST_MONITORING_LIST_DATA_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "basic_relationship.hpp"

#include "wago/wdx/responses.hpp"

#include <string>
#include <vector>
#include <map>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

class core_error;
template <class T>
class basic_resource;
using parameter_resource = basic_resource<wdx::parameter_response>;

using wdx::monitoring_list_id_t;
using std::string;
using std::vector;
using std::map;
using std::shared_ptr;

class monitoring_list_data {
private:
    monitoring_list_id_t id_m;
    uint16_t             timeout_m;
    bool                           has_included_parameters_m = false;
    vector<parameter_resource>     included_parameters_m;
    bool                           errors_in_resource_attributes_m;
public:
    monitoring_list_data(monitoring_list_id_t const id,
                         uint16_t             const timeout);

    monitoring_list_id_t get_monitoring_list_id() const;
    uint16_t             get_timeout() const;

    bool                     has_related_data(std::string const &relationship_name) const;
    vector<related_resource> get_related_data(std::string const &relationship_name) const;
    void                     set_related_data(std::string                     const &relationship_name, 
                                              vector<wdx::parameter_response>        included_parameters,
                                              bool                                   errors_in_resource_attributes);
    // TODO: set related errors separatly

    bool                           has_related_resources(std::string const &relationship_name) const;
    vector<parameter_resource>     get_related_resources(std::string const &relationship_name) const; // FIXME: Related parameter resources?
};


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_MONITORING_LIST_DATA_HPP_
//---- End of source file ------------------------------------------------------
