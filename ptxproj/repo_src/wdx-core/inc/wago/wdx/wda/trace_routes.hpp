//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Trace routes for WAGO Parameter Service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_WDA_TRACE_ROUTES_HPP_
#define INC_WAGO_WDX_WDA_TRACE_ROUTES_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/api.h"

#include <wc/trace.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

/// Supported trace routes by parameter service
enum class trace_route
{
    none              = wc_trace_channels::invalid,     //!< Invalid marker for no active route
    rest_api_call     = wc_trace_channels::channel_1,   //!< All incoming REST API calls
    file_api_call     = wc_trace_channels::channel_2,   //!< All incoming file API calls
    ipc_frontend_call = wc_trace_channels::channel_3,   //!< All incoming frontend calls
    ipc_file_api_call = wc_trace_channels::channel_4,   //!< All incoming file api calls
    ipc_backend_call  = wc_trace_channels::channel_5,   //!< All incoming backend calls
    all               = (rest_api_call|file_api_call|ipc_frontend_call|ipc_file_api_call|ipc_backend_call)   //!< Marker to set all supported routes active
};

/// Global array with all distinct, supported trace routes by parameter service
constexpr trace_route all_trace_routes[] = { trace_route::rest_api_call,
                                             trace_route::file_api_call,
                                             trace_route::ipc_frontend_call,
                                             trace_route::ipc_file_api_call,
                                             trace_route::ipc_backend_call };

//------------------------------------------------------------------------------
/// Helper function to convert a service trace route into a trace channel.
///
/// \param route
///   Service route to convert into a trace channel.
///
/// \return
///   Trace channel for service route.
//------------------------------------------------------------------------------
inline wc_trace_channels for_route(trace_route route)
{
    return static_cast<wc_trace_channels>(route); // parasoft-suppress CERT_CPP-INT50-a-3 "Trace routes explicitly build on trace channels in definition"
}

} // Namespace wda
} // Namespace wdx
} // Namespace wago

//------------------------------------------------------------------------------
/// Operator to combine multiple trace routes.
///
/// \param lhs
///   First set of trace routes to combine.
/// \param rhs
///   Second set of trace routes to combine.
///
/// \return
///   Combined set of trace routes.
//------------------------------------------------------------------------------
inline wago::wdx::wda::trace_route operator+(wago::wdx::wda::trace_route const &lhs, wago::wdx::wda::trace_route const &rhs)
{
    return static_cast<wago::wdx::wda::trace_route>(for_route(lhs) | for_route(rhs)); // parasoft-suppress CERT_CPP-INT50-a-3 "Trace routes explicitly build on trace channels in definition"
}

#endif // INC_WAGO_WDX_WDA_TRACE_ROUTES_HPP_
//---- End of source file ------------------------------------------------------

