//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
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
#include "paramd_args.hpp"

#include <wago/optparsing/get_opt_parser.hpp>
#include <wago/wdx/linuxos/com/definitions.hpp>
#include <wc/assertion.h>
#include <wc/log.h>

#include <string>
#include <exception>
#include <iostream>
#include <unistd.h>

namespace wago {
namespace wdx {
namespace linuxos {
namespace paramd {

using wdx::wda::trace_route;

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
#define WAGO_TOOL_NAME        "paramd"
#define SERVICE_NAME          "WAGO Parameter Service"
#define LOG_CHANNEL_OPTIONS   "<stdout|stderr|syslog|journal>"
#define LOG_LEVEL_OPTIONS     "<off|fatal|error|warning|notice|info|debug>"
#define TRACER_OPTIONS        "<none|stdout|stderr|ktrace|ktrace-passive>"
#define TRACE_ROUTE_OPTIONS   "<all|rest_api_call|file_api_call|ipc_frontend_call|ipc_file_api_call|ipc_backend_call>"

static constexpr char          const rest_api_base_path_default[] = "/wda";
static constexpr char          const file_api_base_path_default[] = "/files";
static constexpr char          const doc_base_path_default[]      = "";
static constexpr char          const rest_api_socket_default[]    = "/tmp/wdx.wda.fcgi.socket";
static constexpr char          const file_api_socket_default[]    = "/tmp/wdx.wda.files.fcgi.socket";
static constexpr char const *  const frontend_socket_default      = wdx::linuxos::com::frontend_domain_socket;
static constexpr char const *  const file_api_ipc_socket_default  = wdx::linuxos::com::file_api_domain_socket;
static constexpr char const *  const backend_socket_default       = wdx::linuxos::com::backend_domain_socket;
static constexpr log_channel   const log_channel_default          = log_channel::syslog;
static constexpr log_level_t   const log_level_default            = log_level_t::warning;
static constexpr tracer        const tracer_default               = tracer::none;
static constexpr uint32_t      const trace_routes_default         = static_cast<uint32_t>(trace_route::none);
static constexpr bool          const single_thread_mode_default   = false;
static constexpr char          const ordernumber_default[]        = "N/A";
static constexpr char          const firmware_version_default[]   = "N/A";

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
arguments::arguments(int    argc,
                     char **argv)
: parser_m(std::make_unique<cli_parser>(WAGO_TOOL_NAME, (argc > 0) ? argv[0] : "(unknown)", true))
, parse_error_m()
, log_channel_m(log_channel_default)
, log_level_m(log_level_default)
, tracer_m(tracer_default)
, trace_routes_m(trace_routes_default)
, rest_api_base_path_m()
, file_api_base_path_m()
, doc_base_path_m()
, rest_api_socket_m(rest_api_socket_default)
, file_api_socket_m(file_api_socket_default)
, frontend_socket_m(frontend_socket_default)
, file_api_ipc_socket_m(file_api_ipc_socket_default)
, backend_socket_m(backend_socket_default)
, single_thread_mode_m(single_thread_mode_default)
, ordernumber_m(ordernumber_default)
, firmware_version_m(firmware_version_default)
{
    // Configure command line arguments
    parser_m->add_argument('s', "rest-api-socket",     rest_api_socket_m,     "FCGI Socket path or name for REST-API.");
    parser_m->add_argument('z', "file-api-socket",     file_api_socket_m,     "FCGI Socket path or name for File-API.");
    parser_m->add_argument('w', "frontend-socket",     frontend_socket_m,     "Socket path or name for " SERVICE_NAME " frontend.");
    parser_m->add_argument('q', "file-api-ipc-socket", file_api_ipc_socket_m, "Socket path or name for " SERVICE_NAME " file API.");
    parser_m->add_argument('p', "backend-socket",      backend_socket_m,      "Socket path or name for " SERVICE_NAME " backend.");
    parser_m->add_flag(    'u', "single-thread",       single_thread_mode_m,  "Run a single main thread only (experimental feature).");
    parser_m->add_argument('o', "ordernumber",         ordernumber_m,         "Order number used to register the device/headstation in WDx.");
    parser_m->add_argument('v', "firmware-version",    firmware_version_m,    "Firmware version used to register the device/headstation in WDx.");

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

    parser_m->add_argument('r', "rest-api-base", rest_api_base_path_m, "URL base path for the REST-API.");
    parser_m->add_argument('f', "file-api-base", file_api_base_path_m, "URL base path for the File-API.");
    parser_m->add_argument('d', "doc-base", doc_base_path_m, "URL base path where the online documentation is available.");

    // Configure help
    parser_m->set_help_title("===== " WAGO_TOOL_NAME " - A daemon for " SERVICE_NAME " =====");
    parser_m->set_help_additional_info("Options rest-api-base and file-api-base are mandatory, "
                                       "if not given the internal defaults will be used.");
    parser_m->add_help_example(WAGO_TOOL_NAME " -l warning",
                               "Set program log level to \"warning\".");
    parser_m->add_help_example(WAGO_TOOL_NAME " --rest-api-base /wda",
                               "Program will assume base path \"/wda\" for incoming REST-API requests.");

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

char const * arguments::get_tool_name()
{
    return WAGO_TOOL_NAME;
}

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

std::string arguments::get_rest_api_base_path()
{
    return rest_api_base_path_m;
}

std::string arguments::get_file_api_base_path()
{
    return file_api_base_path_m;
}

std::string arguments::get_doc_base_path()
{
    return doc_base_path_m;
}

std::string arguments::get_rest_api_socket()
{
    return rest_api_socket_m;
}

std::string arguments::get_file_api_socket()
{
    return file_api_socket_m;
}

std::string arguments::get_frontend_socket()
{
    return frontend_socket_m;
}

std::string arguments::get_file_api_ipc_socket()
{
    return file_api_ipc_socket_m;
}

std::string arguments::get_backend_socket()
{
    return backend_socket_m;
}

bool arguments::get_single_thread_mode()
{
    return single_thread_mode_m;
}

std::string  arguments::get_ordernumber()
{
    return ordernumber_m;
}

std::string arguments::get_firmware_version()
{
    return firmware_version_m;
}

void arguments::parse(int   argc,
                      char *argv[])
{
    parser_m->parse(argc, argv);
}

void arguments::validate()
{
    if(rest_api_base_path_m.empty() || rest_api_base_path_m.at(0) != '/' || (rest_api_base_path_m.length() < 2))
    {
        rest_api_base_path_m = rest_api_base_path_default;
        if(!is_help_requested())
        {
            std::string const warn_message = "No valid base path for REST-API given, using default: " + rest_api_base_path_m;
            std::cerr << "Warning: " << warn_message << std::endl;
        }
    }
    if(file_api_base_path_m.empty() || file_api_base_path_m.at(0) != '/' || (file_api_base_path_m.length() < 2))
    {
        file_api_base_path_m = file_api_base_path_default;
        if(!is_help_requested())
        {
            std::string const warn_message = "No valid base path for file-API given, using default: " + file_api_base_path_m;
            std::cerr << "Warning: " << warn_message << std::endl;
        }
    }
    if(doc_base_path_m.empty())
    {
        doc_base_path_m = doc_base_path_default;
        if(!is_help_requested())
        {
            std::string const info_message = "No doc path provided";
            std::cerr << "Info: " << info_message << std::endl;
        }
    }
    if((trace_routes_m > 0) && (tracer_m == tracer::none))
    {
        if(!is_help_requested())
        {
            std::string const warn_message = "Trace routes given, but no tracer selected. Routes will be ignored.";
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
    if(value == "rest_api_call")
    {
        result = trace_route::rest_api_call;
    }
    else if(value == "file_api_call")
    {
        result = trace_route::file_api_call;
    }
    else if(value == "ipc_frontend_call")
    {
        result = trace_route::ipc_frontend_call;
    }
    else if(value == "ipc_file_api_call")
    {
        result = trace_route::ipc_file_api_call;
    }
    else if(value == "ipc_backend_call")
    {
        result = trace_route::ipc_backend_call;
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

} // Namespace paramd
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
