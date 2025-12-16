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
///  \brief    Check for provider register responses.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_CLIENT_CHECK_REGISTER_HPP_
#define INC_WAGO_WDX_LINUXOS_CLIENT_CHECK_REGISTER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/responses.hpp>
#include <wago/future.hpp>

#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace client {

//------------------------------------------------------------------------------
/// Check a single provider register response.
///
/// \param response
///   Response to check.
///
/// \throws std::runtime_error if response result is not success.
//------------------------------------------------------------------------------
void check_register(wago::wdx::response const &response);

//------------------------------------------------------------------------------
/// Check a vector of provider register responses.
///
/// \param responses
///   Responses to check.
///
/// \throws std::runtime_error if any response result is not success.
//------------------------------------------------------------------------------
template<typename ResponseType>
inline void check_register(std::vector<ResponseType> const &responses)
{
    for(auto response : responses)
    {
        check_register(response);
    }
}

//------------------------------------------------------------------------------
/// Check a provider registration response asynchronously.
///
/// \param response
///   Async response to check packed in a future.
/// \param fail_handler
///   Fail handler called asynchronously if result is not success.
///   Fail handler must be noexcept.
//------------------------------------------------------------------------------
template<typename ResponseType>
inline void check_register(wago::future<ResponseType>                            &&pending_response,
                           std::function<void(std::string const &error_message)>   fail_handler) noexcept
{
    try {
        pending_response.set_notifier([fail_handler](ResponseType response){
            try
            {
                check_register(response);
            }
            catch(std::exception const &e)
            {
                fail_handler(e.what());
            }
        });
        pending_response.set_exception_notifier([fail_handler](auto e_ptr){
            try
            {
                if(e_ptr)
                {
                    std::rethrow_exception(e_ptr);
                }
            }
            catch(std::exception const &e)
            {
                fail_handler(e.what());
            }
        });
    }
    catch(std::exception const &e)
    {
        fail_handler(e.what());
    }
}

} // Namespace client
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago

#endif // INC_WAGO_WDX_LINUXOS_CLIENT_CHECK_REGISTER_HPP_
//---- End of source file ------------------------------------------------------

