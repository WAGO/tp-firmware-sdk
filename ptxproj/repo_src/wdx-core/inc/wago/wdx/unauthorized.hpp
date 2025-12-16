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
//------------------------------------------------------------------------------

#ifndef INC_WAGO_WDX_UNAUTHORIZED_HPP_
#define INC_WAGO_WDX_UNAUTHORIZED_HPP_

#include <string>
#include <memory>

namespace wago {
namespace wdx {

/// User data structure used for authorization.
struct user_data
{
    /// Name of the user.
    std::string name;

    /// Create user_data with a given name.
    /// \param name_
    ///   The name of the user.
    explicit user_data(std::string name_)
    : name(name_)
    { }
};

// Internal interface
class parameter_service_i;

/// Interface to retrieve an authorized parameter service interface.
/// Provide a user name to obtain an authorized service interface.
template<class ParameterServiceInterface>
class unauthorized final
{
private:
    std::shared_ptr<parameter_service_i> service_m;
public:

    /// Construct an unauthorized wrapper for a given interface.
    /// \param service
    ///   The parameter service to create the wrapper with.
    explicit unauthorized(std::shared_ptr<parameter_service_i> service) noexcept;

    /// Obtain an authorized parameter service interface for a given user.
    /// \param user
    ///   The user data identifying a user.
    std::unique_ptr<ParameterServiceInterface> authorize(user_data user) const noexcept;
};

}
}

#endif // INC_WAGO_WDX_UNAUTHORIZED_HPP_
