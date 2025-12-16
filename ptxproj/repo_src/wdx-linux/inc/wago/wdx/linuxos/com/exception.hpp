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
///  \brief    Header for exceptions thrown by WAGO Parameter Service communication library.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_COM_EXCEPTION_HPP_
#define INC_WAGO_WDX_LINUXOS_COM_EXCEPTION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <string>
#include <stdexcept>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

using std::string;

//------------------------------------------------------------------------------
/// \brief Base exception to be thrown in WAGO Parameter Service communication library.
///
/// All exceptions thrown by WAGO Parameter Service communication library are derived from this base exception.
//------------------------------------------------------------------------------
class WDXLINUXOSCOM_API exception : public std::runtime_error
{
public:
    //------------------------------------------------------------------------------
    /// Primary constructor to create an exception.
    ///
    /// \param message
    ///   Error message describing the problem/error occurred.
    //------------------------------------------------------------------------------
    explicit exception(string const &message) noexcept;

    //------------------------------------------------------------------------------
    /// Conversion constructor to create an exception from std::exception.
    ///
    /// \param e
    ///   Exception to create a wdx::linuxos::com::exception from.
    //------------------------------------------------------------------------------
    exception(std::exception const &e) noexcept; // Conversion constructor: NOLINT(google-explicit-constructor)

    //------------------------------------------------------------------------------
    /// Conversion constructor to create an exception from std::runtime_error.
    ///
    /// \param e
    ///   Exception to create a wdx::linuxos::com::exception from.
    //------------------------------------------------------------------------------
    exception(std::runtime_error const &e) noexcept; // Conversion constructor: NOLINT(google-explicit-constructor)

    //------------------------------------------------------------------------------
    /// Copy constructor to copy an exception from another wdx::linuxos::com::exception.
    ///
    /// \param e
    ///   Exception to copy to this wdx::linuxos::com::exception.
    //------------------------------------------------------------------------------
    exception(exception const &e) noexcept = default;

    //------------------------------------------------------------------------------
    /// Copy assign constructor to copy an exception from another wdx::linuxos::com::exception.
    ///
    /// \param e
    ///   Exception to copy to this wdx::linuxos::com::exception.
    //------------------------------------------------------------------------------
    exception& operator=(exception const &e) noexcept = default;

    //------------------------------------------------------------------------------
    /// Move constructor to move an exception from another wdx::linuxos::com::exception.
    ///
    /// \param e
    ///   Exception to move to this wdx::linuxos::com::exception.
    //------------------------------------------------------------------------------
    exception(exception &&e) noexcept = default;

    //------------------------------------------------------------------------------
    /// Move assign constructor to move an exception from another wdx::linuxos::com::exception.
    ///
    /// \param e
    ///   Exception to move to this wdx::linuxos::com::exception.
    //------------------------------------------------------------------------------
    exception& operator=(exception &&e) noexcept = default;

    //------------------------------------------------------------------------------
    /// Default destructor of wdx::linuxos::com::exception.
    //------------------------------------------------------------------------------
    ~exception() noexcept override = default;
};

//------------------------------------------------------------------------------
/// \brief Exception thrown as replacement for remote caught exceptions
///        in WAGO Parameter Service communication library.
//------------------------------------------------------------------------------
class WDXLINUXOSCOM_API remote_exception : public std::runtime_error
{
public:
    //------------------------------------------------------------------------------
    /// Constructor to create a remote exception.
    ///
    /// \param message
    ///   Error message describing the problem/error occurred.
    //------------------------------------------------------------------------------
    explicit remote_exception(string const &message) noexcept;

    //------------------------------------------------------------------------------
    /// Copy constructor to copy a remote_exception from another wdx::linuxos::com::remote_exception.
    ///
    /// \param e
    ///   Remote exception to copy to this wdx::linuxos::com::remote_exception.
    //------------------------------------------------------------------------------
    remote_exception(remote_exception &e) noexcept = default;

    //------------------------------------------------------------------------------
    /// Copy assign constructor to copy a remote_exception from another wdx::linuxos::com::remote_exception.
    ///
    /// \param e
    ///   Remote exception to copy to this wdx::linuxos::com::remote_exception.
    //------------------------------------------------------------------------------
    remote_exception& operator=(remote_exception const &e) noexcept = default;

    //------------------------------------------------------------------------------
    /// Move constructor to move a remote_exception from another wdx::linuxos::com::remote_exception.
    ///
    /// \param e
    ///   Remote exception to move to this wdx::linuxos::com::remote_exception.
    //------------------------------------------------------------------------------
    remote_exception(remote_exception &&e) noexcept = default;

    //------------------------------------------------------------------------------
    /// Move assign constructor to move a remote_exception from another wdx::linuxos::com::remote_exception.
    ///
    /// \param e
    ///   Remote exception to move to this wdx::linuxos::com::remote_exception.
    //------------------------------------------------------------------------------
    remote_exception& operator=(remote_exception &&e) noexcept = default;

    //------------------------------------------------------------------------------
    /// Default constructor of a remote exception.
    //------------------------------------------------------------------------------
    ~remote_exception() noexcept override = default;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_COM_EXCEPTION_HPP_
//---- End of source file ------------------------------------------------------
