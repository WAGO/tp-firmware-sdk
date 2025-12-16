//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Weakly linked failure management to satisfy wc/assertion.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/exception.hpp"
#include "wago/wdx/linuxos/com/fail.hpp"

#include <wc/assertion.h>
#include <wc/compiler.h>
#include <wc/log.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using std::string;
using std::to_string;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-attribute=noreturn")
GNUC_ATTRIBUTE(weak) void wc_Fail(char const * const szReason, // parasoft-suppress CERT_CPP-DCL60-a-3 "Marked as weak: Maybe replaced"
                                  char const * const szFile,
                                  char const * const szFunction,
                                  int const line)
{
    string problem = szReason;
    problem += " [";
    problem += "from ";
    problem += szFile;
    problem += " in function ";
    problem += szFunction;
    problem += ", line ";
    problem += to_string(line);
    problem += "]";

    // Log problem
    wc_log(fatal, problem.c_str());

    // Throw to indicate problem
    throw wago::wdx::linuxos::com::exception(problem);
}
GNUC_DIAGNOSTIC_POP


//---- End of source file ------------------------------------------------------
