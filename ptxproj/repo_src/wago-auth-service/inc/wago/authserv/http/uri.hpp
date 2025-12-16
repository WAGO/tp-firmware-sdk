//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Helper class for HTTP URIs.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_AUTHSERV_HTTP_URI_HPP_
#define INC_WAGO_AUTHSERV_HTTP_URI_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/api.h"

#include <string>
#include <map>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace http {

class AUTHSERV_API uri
{
private:
    std::string                        uri_m;
    std::string                        path_m;
    std::string                        query_m;

public:

    /// Create an empty / unset URI.
    uri() = default;

    /// Create an URI from a string. The URI must at least contain the protocol
    /// and host. Port number, path and query parts are optional.
    /// 
    /// \param uri_str 
    ///     The URI as a string.
    /// \throw std::invalid_argument 
    ///     The URI string is malformed.
    uri(std::string uri_str); // NOLINT(google-explicit-constructor)

    /// Create an URI from a string. The URI must at least contain the protocol
    /// and host. Port number, path and query parts are optional.
    /// 
    /// \param uri_str 
    ///     The URI as a c-string.
    /// \throw std::invalid_argument 
    ///     The URI string is malformed.
    uri(char const * uri_str); // NOLINT(google-explicit-constructor)

    /// Get the URI as a string.
    /// 
    /// \return 
    ///     The URI as a string.
    std::string const & as_string() const noexcept;

    /// Check if two URIs are equal
    /// 
    /// \param other_uri
    /// \return 
    ///     The URIs are equal.
    bool operator==(uri const &other_uri) const noexcept;

    /// Get the path part of the URI. Path components are usually URL encoded.
    ///
    /// \return 
    ///    Path of the URI.
    std::string get_path() const noexcept;

    /// Get the query part of the URI.
    ///
    /// \return 
    ///    Query part of the URI.
    std::string get_query() const noexcept;

    /// Parse the URIs query string into a map.
    ///
    /// \return
    ///    The names and values of found parameters. Empty string values are
    ///    used for value-less parameters as well.
    std::map<std::string, std::string> parse_query() const;

    /// Standalone function to parse a query string into a map. The query string 
    /// can start with '?' but hasn't to.
    ///
    /// \param query
    ///    The query to parse. The query string parameter keys and values have
    ///    to be URL-encoded.
    /// \return
    ///    The names and values of found parameters. Empty string values are
    ///    used for value-less parameters as well.
    static std::map<std::string, std::string> parse_query(std::string const &query);
};


} // Namespace http
} // Namespace authserv
} // Namespace wago


#endif // INC_WAGO_AUTHSERV_HTTP_URI_HPP_
//---- End of source file ------------------------------------------------------
