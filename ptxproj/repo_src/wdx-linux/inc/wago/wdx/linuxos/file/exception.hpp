//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Exception in context of file operations.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_FILE_EXCEPTION_HPP_
#define INC_WAGO_WDX_LINUXOS_FILE_EXCEPTION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <wago/future.hpp>
#include <wago/wdx/status_codes.hpp>

#include <stdexcept>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {

//------------------------------------------------------------------------------
/// \brief Base exception to be thrown in file operation context.
///
/// All exceptions thrown by WDx file library are derived from this base exception.
//------------------------------------------------------------------------------
class WDXLINUXOSFILE_API exception : public std::runtime_error
{
private:
    wdx::status_codes const code_m;

public:
    //------------------------------------------------------------------------------
    /// Primary constructor to create an exception.
    ///
    /// \param code
    ///   Core status code associated with the problem/error occurred.
    /// \param message
    ///   Error message describing the problem/error occurred.
    //------------------------------------------------------------------------------
    exception(wdx::status_codes        code,
              std::string       const &message) noexcept;

    //------------------------------------------------------------------------------
    /// Copy constructor to copy a exception from another wdx::linuxos::file::exception.
    ///
    /// \param e
    ///   Exception to copy to this wdx::linuxos::file::exception.
    //------------------------------------------------------------------------------
    exception(exception const &e) noexcept = default;

    //------------------------------------------------------------------------------
    /// Move constructor to move a exception from another wdx::linuxos::file::exception.
    ///
    /// \param e
    ///   Exception to move to this wdx::linuxos::file::exception.
    //------------------------------------------------------------------------------
    exception(exception &&e) noexcept = default;

    // Delete assignment operators
    exception& operator=(exception const &)  = delete;
    exception& operator=(exception       &&) = delete;

    //------------------------------------------------------------------------------
    /// Default destructor of wdx::linuxos::file::exception.
    //------------------------------------------------------------------------------
    ~exception() noexcept override = default;

    //------------------------------------------------------------------------------
    /// Get the associated (wda) core status code.
    ///
    /// \return
    ///   Core status code.
    //------------------------------------------------------------------------------
    wdx::status_codes get_status_code() const noexcept;
};


//------------------------------------------------------------------------------
/// Function to get or create an wdx::linuxos::file::exception from std::exception_ptr.
///
/// \param e_ptr
///   std::exception_ptr to get/create an wdx::linuxos::file::exception from.
///
/// \return
///   wdx::linuxos::file::exception from given std::exception_ptr.
//------------------------------------------------------------------------------
exception get_exception_from_ptr(std::exception_ptr e_ptr) noexcept;


//------------------------------------------------------------------------------
/// Creates a wdx::response for a given std::exception_ptr
///
/// \param e_ptr
///   An std::exception_ptr.
/// \param message_prefix
///   Message prefix for error message on response.
///
/// \return
///   response of the given template type.
//------------------------------------------------------------------------------
template <class ResponseType>
ResponseType create_error_response_from_ptr(std::exception_ptr        e_ptr,
                                            std::string        const &message_prefix)
{
    auto e = wdx::linuxos::file::get_exception_from_ptr(std::move(e_ptr));
    ResponseType error_response(e.get_status_code());
    error_response.message = message_prefix + e.what();

    return error_response;
}

//------------------------------------------------------------------------------
/// Creates an wdx::linuxos::file::exception handler for asynchronous operations based on
/// wago::promise and wdx::response (or derived).
///
/// \param response_promise
///   Shared pointer to wago::promise for wdx::response (or derived).
/// \param message_prefix
///   Message prefix for error message on response.
///
/// \return
///   wago::future exception notifier for automatic responding given promise.
//------------------------------------------------------------------------------
template <class ResponseType>
future_exception_notifier create_exception_handler(std::shared_ptr<promise<ResponseType>>        response_promise,
                                                   std::string                            const &message_prefix)
{
    return [response_promise, message_prefix](std::exception_ptr e_ptr){
        response_promise->set_value(create_error_response_from_ptr<ResponseType>(e_ptr, message_prefix));
    };
}

} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_FILE_EXCEPTION_HPP_

//---- End of source file ------------------------------------------------------
