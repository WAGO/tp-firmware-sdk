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

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "arguments.hpp"
#include "defines.hpp"

#include <wago/optparsing/get_opt_parser.hpp>
#include <wc/assertion.h>
#include <wc/log.h>

#include <string>
#include <exception>
#include <iostream>
#include <unistd.h>

namespace wago {
namespace authserv {

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
class cli_parser : public optparsing::get_opt_parser
{
    using get_opt_parser::get_opt_parser;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
log_channel parse_log_channel(char const * const  option,
                              std::string  const &value);
log_level_t parse_log_level(  char const * const  option,
                              std::string  const &value);

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
#define LOG_CHANNEL_OPTIONS   "<stdout|stderr|syslog|journal>"
#define LOG_LEVEL_OPTIONS     "<off|fatal|error|warning|notice|info|debug>"

static constexpr char          const web_api_base_path_default[] = "/auth";
static constexpr char          const web_api_socket_default[]    = "/tmp/authserv.fcgi.socket";
static constexpr log_channel   const log_channel_default         = log_channel::syslog;
static constexpr log_level_t   const log_level_default           = log_level_t::warning;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
arguments::arguments(int    argc,
                     char **argv)
: parser_m(std::make_unique<cli_parser>(WAGO_TOOL_NAME, (argc > 0) ? argv[0] : "(unknown)", true))
, log_channel_m(log_channel_default)
, log_level_m(log_level_default)
, web_api_socket_m(web_api_socket_default)
{
    // Configure command line arguments
    parser_m->add_argument('s', "web-api-socket", web_api_socket_m, "FCGI Socket path for WEB-API.");

    optparsing::arg_custom_func log_channel_converter = [log_channel = &log_channel_m] (optparsing::arg_option, std::string const &argument_text) mutable
                                                        { *log_channel = parse_log_channel("log-channel (c)", argument_text); };
    parser_m->add_argument('c', "log-channel",
                           log_channel_converter,
                           "Log channel to use for log output.",
                           LOG_CHANNEL_OPTIONS);

    optparsing::arg_custom_func log_level_converter = [log_level = &log_level_m] (optparsing::arg_option, std::string const &argument_text) mutable
                                                      { *log_level = parse_log_level("log-level (l)", argument_text); };
    parser_m->add_argument('l', "log-level",
                           log_level_converter,
                           "Log level (verbosity) to use for log output.",
                           LOG_LEVEL_OPTIONS);

    parser_m->add_argument('r', "web-api-base", web_api_base_path_m, "URL base path for the WEB-API.");

    // Configure help
    parser_m->set_help_title("===== " WAGO_TOOL_NAME " - A daemon for " SERVICE_NAME " =====");
    parser_m->set_help_additional_info("Option web-api-base is mandatory, "
                                       "if not given the internal defaults will be used.");
    parser_m->add_help_example(WAGO_TOOL_NAME " -l warning",
                               "Set program log level to \"warning\".");
    parser_m->add_help_example(WAGO_TOOL_NAME " --web-api-base /auth",
                               "Program will assume base path \"/auth\" for incoming WEB-API requests.");

    try
    {
        parse(argc, argv);
    }
    catch(std::exception &e)
    {
        parse_error_m = e.what();
    }
    if(!has_parse_error())
    {
        validate();
    }
}

arguments::~arguments() noexcept = default;

bool arguments::has_parse_error()
{
    return !parse_error_m.empty();
}

std::string arguments::get_parse_error()
{
    return parse_error_m;
}

bool arguments::is_help_requested()
{
    return parser_m->is_help_requested();
}

std::string arguments::get_help()
{
    return parser_m->generate_help_text();
}

log_channel arguments::get_log_channel()
{
    return log_channel_m;
}

log_level_t arguments::get_log_level()
{
    return log_level_m;
}

std::string arguments::get_web_api_base_path()
{
    return web_api_base_path_m;
}

std::string arguments::get_web_api_socket()
{
    return web_api_socket_m;
}

void arguments::parse(int   argc,
                      char *argv[])
{
    parser_m->parse(argc, argv);
}

void arguments::validate()
{
    if(web_api_base_path_m.empty() || web_api_base_path_m.at(0) != '/' || (web_api_base_path_m.length() < 2))
    {
        web_api_base_path_m = web_api_base_path_default;
        if(!is_help_requested())
        {
            std::string const warn_message = "No valid base path for REST-API given, using default: " + web_api_base_path_m;
            std::cerr << "Warning: " << warn_message << std::endl;
        }
    }
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
log_channel parse_log_channel(char const * const  option,
                              std::string  const &value)
{
    log_channel result;
    if(value == "syslog")
    {
        result = log_channel::syslog;
    }
    else if(value == "journal")
    {
#if SYSTEMD_INTEGRATION
        result = log_channel::journal;
#else
        throw std::runtime_error("Log channel \"" + value + "\" not supported without systemd integration");
#endif
    }
    else if(value == "stdout")
    {
        result = log_channel::std_out;
    }
    else if(value == "stderr")
    {
        result = log_channel::std_err;
    }
    else
    {
        std::string error = std::string("Error in argument for option \"") + option +
                                        "\": Invalid value \"" + value +
                                        "\", allowed values are: " LOG_CHANNEL_OPTIONS;
        throw std::runtime_error(error);
    }

    return result;
}

log_level_t parse_log_level(char const * const  option,
                            std::string  const &value)
{
    log_level_t result;
    if(value == "off")
    {
        result = log_level_t::off;
    }
    else if(value == "fatal")
    {
        result = log_level_t::fatal;
    }
    else if(value == "error")
    {
        result = log_level_t::error;
    }
    else if(value == "warning")
    {
        result = log_level_t::warning;
    }
    else if(value == "notice")
    {
        result = log_level_t::notice;
    }
    else if(value == "info")
    {
        result = log_level_t::info;
    }
    else if(value == "debug")
    {
        result = log_level_t::debug;
    }
    else
    {
        std::string error = std::string("Error in argument for option \"") + option +
                                        "\": Invalid value \"" + value +
                                        "\", allowed values are: " LOG_LEVEL_OPTIONS;
        throw std::runtime_error(error);
    }

    return result;
}

} // Namespace paramservlinux
} // Namespace wago


//---- End of source file ------------------------------------------------------
