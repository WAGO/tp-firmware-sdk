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
///  \brief    Utility functions for daemon argument parsing.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_AUTHD_MODULES_ARGUMENTS_HPP_
#define SRC_AUTHD_MODULES_ARGUMENTS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/std_includes.h>
#include <wc/assertion.h>
#include <wc/log.h>

#include <string>
#include <memory>

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class cli_parser;
using parser_ptr = std::unique_ptr<cli_parser>;

enum class log_channel
{
    std_out = 0,
    std_err = 1,
    syslog  = 2,
    journal = 3
};

class arguments final
{
private:
    parser_ptr  parser_m;
    std::string parse_error_m;

    // Argument configuration values
    log_channel log_channel_m;
    log_level_t log_level_m;
    std::string web_api_base_path_m;
    std::string web_api_socket_m;

public:
    arguments(int    argc,
              char **argv);
    ~arguments() noexcept;

    bool         has_parse_error();
    std::string  get_parse_error();
    bool         is_help_requested();
    std::string  get_help();
    log_channel  get_log_channel();
    log_level_t  get_log_level();
    std::string  get_web_api_base_path();
    std::string  get_web_api_socket();

private:
    void parse(int   argc,
               char *argv[]);
    void validate();
};


} // Namespace authserv
} // Namespace wago


#endif // SRC_AUTHD_MODULES_ARGUMENTS_HPP_
//---- End of source file ------------------------------------------------------
