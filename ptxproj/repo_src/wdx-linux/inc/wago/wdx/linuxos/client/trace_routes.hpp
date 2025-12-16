//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Trace routes for WAGO Parameter Service clients.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_CLIENT_TRACE_ROUTES_HPP_
#define INC_WAGO_WDX_LINUXOS_CLIENT_TRACE_ROUTES_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/trace.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace client {

/// Supported trace routes by client
enum class trace_route
{
    none           = wc_trace_channels::invalid,       //!< Invalid marker for no active route
    lifetime_mgmt  = wc_trace_channels::channel_1,     //!< Client init & destroy
    provider_calls = wc_trace_channels::channel_2,     //!< All incoming provider calls
    all            = (lifetime_mgmt|provider_calls)   //!< Marker to set all supported routes active
};

/// Global array with all distinct, supported trace routes by client
constexpr trace_route all_trace_routes[] = { trace_route::lifetime_mgmt,
                                             trace_route::provider_calls };

//------------------------------------------------------------------------------
/// Helper function to convert a client trace route into a trace channel.
///
/// \param route
///   Client route to convert into a trace channel.
///
/// \return
///   Trace channel for client route.
//------------------------------------------------------------------------------
inline wc_trace_channels for_route(trace_route route)
{
    return static_cast<wc_trace_channels>(route); // parasoft-suppress CERT_CPP-INT50-a-3 "Trace routes explicitly build on trace channels in definition"
}

} // Namespace client
} // Namespace linuxos
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
inline wago::wdx::linuxos::client::trace_route operator+(wago::wdx::linuxos::client::trace_route const &lhs, wago::wdx::linuxos::client::trace_route const &rhs)
{
    return static_cast<wago::wdx::linuxos::client::trace_route>(for_route(lhs) + for_route(rhs)); // parasoft-suppress CERT_CPP-INT50-a-3 "Trace routes explicitly build on trace channels in definition"
}

#endif // INC_WAGO_WDX_LINUXOS_CLIENT_TRACE_ROUTES_HPP_
//---- End of source file ------------------------------------------------------

