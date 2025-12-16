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
///  \brief    Main implementation of a parameter client application
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/client/client.hpp"
#include "wago/wdx/linuxos/client/trace_routes.hpp"

#include "wago/wdx/linuxos/com/parameter_service_backend_proxy.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"
#include "modules/client_args.hpp"

#include <wago/logging/log.hpp>
#include <wago/privileges/privileges.hpp>
#include <wago/wtrace/trace.hpp>
#include <wc/exit.h>
#include <wc/log.h>
#include <wc/structuring.h>

#include <string>
#include <atomic>
#include <iostream>
#include <vector>
#include <thread>
#include <cassert>
#include <csignal>
#include <pthread.h>
#include <syslog.h>
#include <unistd.h>

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
extern "C"
{
    void linuxos_client_signal_handler(int arg);
}

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::linuxos::client;

namespace {
using wago::wdx::linuxos::com::driver_i;
std::atomic<bool>       shutdown_requested(false);
std::atomic<driver_i *> driver(nullptr);

constexpr int const success_exit_status             = WC_EXIT_SUCCESS;
constexpr int const general_error_exit_status       = WC_EXIT_ERROR_GENERAL;
constexpr int const init_fails_exit_status          = WC_EXIT_ERROR_FIRST;
constexpr int const client_setup_fails_exit_status  = WC_EXIT_ERROR_FIRST + 1;
WC_STATIC_ASSERT(   client_setup_fails_exit_status <= WC_EXIT_ERROR_LAST);

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
void rename_thread(std::thread &thread, char const *name)
{
    pthread_setname_np(thread.native_handle(), name);
}
}

void linuxos_client_signal_handler(int WC_UNUSED_PARAM(arg))
{
    // this will break the main loop
    shutdown_requested = true;
    if(driver)
    {
        driver.load()->stop();
    }
}

//------------------------------------------------------------------------------
/// Main function of the executable.
///
/// \return
///   Returns 0 in every case.
///------------------------------------------------------------------------------
int main(int    argc, // parasoft-suppress CERT_CPP-ERR50-d-3 "Minimal log handling in catch for main necessary"
         char** argv)
{
    try 
    {
        // Setup signals for shutdown request
        struct sigaction act;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        act.sa_handler = linuxos_client_signal_handler;
        // parasoft-begin-suppress CERT_C-CON37-a "Only used to react on signals from outside this process."
        sigaction(SIGINT, &act, nullptr);
        sigaction(SIGTERM, &act, nullptr);
        // parasoft-end-suppress CERT_C-CON37-a

        std::unique_ptr<abstract_client> client;
        std::string                      client_name = argv[0];
        std::unique_ptr<arguments>       args;
        try
        {
            client      = create_client();
            client_name = client->get_name();
            args        = std::make_unique<arguments>(argc, argv, client_name, client->get_additional_options());
        }
        catch(const std::exception &e)
        {
            printf("Failed to setup client: %s\n", e.what());
            wc_exit(client_setup_fails_exit_status);
        }

        // Setup log functionality
        wago::logging::init(client_name.c_str());

        // Setup logging options
        switch(args->get_log_channel())
        {
            case log_channel::std_out:
                wago::logging::set_log_function(wago::logging::log_stdout);
                break;

            default:
                WC_FAIL(("Log channel " + std::to_string(static_cast<unsigned>(args->get_log_channel())) + " not supported").c_str());
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
        wago::logging::set_log_level(args->get_log_level());

        // Check commandline parsing results
        if(args->has_parse_error())
        {
            std::string const error_text = "Failed to parse commandline arguments: " + args->get_parse_error();
            wc_log(log_level_t::fatal, error_text);
            std::cerr << error_text << std::endl;
            wc_exit(general_error_exit_status);
        }
        if(args->is_help_requested())
        {
            std::cout << args->get_help() << std::endl;
            wc_exit(success_exit_status);
        }

        // Setup tracer
        switch(args->get_tracer())
        {
            case tracer::std_out:
                wago::wtrace::set_tracer(wago::wtrace::get_stdout_tracer());
                break;

            case tracer::std_err:
                wago::wtrace::set_tracer(wago::wtrace::get_stderr_tracer());
                break;

            case tracer::ktrace:
                wago::wtrace::set_tracer(wago::wtrace::get_kernel_tracer());
                break;

            case tracer::ktrace_passive:
                wago::wtrace::set_tracer(wago::wtrace::get_passive_kernel_tracer());
                break;

            default:
                WC_FAIL(("Tracer " + std::to_string(static_cast<unsigned>(args->get_tracer())) + " not supported").c_str());
            case tracer::none:
                break;
        }
        if(args->get_tracer() != tracer::none)
        {
            for(auto const &route : all_trace_routes)
            {
                if(static_cast<uint32_t>(route) & args->get_trace_routes())
                {
                    wc_trace_activate_channel(for_route(route));
                }
            }
        }

    #ifndef SYSTEMD_INTEGRATION
        // Drop privileges
        if(args->get_desired_user() != "root")
        {
            wago::privileges::drop_privileges(args->get_desired_user().c_str(), args->get_desired_group().c_str());
        }
    #endif

        // Setup proxy
        std::atomic<bool> initialized(false);
        wago::wdx::linuxos::com::parameter_service_backend_proxy service(client->get_name(), args->get_backend_socket(),
                                                                for_route(trace_route::provider_calls));
        service.get_notification_manager().add_notifier([&client, &initialized] (wago::wdx::parameter_service_backend_i &backend) {
            if(initialized)
            {
                wc_log(info, "WAGO Parameter Service client re-connected");
            }
            size_t           retry_delay = 100000;
            size_t const max_retry_delay = 10000000;
            while(!initialized)
            {
                wc_log(info, "WAGO Parameter Service client connected");
                // Application lifecycle init
                wc_trace_start_channel(for_route(trace_route::lifetime_mgmt));
                WC_TRACE_SET_MARKER(for_route(trace_route::lifetime_mgmt), "Client: Do initialization");
                try
                {
                    client->init({ backend });
                }
                // Catch exceptions thrown by registration process via IPC:
                // This type of exception is expected to occurr only when the server
                // is currently not running, which is a temporary situation by
                // design. As it is not possible to say how long the server will be
                // unavailable, we will retry to connect to the server indefinitly
                // using an incrementing sleep time.
                catch(const wago::wdx::linuxos::com::exception &e)
                {
                    WC_TRACE_SET_MARKER(for_route(trace_route::lifetime_mgmt), "Client: Initialization failed");
                    usleep(retry_delay);
                    retry_delay = std::min(max_retry_delay, retry_delay * 2);
                    continue;
                }
                // Catch exceptions thrown by check function
                catch(const std::exception &e)
                {
                    WC_TRACE_SET_MARKER(for_route(trace_route::lifetime_mgmt), "Client: Initialization failed permanently");
                    wc_trace_stop_channel(for_route(trace_route::lifetime_mgmt));
                    wc_log_format(fatal, "Failed to initialize client: %s", e.what());

                    // Eventually registered providers will automatically be unregistered when client closes connection
                    wc_exit(init_fails_exit_status);
                }
                WC_TRACE_SET_MARKER(for_route(trace_route::lifetime_mgmt), "Client: Initialization done");
                wc_trace_stop_channel(for_route(trace_route::lifetime_mgmt));
                initialized = true;
            }
        });
        driver = &service;

        // Create worker threads if requested
        std::vector<std::thread> workers(args->get_desired_worker_count());
        for(size_t i = 0; i < args->get_desired_worker_count(); ++i)
        {
            std::thread worker([&service]{
                while(!shutdown_requested)
                {
                    service.run();
                }
            });
            rename_thread(worker, ("worker_" + std::to_string(i)).c_str());
            workers[i] = std::move(worker);
        }

        while(!shutdown_requested)
        {
            service.run();
        }

        // Wait for open worker threads
        for(auto &worker : workers)
        {
            worker.join();
        }

        // Reset handlers signal
        // parasoft-begin-suppress CERT_C-CON37-a "Only used to react on signals from outside this process."
        sigaction(SIGINT, nullptr, nullptr);
        sigaction(SIGTERM, nullptr, nullptr);
        // parasoft-end-suppress CERT_C-CON37-a

        // Destroy client
        wc_trace_start_channel(for_route(trace_route::lifetime_mgmt));
        WC_TRACE_SET_MARKER(for_route(trace_route::lifetime_mgmt), "Client: Destroy");
        client.reset();
        WC_TRACE_SET_MARKER(for_route(trace_route::lifetime_mgmt), "Client: Destruction done");
        wc_trace_stop_channel(for_route(trace_route::lifetime_mgmt));

        // Close log functionality
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
//------------------------------------------------------------------------------
/// Fatal error handler, called when a runtime assertion is hit.
///
/// \param szReason
///   Fail reason as text.
/// \param szFile
///   File name where the failure occurred.
/// \param szFunction
///   Function name where the failure occurred.
/// \param line
///   Line number where the failure occurred.
//------------------------------------------------------------------------------
void wc_Fail(char const * const szReason, // parasoft-suppress CERT_CPP-DCL60-a-3 "Intentionally replacing other as weak marked functions"
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
    exit(general_error_exit_status);
}
GNUC_DIAGNOSTIC_POP


//---- End of source file ------------------------------------------------------
