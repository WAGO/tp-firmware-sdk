//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Request class specific to our frontend.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_REQUEST_HPP_
#define SRC_LIBWDXWDA_REST_REQUEST_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/request_i.hpp"
#include "definitions.hpp"
#include "deserializer_i.hpp"
#include "serializer_i.hpp"
#include "response.hpp"

#include <string>
#include <map>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using http::http_method;
using http::response_i;
using http::request_i;
using std::string;
using std::map;

class request : public request_i
{
private:
    std::unique_ptr<request_i>         original_request_m;
    serializer_i               const  &serializer_m;
    deserializer_i             const  &deserializer_m;
    map<string, string>        const   path_parameters_m;
    string                     const   doc_link_m;

public:
    request(std::unique_ptr<request_i>        original_request,
            serializer_i               const &serializer,
            deserializer_i             const &deserializer,
            map<string, string>        const  path_parameters,
            string                     const &doc_link = "");

    bool is_https() const override;
    bool is_localhost() const override;
    http_method get_method() const override;
    http::uri   get_request_uri() const override;
    bool        has_query_parameter(std::string const & name) const override;
    std::string get_query_parameter(std::string const & name, bool decode = true) const override;
    bool        has_http_header(std::string const &header_name) const override;
    std::string get_http_header(std::string const &header_name) const override;
    std::string get_accepted_types() const override;
    std::string get_content_type() const override;
    std::string get_content_length() const override;
    std::string get_content() override;
    std::istream &get_content_stream() override;
    void add_response_header(std::string const &header_name,
                             std::string const &header_value) override;
    void respond(response_i const &response) noexcept override;
    bool is_responded() const override;
    void send_data(char const *bytes, size_t size) override;

    void finish() override;

    string const &get_doc_link() const;
    bool        has_path_parameter(std::string const &parameter_name) const;
    std::string get_path_parameter(std::string const &parameter_name) const;

    serializer_i   const &get_serializer()   const;
    deserializer_i const &get_deserializer() const;
    
    /// Get URL query parameters for the include field: /a/path?include=firstrelated,secondrelated.nestedrelated
    /// \param relationship_paths Will be filled with include paths component-wise, if any are found
    void get_include_parameters(vector<vector<string>> &relationship_paths) const;

    void get_pagination_parameters(unsigned &page_limit,
                                   unsigned &page_offset) const;

    bool get_errors_as_data_attributes_parameter() const;

    bool get_deferred_parameters_as_errors() const;

    bool get_adjusted_parameters_as_errors() const;

    result_behavior_types get_result_behavior() const;

    std::map<std::string, std::string> get_filter_queries() const;

    template<class T>
    T get_content()
    {
        T result;
        deserializer_m.deserialize(&result, get_content());
        return result;
    }
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_REQUEST_HPP_
//---- End of source file ------------------------------------------------------
