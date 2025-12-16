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
#ifndef SRC_PARAMD_MODULES_PARAMD_ARGS_HPP_
#define SRC_PARAMD_MODULES_PARAMD_ARGS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/wda/trace_routes.hpp>
#include <wago/wtrace/trace.hpp>
#include <wc/std_includes.h>
#include <wc/assertion.h>
#include <wc/log.h>

#include <string>
#include <memory>

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace paramd {

class cli_parser;
using parser_ptr = std::unique_ptr<cli_parser>;

enum class log_channel
{
    std_out = 0,
    std_err = 1,
    syslog  = 2,
    journal = 3
};

enum class tracer
{
    none            = 0,
    std_out         = 1,
    std_err         = 2,
    ktrace          = 3,
    ktrace_passive  = 4
};

class arguments final
{
private:
    parser_ptr  parser_m;
    std::string parse_error_m;

    // Argument configuration values
    log_channel log_channel_m;
    log_level_t log_level_m;
    tracer      tracer_m;
    uint32_t    trace_routes_m;
    std::string rest_api_base_path_m;
    std::string file_api_base_path_m;
    std::string doc_base_path_m;
    std::string rest_api_socket_m;
    std::string file_api_socket_m;
    std::string frontend_socket_m;
    std::string file_api_ipc_socket_m;
    std::string backend_socket_m;
    bool        single_thread_mode_m;
    std::string ordernumber_m;
    std::string firmware_version_m;
    WC_STATIC_ASSERT(sizeof(trace_routes_m) >= sizeof(wdx::wda::trace_route));

public:
    arguments(int    argc,
              char **argv);
    ~arguments() noexcept;

    char const * get_tool_name();
    bool         has_parse_error();
    std::string  get_parse_error();
    bool         is_help_requested();
    std::string  get_help();
    log_channel  get_log_channel();
    log_level_t  get_log_level();
    tracer       get_tracer();
    uint32_t     get_trace_routes();
    std::string  get_rest_api_base_path();
    std::string  get_doc_base_path();
    std::string  get_file_api_base_path();
    std::string  get_rest_api_socket();
    std::string  get_file_api_socket();
    std::string  get_frontend_socket();
    std::string  get_file_api_ipc_socket();
    std::string  get_backend_socket();
    bool         get_single_thread_mode();
    std::string  get_ordernumber();
    std::string  get_firmware_version();

private:
    void parse(int   argc,
               char *argv[]);
    void validate();
};


} // Namespace paramd
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_PARAMD_MODULES_PARAMD_ARGS_HPP_
//---- End of source file ------------------------------------------------------
