//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project daemon-utils.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Definition for default <wc/assertion> implementation.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_COM_FAIL_HPP_
#define INC_WAGO_WDX_LINUXOS_COM_FAIL_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <wc/assertion.h>

// export symbols for functions declared by wc/assertion
extern "C"
{
WDXLINUXOSCOM_API void wc_Fail(char const * const szReason,
                               char const * const szFile,
                               char const * const szFunction,
                               int const line);
}

#endif // INC_WAGO_WDX_LINUXOS_COM_FAIL_HPP_
//---- End of source file ------------------------------------------------------
