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
///  \brief    Implementation of relationship class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "basic_relationship.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
basic_relationship::basic_relationship(map<string, string> const &links)
: links_m(links)
, mode_m(data_mode::none)
{ }

basic_relationship::basic_relationship(map<string, string>      const &links,
                                       vector<related_resource> const &data)
: links_m(links)
, data_m(data)
, mode_m(data_mode::multi)
{ }

basic_relationship::basic_relationship(map<string, string> const &links,
                                       related_resource    const &data)
: links_m(links)
, data_m({data})
, mode_m(data_mode::single)
{ }

map<string, string> const & basic_relationship::get_links() const
{
    return links_m;
}


basic_relationship::data_mode basic_relationship::get_data_mode() const
{
    return mode_m;
}

vector<related_resource> const & basic_relationship::get_data() const
{
    return data_m;
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
