//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Utility functions grouping operations that create or modify or evaluate vectors.
///
///  \author   PEn : WAGO GmbH & Co. KG
///  \author   MaHe : WAGO GmbH & Co. KG
///  \author   FHa : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "utils/vector_operations.hpp"

#include <algorithm>
#include <sstream>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

bool is_subset(std::vector<std::string> const &subset,
               std::vector<std::string> const &superset)
{
    for(auto const &element : subset)
    {
        if(std::find(superset.begin(), superset.end(), element) == superset.end())
        {
            return false;
        }
    }
    return true;
}

std::vector<std::string> split_string(std::string const &elements,
                                      char        const  splitter)
{
    std::stringstream element_stream(elements);
    std::string element;
    std::vector<std::string> element_list;

    while(std::getline(element_stream, element, splitter))
    {
       element_list.push_back(element);
    }
    return element_list;
}

} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
