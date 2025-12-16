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
///  \brief    Implementation of URL utils.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "url_utils.hpp"

#include <regex>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace http {

using std::string;
using std::regex;
using std::regex_replace;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

// See also for special characters to escape:
//     https://www.regular-expressions.info/refcharacters.html
//     https://stackoverflow.com/questions/40195412/c11-regex-search-for-exact-string-escape
static regex const special_chars_regex { // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Any unexcpected exception in the init code will be noticed in the correspondig unit-tests."
    R"([[\\{}()*+?.\^$|])", regex::optimize
};
static regex const url_variables_regex { // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Any unexcpected exception in the init code will be noticed in the correspondig unit-tests."
    string() + R"([)" + parameter_marker + R"(][^/]*[)" + parameter_marker + R"(])", regex::optimize
};
static regex const url_path_variables_regex { // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Any unexcpected exception in the init code will be noticed in the correspondig unit-tests."
    string() + R"([)" + parameter_marker_path + R"(][^/]*[)" + parameter_marker_path + R"(])", regex::optimize
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

string build_url_regex_pattern(string const &pattern_template,
                               bool          allow_trailing_slash,
                               bool          match_following)
{
    // Create URL match regex
    string const escaped_url_template   = regex_replace(pattern_template, special_chars_regex, R"(\$&)" );
    char const * const start            = R"(^)";
    char const * const var_matcher      = R"(([^/?]+))";
    char const * const var_path_matcher = R"(([^?]+))";
    char const * const end              = match_following ? R"(([/].*)?)" : allow_trailing_slash ? R"([/]?)" : "";
    string const match_regex_string =
        start
        + regex_replace(regex_replace(escaped_url_template, url_variables_regex,      var_matcher),
                                                            url_path_variables_regex, var_path_matcher)
        + end;
    return match_regex_string;
}


} // Namespace http
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
