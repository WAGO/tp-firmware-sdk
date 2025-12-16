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
///  \brief    Main function for WAGO auth service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/auth_service.hpp"
#include "modules/arguments.hpp"
#include "modules/defines.hpp"
#include "wago/wdx/linuxos/com/parameter_service_proxy.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"
#include "wago/authserv/auth_service_provider.hpp"
#include "system_abstraction.hpp"

#include <wago/logging/log.hpp>
#include <wago/privileges/privileges.hpp>

#include <wc/structuring.h>
#include <wc/assertion.h>
#include <wc/log.h>

#include <atomic>
#include <cstdio>
#include <csignal>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include <sys/poll.h>
#include <unistd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::authserv;
//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
extern "C"
{
    void authd_signal_handler(int arg);
}

namespace {
//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
#if LEGACY_DAEMON
uid_t                      const daemon_uid       =   124U;
gid_t                      const daemon_gid       =   124U;
#endif // LEGACY_DAEMON

std::atomic<bool>                shutdown_requested(false);
std::atomic<bool>                reload_requested(false);

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
}

extern "C" 
{
void authd_signal_handler(int signum)
{
#if SYSTEMD_INTEGRATION
    sal::systemd::get_instance().notify_stopping();
#endif
    switch (signum)
    {
        case SIGINT:  // parasoft-suppress CERT_C-CON37-a-2 "SIGINT is not used to terminate threads."
        case SIGTERM: // parasoft-suppress CERT_C-CON37-a-2 "SIGTERM is not used to terminate threads."
            // this will break the main loop
            shutdown_requested = true;
            break;
        case SIGUSR1:
            // this will reload the auth service on next loop
            reload_requested = true;
            break;
        default:
            break;
    }
}
} //extern "C"

int main(int    argc, // parasoft-suppress CERT_CPP-ERR50-d-3 "Minimal log handling in catch for main necessary"
         char **argv)
{
    try
    {
        printf("~~~ Daemon for %s %s ~~~\n", service_name, service_version_string);

        arguments args(argc, argv);

        // Check for help request
        if(args.is_help_requested())
        {
            std::cout << args.get_help() << std::endl;
            exit(0);
        }

        // Setup log functionality
        wago::logging::init(service_file);

        // Setup logging options
        switch(args.get_log_channel())
        {
            case log_channel::std_out:
                wago::logging::set_log_function(wago::logging::log_stdout);
                break;

            default:
                WC_FAIL(("Log channel " + std::to_string(static_cast<unsigned>(args.get_log_channel())) + " not supported").c_str());
                WC_FALLTHROUGH;

            case log_channel::std_err:
                wago::logging::set_log_function(wago::logging::log_stderr);
                break;

            case log_channel::syslog:
                wago::logging::set_log_function(wago::logging::log_syslog);
                break;

            case log_channel::journal:
                wago::logging::set_log_function(wago::logging::log_journal);
                break;
        }
        wago::logging::set_log_level(args.get_log_level());

        // Check commandline parsing results
        if(args.has_parse_error())
        {
            std::string const error_text = "Failed to parse commandline arguments: " + args.get_parse_error();
            wc_log(log_level_t::fatal, error_text);
            std::cerr << error_text << std::endl;
            exit(1);
        }

        // Setup signals for shutdown or reload request
        struct sigaction act;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        act.sa_handler = authd_signal_handler;
        sigaction(SIGINT,  &act, nullptr); /* shutdown */ // parasoft-suppress CERT_C-CON37-a-2 "SIGINT is not used to terminate threads."
        sigaction(SIGTERM, &act, nullptr); /* shutdown */ // parasoft-suppress CERT_C-CON37-a-2 "SIGTERM is not used to terminate threads."
        sigaction(SIGUSR1, &act, nullptr); /* reload   */

        // Setup auth service
        try 
        {
            std::atomic<bool> parameter_service_initialized(false);
            wago::wdx::linuxos::com::parameter_service_proxy parameter_service(service_name);
            auth_service service(service_file, service_name, args.get_web_api_base_path(), args.get_web_api_socket(), parameter_service.get_frontend());
            std::shared_ptr<oauth_token_handler_i> token_handler = service.get_oauth_token_handler();
            auth_service_provider auth_service_prov(service.get_settings_store(), token_handler);
            auto &web_api       = service.get_web_api_server();

#if LEGACY_DAEMON
            // Drop privileges
            wago::privileges::drop_privileges(daemon_uid, daemon_gid);
#endif

            parameter_service.get_notification_manager().add_notifier([&parameter_service_initialized, &auth_service_prov](wago::wdx::parameter_service_backend_i &backend) {
                if(parameter_service_initialized)
                {
                    wc_log(info, "WAGO Auth Service parameter client re-connected");
                }
                size_t           retry_delay = 100000;
                size_t const max_retry_delay = 10000000;
                while(!parameter_service_initialized && !shutdown_requested)
                {
                    wc_log(info, "WAGO Auth Service parameter client connected");
                    // Application lifecycle init
                    try
                    {
                        auto register_result = backend.register_parameter_provider(&auth_service_prov);
                        register_result.set_notifier([](auto &&result){
                            if (result.has_error())
                            {
                                wc_log(log_level_t::error, "Failed to register auth service parameter provider: " + result.get_message());
                            }
                        });
                        register_result.set_exception_notifier([](std::exception_ptr exception){
                            try { std::rethrow_exception(exception); }
                            catch (std::exception const &e)
                            {
                                wc_log(log_level_t::error, "Failed to register auth service parameter provider: " + std::string(e.what()));
                            }
                        });
                        parameter_service_initialized = true;
                    }
                    // Catch exceptions thrown by registration process via IPC
                    catch(wago::wdx::linuxos::com::exception const &e)
                    {
                        usleep(retry_delay);
                        retry_delay = std::min(max_retry_delay, retry_delay * 2);
                    }
                    // Catch exceptions thrown by check function
                    catch(std::exception const &e)
                    {
                        wc_log_format(error, "Failed to initialize auth service parameter client: %s", e.what());

                        // Stop retrying but don't exit as auth service is running
                        return;
                    }
                }
            });

            // Start web service and wait for shutdown request
            std::vector<pollfd> poll_list = {
                { web_api.get_native_handle(), POLLIN, 0 }
            };
            std::thread thread([&parameter_service, &token_handler](){
                while(!shutdown_requested)
                {
                    parameter_service.run_once(3600000); // one hour
                    token_handler->cleanup_blacklist();
                }
            });
#if SYSTEMD_INTEGRATION
                sal::systemd::get_instance().notify_ready();
#endif
            wc_log(log_level_t::debug, "Enter main loop.");
            while(!shutdown_requested)
            {
                if(reload_requested)
                {
                    service.reload_config();
                    reload_requested = false;
                }
                ::poll(poll_list.data(), poll_list.size(), -1);
                if((poll_list[0].revents & (POLLIN)) > 0)
                {
                    web_api.receive_next();
                }
                token_handler->cleanup_blacklist();
            }
            parameter_service.stop();
            thread.join();
        }
        catch(std::exception const &e)
        {
            wc_log_format(log_level::error, "Uncaught exception, terminating. what(): %s", e.what());
        }
        wago::logging::close();

        return 0;
    }
    catch(std::exception const &e)
    {
        wc_log(fatal, "Terminated due to a critical program failure: " + std::string(e.what()));
        std::terminate();
    }
    catch(...)
    {
        wc_log(fatal, "Terminated due to a critical program failure: unknown exception");
        std::terminate();
    }
}

GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-attribute=noreturn")
void wc_Fail(char const * const szReason,
             char const * const szFile,
             char const * const szFunction,
             int const line)
{
    std::string problem = szReason;
    problem += " [";
    problem += "from ";
    problem += szFile;
    problem += " in function ";
    problem += szFunction;
    problem += ", line ";
    problem += std::to_string(line);
    problem += "]";

    // Log problem
    wc_log(fatal, problem);

    // Trigger standard assert
    assert(false);
}
GNUC_DIAGNOSTIC_POP

//---- End of source file ------------------------------------------------------
