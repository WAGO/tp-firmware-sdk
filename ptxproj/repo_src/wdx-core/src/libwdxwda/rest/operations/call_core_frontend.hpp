//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Utility to call the core frontend.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_OPERATIONS_CALL_CORE_FRONTEND_HPP_
#define SRC_LIBWDXWDA_REST_OPERATIONS_CALL_CORE_FRONTEND_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/request.hpp"

#include "wago/wdx/wda/trace_routes.hpp"
#include "wago/wdx/parameter_instance_path.hpp"
#include "parameter_service_frontend_extended_i.hpp"

#include <wc/trace.h>
#include <utility>

/// Helper macro to call a method on the wdx frontend. 
/// \param core_frontend The frontend to call the method on.
/// \param core_frontend_method The method to call.
/// \param handler The handler to call with the future result.
/// \param ... Any arguments to pass to the method.
/// \note This function macro helps to get non-repeating, readable code, but 
///       maybe there will be better possibilities in C++17/C++20.
#define CALL_CORE_FRONTEND(core_frontend, core_frontend_method, handler, ...) \
    call_core_frontend_impl<decltype(core_frontend.core_frontend_method(__VA_ARGS__))::value_type>\
                           ("Core operation: "      WC_STR(core_frontend_method), \
                            "Core operation done: " WC_STR(core_frontend_method), \
                            sizeof("Core operation: ")      - 1 + sizeof(WC_STR(core_frontend_method)) - 1, \
                            sizeof("Core operation done: ") - 1 + sizeof(WC_STR(core_frontend_method)) - 1, \
                            core_frontend, \
                            &wdx::parameter_service_frontend_extended_i::core_frontend_method, \
                            handler, \
                            ##__VA_ARGS__);


namespace wago {
namespace wdx {
namespace wda {
namespace rest {

template<typename Ret, typename ... Args>
using core_frontend_extended_method = future<Ret>(wdx::parameter_service_frontend_extended_i::*)(Args...);

template<typename CoreResp,
         typename ... CoreArgs>
future<unique_ptr<response_i>> call_core_frontend_impl(char   const                                          *trace_point_start_name,
                                                       char   const                                          *trace_point_end_name,
                                                       size_t const                                           trace_point_start_length,
                                                       size_t const                                           trace_point_end_length,
                                                       wdx::parameter_service_frontend_extended_i            &core_frontend,
                                                       core_frontend_extended_method<CoreResp, CoreArgs...>   core_frontend_method,
                                                       std::function<unique_ptr<response_i>(CoreResp &&)>   &&handler,
                                                       CoreArgs ...                                           args)
{
    auto resp_promise = std::make_shared<promise<unique_ptr<response_i>>>();
    wc_trace_set_marker(for_route(trace_route::rest_api_call), trace_point_start_name, trace_point_start_length);
    auto pending_core_response = (core_frontend.*core_frontend_method)(args...);
    pending_core_response.set_notifier([resp_promise,
                                        trace_point_end_name,
                                        trace_point_end_length,
                                        handler=std::move(handler)]
                                       (auto && core_response) {
        wc_trace_set_marker(for_route(trace_route::rest_api_call), trace_point_end_name, trace_point_end_length);
        try
        {
            resp_promise->set_value(handler(std::forward<CoreResp>(core_response)));
        }
        catch(...)
        {
            resp_promise->set_exception(std::current_exception());
        }
    });
    pending_core_response.set_exception_notifier([resp_promise](auto e) {
        resp_promise->set_exception(e);
    });
    return resp_promise->get_future();
}

template<typename Ret, typename ... Args>
using core_frontend_method = future<Ret>(wdx::parameter_service_frontend_i::*)(Args...);

template<typename CoreResp,
         typename ... CoreArgs>
future<unique_ptr<response_i>> call_core_frontend_impl(char   const                                          *trace_point_start_name,
                                                       char   const                                          *trace_point_end_name,
                                                       size_t const                                           trace_point_start_length,
                                                       size_t const                                           trace_point_end_length,
                                                       wdx::parameter_service_frontend_extended_i            &core_frontend,
                                                       core_frontend_method<CoreResp, CoreArgs...>            core_frontend_method,
                                                       std::function<unique_ptr<response_i>(CoreResp &&)>   &&handler,
                                                       CoreArgs ...                                           args)
{
    return call_core_frontend_impl(trace_point_start_name, trace_point_end_name, trace_point_start_length, trace_point_end_length, core_frontend,
                                   static_cast<core_frontend_extended_method<CoreResp, CoreArgs...>>(core_frontend_method),
                                   std::move(handler),
                                   args...);
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_OPERATIONS_CALL_CORE_FRONTEND_HPP_
//---- End of source file ------------------------------------------------------
