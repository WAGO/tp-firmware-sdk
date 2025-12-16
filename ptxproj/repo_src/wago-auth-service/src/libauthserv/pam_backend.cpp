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
///  \brief    PAM authentication backend for user/password combination.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "pam_backend.hpp"
#include "wago/authserv/exception.hpp"

#include <wc/assertion.h>
#include <wc/log.h>

#include <cstdlib>
#include "system_abstraction.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
extern "C"
{
static int conversation(int                  message_count,
                        pam_message  const **message,
                        pam_response       **response,
                        void                *appdata_ptr);
} //extern "C"

static void free_response(size_t       const  message_count,
                          pam_response       *response);

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
class pam_invokation
{
    std::string user_m;
    std::string password_m;
public:
    pam_invokation(std::string user, std::string password)
    : user_m(std::move(user))
    , password_m(std::move(password))
    { }

    int conversation(size_t       const                 message_count,
                     pam_message  const * const * const message,
                     pam_response       *       * const response);
};

pam_backend::pam_backend(std::string service_file)
: pam_service_name(std::move(service_file))
{}

pam_backend::~pam_backend() noexcept = default;

auth_result pam_backend::authenticate(std::string const &user,
                                      std::string const &password)
{
    auth_result result = { false, false, false, user };
    WC_DEBUG_LOG("Try to authenticate user \"" + user + "\" via PAM");

    pam_invokation invokation(user, password);

    pam_handle_t * pam_handle = nullptr;
    pam_conv       pam_conversation_data;
    pam_conversation_data.appdata_ptr = &invokation;
    pam_conversation_data.conv = conversation;
    
    auto &pam = sal::pam::get_instance();

    int pam_status = pam.start(pam_service_name.c_str(), user.c_str(), &pam_conversation_data, &pam_handle);
    if(pam_status != PAM_SUCCESS)
    {
        std::string const error_message = std::string("Failed to start PAM:") +
                                          pam.strerror(pam_handle, pam_status) +
                                          " (" + std::to_string(pam_status) + ")";
        wc_log(log_level_t::error, error_message);
        throw authserv::exception(error_message);
    }

    int const flags = PAM_SILENT;
    pam_status = pam.authenticate(pam_handle, flags);
    if(pam_status != PAM_SUCCESS)
    {
        wc_log(log_level_t::warning, "PAM failed to authenticate user \"" + user +
                                     "\": " + pam.strerror(pam_handle, pam_status) +
                                     " (" + std::to_string(pam_status) + ")");
    }
    else
    {
        pam_status = pam.acct_mgmt(pam_handle, 0);
        switch(pam_status)
        {
            case PAM_SUCCESS:
                wc_log(log_level_t::debug, "PAM successfully authenticated user \"" + user + "\"");
                result.success = true;
                break;
            case PAM_NEW_AUTHTOK_REQD:
                result.success = true;
                result.expired = true;
                break;
            case PAM_ACCT_EXPIRED:
                // User not allowed in anymore
            case PAM_USER_UNKNOWN:
            default:
                wc_log(log_level_t::warning, "PAM failed to authorize user \"" + user +
                                             "\": " + pam.strerror(pam_handle, pam_status) +
                                             " (" + std::to_string(pam_status) + ")");
        }
    }

    pam_status = pam.end(pam_handle, pam_status);
    if(pam_status != PAM_SUCCESS)
    {
        wc_log(log_level_t::error, "Failed to end PAM:" +
                                   std::string(pam.strerror(pam_handle, pam_status)) +
                                   " (" + std::to_string(pam_status) + ")");
    }
    return result;
}

int pam_invokation::conversation(size_t       const                 message_count,
                                 pam_message  const * const * const message,
                                 pam_response       *       * const response)
{
    // Check parameters
    WC_ASSERT_RETURN(message  != nullptr, PAM_CONV_ERR);
    WC_ASSERT_RETURN(response != nullptr, PAM_CONV_ERR);

    int result = PAM_SUCCESS;

    // Create response
    pam_response *prepared_response = static_cast<pam_response *>(calloc(message_count, sizeof(*prepared_response)));
    WC_ASSERT_RETURN(prepared_response != nullptr, PAM_CONV_ERR);
    for(size_t i = 0; (i < message_count) && (result == PAM_SUCCESS); ++i)
    {
        // Guess expected response based on selected output style (same assumptions made as in PHP7 PAM module)
        switch(message[i]->msg_style)
        {
            case PAM_PROMPT_ECHO_ON:
                prepared_response[i].resp = strdup(user_m.c_str());
                result = PAM_SUCCESS;
                break;

            case PAM_PROMPT_ECHO_OFF:
                prepared_response[i].resp = strdup(password_m.c_str());
                result = PAM_SUCCESS;
                break;

            default:
                free_response(message_count, prepared_response);
                result = PAM_CONV_ERR;
        }
    }

    // Return results
    if(result == PAM_SUCCESS)
    {
        *response = prepared_response;
    }
    return result;
} // parasoft-suppress CERT_C-FIO42-a-3 CERT_C-MEM31-a-2 CERT_CPP-ERR57-a-3 CERT_CPP-FIO51-a-3 "Bug in Parasoft memory analysis"

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
extern "C"
{
static int conversation(int                  message_count,
                        pam_message  const **message,
                        pam_response       **response,
                        void                *appdata_ptr)
{
    // Check parameters
    WC_ASSERT_RETURN(appdata_ptr   != nullptr, PAM_CONV_ERR);
    WC_ASSERT_RETURN(message_count >= 0,       PAM_CONV_ERR);

    // Call conversation method
    pam_invokation *auth = reinterpret_cast<pam_invokation *>(appdata_ptr);
    WC_STATIC_ASSERT(sizeof(message_count) <= sizeof(size_t));
    return auth->conversation(static_cast<size_t>(message_count), message, response);
}
} //extern "C"

static void free_response(size_t       const  message_count,
                          pam_response       *response)
{
    if(response != nullptr)
    {
        for(size_t i = 0; i < message_count; ++i)
        {
            if(response[i].resp != nullptr)
            {
                free(response[i].resp);
            }
        }
        free(response);
    }
}

} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
