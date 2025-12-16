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
///
///  \brief    Interface extensions for request_i.
///
///  \author   PEn : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/request_i.hpp"

#include <wc/log.h>

#include <cstring>


namespace wago {
namespace wdx {
namespace wda {
namespace http {
//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace {

bool match_type(std::vector<string> const &prototypes,
                string              const &candidate);

} // Namespace <anonymous>

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// extension implementation
//------------------------------------------------------------------------------
bool request_i::is_content_type_set() const
{
    return !get_content_type().empty();
}

bool request_i::is_response_content_type_accepted(std::string const &type,
                                                  std::string const &sub_type) const
{
    bool match = true;

    std::string const accepted_const = get_accepted_types();
    if(!accepted_const.empty())
    {
        WC_DEBUG_LOG(string("Accepted content types: \"") + accepted_const + "\"");
        match = false;

        // Create vector of valid prototypes
        std::vector<string> prototypes = { string(type) + "/"  + sub_type,
                                           string(type) + "/*",
                                                        string("*/") + sub_type,
                                                               "*/*"              };

        // Split strings on separator (,)
        std::vector<string> candidates;
        char accepted[2048];
        strncpy(accepted, accepted_const.c_str(), sizeof accepted - 1);
        accepted[sizeof accepted - 1] = '\0'; // terminate in case of string trancation
        char * accepted_part = accepted;
        char * accepted_part_next = accepted_part;
        while((accepted_part_next = strpbrk(accepted_part, ", ")) != nullptr) // parasoft-suppress CERT_C-EXP45-d-2 "Compliant solution - Intentional Assignment. Parasoft has a bug here."
        {
            accepted_part_next[0] = '\0';
            if((accepted_part_next - accepted_part) > 0)
            {
                WC_DEBUG_LOG(string("Found accepted content type candidate \"") + accepted_part + "\"");
                candidates.push_back(string(accepted_part));
            }
            accepted_part = accepted_part_next + 1;
        }
        if(accepted_part[0] != '\0')
        {
            WC_DEBUG_LOG(string("Found accepted content type candidate \"") + accepted_part + "\"");
            candidates.push_back(string(accepted_part));
        }

        // Check all candidates
        for(auto const &candidate: candidates)
        {
            if(match_type(prototypes, candidate))
            {
                WC_DEBUG_LOG(string("Found accepted content type \"") + candidate + "\"");
                match = true;
            }
        }
    }

    return match;
}

bool request_i::is_content_type_matching(std::string const &type,
                                         std::string const &sub_type) const
{
    bool match = false;

    std::string content_type = get_content_type();
    if(!content_type.empty())
    {
        WC_DEBUG_LOG("Content type: \"" + content_type + "\"");

        // Create vector of valid prototypes
        std::vector<string> prototypes = { type + "/"  + sub_type };

        // Check content type
        if(match_type(prototypes, content_type))
        {
            WC_DEBUG_LOG("Found content type match for \"" + content_type + "\"");
            match = true;
        }
    }

    return match;
}

namespace {

bool match_type(std::vector<string> const &prototypes,
                string              const &candidate)
{
    for(auto const &prototype: prototypes)
    {
        // Ignore extensions (after ;)
        char const * const prototype_cstring = prototype.c_str();
        char const * const candidate_cstring = candidate.c_str();
        size_t const prototype_length = prototype.length();
        if(    (strncmp(prototype_cstring, candidate_cstring, prototype_length) == 0)
            && ((candidate_cstring[prototype_length] == '\0') || (candidate_cstring[prototype_length] == ';')))
        {
            WC_DEBUG_LOG(string("Found type match for \"") + candidate + "\" with prototype \"" + prototype + "\"");
            return true;
        }
    }

    return false;
}

} // Namespace <anonymous>
} // Namespace http
} // Namespace wda
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
