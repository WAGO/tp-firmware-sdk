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
#include "wago/wdx/status_codes.hpp"

namespace wago {
namespace wdx {

std::string to_string(status_codes const &status)
{
    return status_code_strings[static_cast<int>(status)];
}

status_codes from_string(std::string const &s)
{
    for(int i = 0; i <= static_cast<int>(status_codes::highest); i++)
    {
        if(status_code_strings[i] == s)
        {
            return static_cast<status_codes>(i);  //parasoft-suppress CERT_CPP-INT50-a-3 "Enum values of status codes are explicitly designed to be searched. Values are defined without gap and with an explicit end marker 'status_codes::highest'."
        }
    }
    return status_codes::no_error_yet;
}

const char* to_c_string(status_codes const &status)
{
    return status_code_strings[static_cast<int>(status)];
}

bool has_error(status_codes const &status, status_codes_context const &context)
{
    switch (context)
    {
    case status_codes_context::parameter_read:
        return status != status_codes::no_error_yet 
            && status != status_codes::success 
            && status != status_codes::status_value_unavailable;
    case status_codes_context::parameter_write:
        return status != status_codes::no_error_yet 
            && status != status_codes::success 
            && status != status_codes::wda_connection_changes_deferred
            && status != status_codes::success_but_value_adjusted; //  && status != status_codes::ignored was considered, but would need additions for the REST-API;
    case status_codes_context::general:
    default:
        return status != status_codes::no_error_yet 
            && status != status_codes::success;
    }
}

bool is_determined(status_codes const &status)
{
    return status != status_codes::no_error_yet;
}

bool is_success(status_codes const &status, status_codes_context const &context)
{
    return (status == status_codes::success) 
        || (status == status_codes::success_but_value_adjusted && context == status_codes_context::parameter_write);
}

}
}
