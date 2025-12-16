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
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCLIENT_MODULES_CLIENT_ARGS_HPP_
#define SRC_LIBWDXLINUXOSCLIENT_MODULES_CLIENT_ARGS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/definitions.hpp"
#include "wago/wdx/linuxos/client/trace_routes.hpp"
#include "wago/wdx/linuxos/client/client_option.hpp"

#include <wago/wtrace/trace.hpp>
#include <wc/std_includes.h>
#include <wc/structuring.h>
#include <wc/assertion.h>
#include <wc/log.h>

#include <string>
#include <vector>
#include <memory>

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace client {

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
    none           = 0,
    std_out        = 1,
    std_err        = 2,
    ktrace         = 3,
    ktrace_passive = 4
};

class arguments final
{
private:
    std::string name_m;
    parser_ptr  parser_m;
    std::string parse_error_m;

    // Argument configuration values
    log_channel log_channel_m          = log_channel::syslog;
    log_level_t log_level_m            = log_level_t::warning;
    tracer      tracer_m               = tracer::none;
    uint32_t    trace_routes_m         = 0;
    std::string desired_user_m         = "user";
    std::string desired_group_m        = WC_ARRAY_TO_PTR(wdx::linuxos::com::backend_user_group);
    std::string backend_socket_m       = WC_ARRAY_TO_PTR(wdx::linuxos::com::backend_domain_socket);
    uint64_t    desired_worker_count_m = 0;
    WC_STATIC_ASSERT(sizeof(trace_routes_m) >= sizeof(trace_route));

    WC_DISBALE_CLASS_COPY_AND_ASSIGN(arguments)

public:
    arguments(int                               argc,
              char                             *argv[],
              std::string                       name,
              std::vector<client_option> const  additional_options = {});
    ~arguments() noexcept;
    arguments(arguments &&) noexcept;
    arguments& operator=(arguments&&) noexcept;

    bool        has_parse_error();
    std::string get_parse_error();
    bool        is_help_requested();
    std::string get_help();
    log_channel get_log_channel();
    log_level_t get_log_level();
    tracer      get_tracer();
    uint32_t    get_trace_routes();
    std::string get_desired_user();
    std::string get_desired_group();
    std::string get_backend_socket();
    uint8_t     get_desired_worker_count();

private:
    void parse(int   argc,
               char *argv[]);
    void validate();
};

} // Namespace client
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCLIENT_MODULES_CLIENT_ARGS_HPP_
//---- End of source file ------------------------------------------------------
