//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    WEB-API frontend for WAGO Auth Service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_DEFINITIONS_HPP_
#define SRC_LIBAUTHSERV_DEFINITIONS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/std_includes.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

constexpr char     const endpoint_auth[]            = "/authorize";
constexpr char     const endpoint_token[]           = "/token";
constexpr char     const endpoint_verify[]          = "/verify";
constexpr char     const endpoint_password_change[] = "/password_change";

#define                  POST_CONTENT_TYPE          "application"
#define                  POST_CONTENT_SUBTYPE       "x-www-form-urlencoded"
constexpr char     const post_content_type[]      = POST_CONTENT_TYPE;
constexpr char     const post_content_subtype[]   = POST_CONTENT_SUBTYPE;
constexpr char     const post_content_format[]    = POST_CONTENT_TYPE "/" POST_CONTENT_SUBTYPE;
#undef                   POST_CONTENT_TYPE
#undef                   POST_CONTENT_SUBTYPE

constexpr char     const query_param_response_type[]         = "response_type";
constexpr char     const query_param_client_id[]             = "client_id";
constexpr char     const query_param_scope[]                 = "scope";
constexpr char     const query_param_state[]                 = "state";
constexpr char     const query_param_code_challenge_method[] = "code_challenge_method";
constexpr char     const query_param_code_challenge[]        = "code_challenge";
constexpr char     const query_param_user[]                  = "username";

constexpr char     const response_type_code[]                = "code";
constexpr char     const code_challenge_method_s256[]        = "S256";

constexpr char     const grant_type_code[]                   = "authorization_code";
constexpr char     const grant_type_refresh[]                = "refresh_token";
constexpr char     const grant_type_password[]               = "password";

constexpr char     const form_param_grant_type[]             = "grant_type";
constexpr char     const form_param_token[]                  = "token";
constexpr char     const form_param_code_verifier[]          = "code_verifier";
constexpr char     const form_param_username[]               = "username";
constexpr char     const form_param_password[]               = "password";
constexpr char     const form_param_new_password[]           = "new_password";
constexpr char     const * const form_param_code             = response_type_code;
constexpr char     const * const form_param_client_id        = query_param_client_id;
constexpr char     const * const form_param_refresh_token    = grant_type_refresh;
constexpr char     const * const form_param_scope            = query_param_scope;

constexpr char const password_setup_page_file_path[]         = HTML_TEMPLATE_LOCATION "/password_setup.html.template";
constexpr char const login_page_file_path[]                  = HTML_TEMPLATE_LOCATION "/login.html.template";
constexpr char const password_change_page_file_path[]        = HTML_TEMPLATE_LOCATION "/password_change.html.template";
constexpr char const confirmation_page_file_path[]           = HTML_TEMPLATE_LOCATION "/confirmation.html.template";

constexpr size_t const max_client_id_log_length              = 32U;

} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_DEFINITIONS_HPP_
//---- End of source file ------------------------------------------------------
