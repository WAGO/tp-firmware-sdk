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
///  \brief    Class to represent a generic resource relationship.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_BASIC_RELATIONSHIP_HPP_
#define SRC_LIBWDXWDA_REST_BASIC_RELATIONSHIP_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/compiler.h>

#include <string>
#include <map>
#include <tuple>
#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using std::string;
using std::map;
using std::vector;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
struct related_resource {
    string id;
    string type;

    related_resource(string id_, string type_)
    : id(id_)
    , type(type_)
    { }
};

class basic_relationship
{
public:
    enum class data_mode {
        multi,
        single,
        none
    };
private:
    map<string, string>      links_m;
    vector<related_resource> data_m;
    data_mode                mode_m;
public:
    basic_relationship(map<string, string>      const &links);
    basic_relationship(map<string, string>      const &links, 
                       vector<related_resource> const &data);
    basic_relationship(map<string, string>      const &links, 
                       related_resource         const &data);
    map<string, string>      const & get_links() const;
    data_mode                        get_data_mode() const;
    vector<related_resource> const & get_data() const;
};

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


#endif // SRC_LIBWDXWDA_REST_BASIC_RELATIONSHIP_HPP_
//---- End of source file ------------------------------------------------------
