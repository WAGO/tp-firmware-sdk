//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2019-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Request interface for HTTP requests.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_AUTHSERV_HTTP_REQUEST_I_HPP_
#define INC_WAGO_AUTHSERV_HTTP_REQUEST_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/api.h"
#include "response_i.hpp"
#include "http_method.hpp"
#include "uri.hpp"

#include <wc/structuring.h>

#include <vector>
#include <iostream>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace http {

/// An HTTP request is usually handled by an HTTP request handler
/// (wago::paramserv::http::http_request_handler_i). The implementation
/// of this interface is usually technology-dependent.
class request_i
{
    WC_INTERFACE_CLASS(request_i)

public:
    /// Check if the request was provided using encryption or not.
    ///
    /// \return 
    ///    True, if HTTPS was used, false if not.
    virtual bool is_https() const = 0;

    /// Check if the request was issued by localhost or not.
    ///
    /// \return
    ///    True, if request was issued by localhost, false if not.
    virtual bool is_localhost() const = 0;

    /// Get the HTTP method of the request.
    ///
    /// \return 
    ///    The HTTP method of the request
    virtual http_method  get_method() const = 0;

    /// Get the URI the request has been sent to.
    ///
    /// \return 
    ///   The HTTP request URI.
    virtual uri get_request_uri() const = 0;

    /// Checks if a query parameter is present
    ///
    /// \param name
    ///    The name of the query parameter to check for.
    /// \return
    ///    True, if the query parameter is present.
    virtual bool has_query_parameter(std::string const & name) const = 0;

    /// Get a query parameter value by its name
    ///
    /// \param name
    ///    The name of the query parameter to get the value for.
    /// \param decode
    ///    Set to `true` to get the actual value, set to `false` to get
    ///    the original, URL encoded value instead.
    /// \return
    ///    The value of the query parameter or empty string if no value is 
    ///    assigned
    /// \throw std::out_of_range query parameter has not been set
    virtual std::string get_query_parameter(std::string const & name,
                                            bool                decode = true) const = 0;

    /// Checks if a x-www-form-urlencoded parameter is present
    ///
    /// \param name
    ///    The name of the form parameter to check for.
    /// \return
    ///    True, if the form parameter is present.
    virtual bool has_form_parameter(std::string const & name) const = 0;

    /// Get a x-www-form-urlencoded parameter value by its name.
    ///
    /// \param name
    ///    The name of the form parameter to get the value for.
    /// \param decode
    ///    Set to `true` to get the actual value, set to `false` to get
    ///    the original, URL encoded value instead.
    /// \return
    ///    The value of the form parameter or empty string if no value is
    ///    assigned
    /// \throw std::out_of_range query parameter has not been set
    virtual std::string get_form_parameter(std::string const & name,
                                           bool                decode = true) const = 0;

    /// Get a specific HTTP header field value.
    ///
    /// \param header_name
    ///    The name of the request header field, e.g. "Content-Type". 
    ///    The name may be provided in mixed cases using either '-' or '_' 
    ///    as word separator.
    /// \return 
    ///    True, if The HTTP header is exsisting the the request.
    virtual bool has_http_header(std::string const &header_name) const = 0;

    /// Get a specific HTTP header field value.
    ///
    /// \param header_name
    ///    The name of the request header field, e.g. "Content-Type". 
    ///    The name may be provided in mixed cases using either '-' or '_' 
    ///    as word separator.
    /// \return 
    ///    The HTTP header field value as a null terminated string
    virtual std::string get_http_header(std::string const &header_name) const = 0;

    /// Shorthand to get the "Accept" HTTP header field value.
    ///
    /// \return 
    ///    The value of the "Accept" header field
    virtual std::string get_accepted_types() const = 0;

    /// Shorthand to get the "Content-Type" HTTP header field value.
    ///
    /// \return 
    ///    The value of the "Content-Type" header field
    virtual std::string get_content_type() const = 0;

    /// Shorthand to get the "Content-Length" HTTP header field value.
    ///
    /// \return 
    ///    The value of the "Content-Length" header field
    virtual std::string get_content_length() const = 0;

    /// Get the content of the HTTP request body. The content may 
    /// only be retrieved once either using this methof or get_content_stream(). 
    /// Further calls to any of these methods will throw an exception.
    ///
    /// \return 
    ///    The whole content of the request body.
    /// \throw 
    ///    std::runtime_error, when the content is tried to be retrieved 
    ///    more than once
    virtual std::string get_content() = 0;

    /// Get the content stream of the HTTP request body. The content may 
    /// only be retrieved once either using this method or get_content(). 
    /// Further calls to any of these methods will throw an exception.
    /// 
    /// \return 
    ///    The content of the request body as an input stream object.
    /// \throw 
    ///    std::runtime_error, when the content is tried to be retrieved 
    ///    more than once
    virtual std::istream &get_content_stream() = 0;

    /// Set an HTTP header field and value for the response. If called 
    /// multiple times for the same header field name, the last call will
    /// be applied, overriding any earlier values.
    ///
    /// \param header_name
    ///    The name of the header field, which may be mixed case and uses
    ///    '-' or '_' as word separators.
    /// \param header_value
    ///    The value or the header field. No validation will be done.
    virtual void add_response_header(std::string const &header_name,
                                     std::string const &header_value) = 0;

    /// Send a response to this request. After calling this method, no more 
    /// calls to \ref add_response_header or \ref respond are allowed.
    /// Subsequent calls will generate an error log but have no effect.
    ///
    /// \pre The request must not be responded, yet
    /// \post The request is considered beeing responded
    ///
    /// \param response
    ///    The response to be sent.
    virtual void respond(response_i const &response) noexcept = 0;


    /// Check whether the request has been responded or not.
    ///
    /// \return
    ///    True, if the request has been responded, false otherwise.
    virtual bool is_responded() const = 0;

    /// Send bytes as part of the response body.
    ///
    /// \param bytes
    ///    The bytes to be sent.
    /// \param size
    ///    The size of the bytes array.
    /// \throw std::runtime_error
    ///    If not ready to send data
    virtual void send_data(char const *bytes, size_t size) = 0;

    /// Finishing the request. A request gets finished automatically on 
    /// destruction if not called manually.
    ///
    /// \throw std::runtime_error
    ///    If not ready to be finished or already finished
    virtual void finish() = 0;

    // Interface extensions

    /// Check if a content type is set in the request.
    /// \return 
    ///    True, if a content type is set on the HTTP request false otherwise
    bool is_content_type_set() const;

    /// Check if the request's "Accept" header value is matching the given 
    /// media type strings. E.g. "Accept: text/*" is matching any type_part2
    /// as long as type_part1 is "text". However, more complex accept headers are 
    /// possible and supported. 
    ///
    /// \param type
    ///    The main type of the media type to check for support.
    /// \param sub_type
    ///    The subtype of the media type to check for support.
    bool is_response_content_type_accepted(std::string const &type,
                                           std::string const &sub_type) const;

    /// Check if the request's "Content-Type" header value is matching the given 
    /// media type strings. 
    ///
    /// \param type
    ///    The main type of the media type to check for support.
    /// \param sub_type
    ///    The subtype of the media type to check for support.
    bool is_content_type_matching(std::string const &type,
                                  std::string const &sub_type) const;
};


} // Namespace http
} // Namespace authserv
} // Namespace wago


#endif // INC_WAGO_AUTHSERV_HTTP_REQUEST_I_HPP_
//---- End of source file ------------------------------------------------------
