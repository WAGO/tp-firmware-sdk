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
///  \brief    Utility functions for daemon argument parsing.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "client_args.hpp"

#include <wago/optparsing/get_opt_parser.hpp>
#include <wc/assertion.h>

#include <string>
#include <exception>
#include <iostream>
#include <cctype>
#include <unistd.h>

namespace wago {
namespace wdx {
namespace linuxos {
namespace client {
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
tracer      parse_tracer(     char const * const  option,
                              std::string  const &value);
trace_route parse_trace_route(char const * const  option,
                              std::string  const &value);

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
#define WAGO_TOOL_NAME        "client"
#define SERVICE_NAME          "WAGO Parameter Service"
#define LOG_CHANNEL_OPTIONS   "<stdout|stderr|syslog|journal>"
#define LOG_LEVEL_OPTIONS     "<off|fatal|error|warning|notice|info|debug>"
#define TRACER_OPTIONS        "<none|stdout|stderr|ktrace|ktrace-passive>"
#define TRACE_ROUTE_OPTIONS   "<all|lifetime-mgmt|provider-calls>"

static constexpr uint8_t       const max_worker_count = 8;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
arguments::arguments(int                               argc,
                     char                             *argv[],
                     std::string                       name,
                     std::vector<client_option> const  additional_options)
: name_m(std::move(name))
, parser_m(std::make_unique<cli_parser>(WAGO_TOOL_NAME, (argc > 0) ? argv[0] : "(unknown)", true))
{
    // Configure command line arguments
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

    optparsing::arg_custom_func tracer_converter = [tracer = &tracer_m] (optparsing::arg_option, std::string const &argument_text) mutable
                                                   {*tracer = parse_tracer("tracer (t)", argument_text); };
    parser_m->add_argument('t', "tracer",
                           tracer_converter,
                           "Tracer to use for trace marks (if some route is enabled, \"none\" disables tracing).",
                           TRACER_OPTIONS);

    optparsing::arg_custom_func trace_route_converter = [trace_routes = &trace_routes_m] (optparsing::arg_option, std::string const &argument_text) mutable
                                                        { trace_route const route = parse_trace_route("trace-route (a)", argument_text);
                                                          *trace_routes |= static_cast<uint32_t>(route); };
    parser_m->add_argument('a', "trace-route",
                           trace_route_converter,
                           "Trace route to activate for trace marks (option may used multiple times).",
                           TRACE_ROUTE_OPTIONS);

#ifndef SYSTEMD_INTEGRATION
    parser_m->add_argument('u', "user",           desired_user_m,          "Run application with access rights for given user.");
    parser_m->add_argument('g', "group",          desired_group_m,         "Run application with access rights for given group.");
#endif

    parser_m->add_argument('b', "backend-socket", backend_socket_m,        "Use given socket path to connect to " SERVICE_NAME " backend.");
    parser_m->add_argument('w', "worker-count",   desired_worker_count_m,  "Use given count of additional worker threads to serve " SERVICE_NAME " requests.");

    // Configure additional options
    for(auto add_opt: additional_options)
    {
        if(!isupper(add_opt.short_option))
        {
            throw std::runtime_error(std::string("Configured additional client option ")
                                     + "with lower case short option character \'"+ add_opt.short_option + "\'");
        }
        parser_m->add_argument(add_opt.short_option, add_opt.long_option, add_opt.value, add_opt.help_text);
    }

    // Configure help
    parser_m->set_help_title("===== " + name_m + " - A client daemon for " SERVICE_NAME " =====");
    parser_m->set_help_additional_info("This program is a client for " SERVICE_NAME ".");
    parser_m->add_help_example(WAGO_TOOL_NAME " -l warning",
                               "Set program log level to \"warning\".");
    parser_m->add_help_example(WAGO_TOOL_NAME " --user admin --group testgroup",
                               "Program will drop root privileges and use access rights for user \"admin\" and group \"testgroup\".");
    parser_m->add_help_example(WAGO_TOOL_NAME " --worker-count 2",
                               "Program will use two additional worker threads to serve requests from " SERVICE_NAME ".");

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

arguments::arguments(arguments&&) noexcept = default;

arguments& arguments::operator=(arguments&&) noexcept = default;

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

tracer arguments::get_tracer()
{
    return tracer_m;
}

uint32_t arguments::get_trace_routes()
{
    return trace_routes_m;
}

std::string arguments::get_desired_user()
{
    return desired_user_m;
}

std::string arguments::get_desired_group()
{
    return desired_group_m;
}

std::string arguments::get_backend_socket()
{
    return backend_socket_m;
}

uint8_t arguments::get_desired_worker_count()
{
    WC_ASSERT(desired_worker_count_m <= UINT8_MAX);
    return static_cast<uint8_t>(desired_worker_count_m);
}

void arguments::parse(int   argc,
                      char *argv[])
{
    parser_m->parse(argc, argv);
}

void arguments::validate()
{
    if(desired_worker_count_m > max_worker_count)
    {
        WC_STATIC_ASSERT(sizeof(desired_worker_count_m) >= sizeof(max_worker_count));
        desired_worker_count_m = max_worker_count;
        WC_STATIC_ASSERT(sizeof(unsigned) >= sizeof(max_worker_count));
        parse_error_m = "Maximum worker count is " + std::to_string(static_cast<unsigned>(max_worker_count)) +
                        ", but requested worker count is " + std::to_string(desired_worker_count_m);
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

tracer parse_tracer(char const * const  option,
                    std::string  const &value)
{
    tracer result;
    if(value == "none")
    {
        result = tracer::none;
    }
    else if(value == "stdout")
    {
        result = tracer::std_out;
    }
    else if(value == "stderr")
    {
        result = tracer::std_err;
    }
    else if(value == "ktrace")
    {
        result = tracer::ktrace;
    }
    else if(value == "ktrace-passive")
    {
        result = tracer::ktrace_passive;
    }
    else
    {
        std::string error = std::string("Error in argument for option \"") + option +
                                        "\": Invalid value \"" + value +
                                        "\", allowed values are: " TRACER_OPTIONS;
        throw std::runtime_error(error);
    }

    return result;
}

trace_route parse_trace_route(char const * const  option,
                              std::string  const &value)
{
    trace_route result;
    if(value == "lifetime-mgmt")
    {
        result = trace_route::lifetime_mgmt;
    }
    else if(value == "provider-calls")
    {
        result = trace_route::provider_calls;
    }
    else if(value == "all")
    {
        result = trace_route::all;
    }
    else
    {
        std::string error = std::string("Error in argument for option \"") + option +
                                        "\": Invalid value \"" + value +
                                        "\", allowed values are: " TRACE_ROUTE_OPTIONS;
        throw std::runtime_error(error);
    }

    return result;
}

} // Namespace client
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
