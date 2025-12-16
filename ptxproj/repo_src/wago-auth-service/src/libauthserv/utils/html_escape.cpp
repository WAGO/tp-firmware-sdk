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
///  \brief    Escape special html characters from strings.
///
///  \author   FHa : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include "html_escape.hpp"

namespace wago {
namespace authserv {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace {
void replaceAll(std::string &str, const std::string &find, const std::string &replace_by);
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
std::string html_escape(std::string text)
{
    replaceAll(text, "&",      "&amp;");
    replaceAll(text, "\"",     "&quot;");
    replaceAll(text, "'",      "&apos;");
    replaceAll(text, "<",      "&lt;");
    replaceAll(text, ">",      "&gt;");
    // manual linebreak by adding \n, \r\n or \n\r to the string
    replaceAll(text, "\\r\\n", "<br>");
    replaceAll(text, "\\n\\r", "<br>");
    replaceAll(text, "\\n",    "<br>");
    // breaking with a linebreak (and return) depending on system
    replaceAll(text, "\r\n",   "<br>");
    replaceAll(text, "\n\r",   "<br>");
    replaceAll(text, "\n",     "<br>");
    return text;
}

namespace {
void replaceAll(std::string &str, std::string const &find, std::string const &replace_by)
{
    size_t pos = 0;
    while ((pos = str.find(find, pos)) != std::string::npos) // parasoft-suppress CERT_C-EXP45-d-2 "Compliant solution - Intentional assignment. Parasoft detects a false-positive here."
    {
        str.replace(pos, find.length(), replace_by);
        pos += replace_by.length(); // skip inserted string in search
    }
}
}

} // Namespace authserv
} // Namespace wago
//---- End of source file ------------------------------------------------------

