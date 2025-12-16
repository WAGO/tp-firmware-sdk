//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Main function for WAGO parameter service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/serv/parameter_service.hpp"
#include "modules/paramd_args.hpp"
#include "system_abstraction.hpp"

#include <wago/logging/log.hpp>
#if LEGACY_DAEMON
#include <wago/privileges/privileges.hpp>
#endif
#include <wago/privileges/user.hpp>
#include <wago/privileges/group.hpp>
#include <wago/wdx/linuxos/com/parameter_service_frontend_stub.hpp>
#include <wago/wdx/linuxos/com/parameter_service_file_api_stub.hpp>
#include <wago/wdx/linuxos/com/parameter_service_backend_stub.hpp>
#include <wago/wtrace/trace.hpp>
#include <wc/structuring.h>
#include <wc/assertion.h>
#include <wc/log.h>

#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <cstdio>
#include <csignal>
#include <cassert>
#include <cstring>
#include <fstream>
#include <streambuf>
#include <regex>

#include <sys/poll.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using std::string;
using std::vector;
using std::make_shared;
using std::make_unique;
using std::shared_ptr;
using std::unique_ptr;
using wago::wdx::wda::trace_route;
using wago::wdx::wda::all_trace_routes;

using namespace wago::wdx::linuxos::serv;
using namespace wago::wdx::linuxos::com;
using namespace wago::wdx::linuxos::paramd;
using namespace wago::wdx::linuxos::sal;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
extern "C"
{
    void linuxos_paramd_signal_handler(int arg);
}

namespace {
using wago::wdx::linuxos::com::driver_i;

inline void rename_thread(std::thread &thread, char const *name) noexcept
{
    pthread_setname_np(thread.native_handle(), name);
}

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
uint32_t          const shutdown_cleanup_timeout_ms = 1000U;
#if LEGACY_DAEMON
constexpr char    const daemon_user[]               = "paramd";
constexpr char    const daemon_group[]              = "paramd";
#endif // LEGACY_DAEMON
constexpr char    const frontend_socket_group[]     = "wdaadm";
constexpr char    const backend_socket_group[]      = "wdxprov";

std::atomic<bool>               shutdown_requested(   false  );
std::atomic<parameter_service*> service_instance(     nullptr);
std::atomic<driver_i *>         frontend_driver(      nullptr);
std::atomic<driver_i *>         file_api_driver(      nullptr);
std::atomic<driver_i *>         backend_driver(       nullptr);

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
}

void linuxos_paramd_signal_handler(int WC_UNUSED_PARAM(arg))
{
#if SYSTEMD_INTEGRATION
    systemd::get_instance().notify_stopping();
#endif

    // service cleanup
    if (service_instance)
    {
        service_instance.load()->cleanup(shutdown_cleanup_timeout_ms);
    }

    // this will break the main loop
    shutdown_requested = true;
    if(frontend_driver)
    {
        frontend_driver.load()->stop();
    }
    if(file_api_driver)
    {
        file_api_driver.load()->stop();
    }
    if(backend_driver)
    {
        backend_driver.load()->stop();
    }
}

int main(int    argc, // parasoft-suppress CERT_CPP-ERR50-d-3 "Minimal log handling in catch for main necessary"
         char **argv)
{
    try
    {
        printf("~~~ Daemon for %s %s ~~~\n", parameter_service::get_service_identity().get_name(),
                                            parameter_service::get_service_identity().get_version_string());

        wago::wdx::linuxos::paramd::arguments args(argc, argv);

        // Check for help request
        if(args.is_help_requested())
        {
            std::cout << args.get_help() << std::endl;
            exit(0);
        }

        // Setup log functionality
        wago::logging::init(args.get_tool_name());

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

        // Setup signals for shutdown request
        struct sigaction act;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        act.sa_handler = linuxos_paramd_signal_handler;
        // parasoft-begin-suppress CERT_C-CON37-a "Only used to react on signals from outside this process."
        sigaction(SIGINT,  &act, nullptr);
        sigaction(SIGTERM, &act, nullptr);
        // parasoft-end-suppress CERT_C-CON37-a

        // Setup tracer
        switch(args.get_tracer())
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
                WC_FAIL(("Tracer " + std::to_string(static_cast<unsigned>(args.get_tracer())) + " not supported").c_str());
            case tracer::none:
                break;
        }
        if(args.get_tracer() != tracer::none)
        {
            for(trace_route const &route : all_trace_routes)
            {
                if(static_cast<uint32_t>(route) & args.get_trace_routes())
                {
                    wc_trace_activate_channel(for_route(route));
                }
            }
        }

        // should we run multiple threads or not?
        bool single_threaded_mode = args.get_single_thread_mode();

        // Setup parameter service
        uid_t const frontend_socket_uid = 0U;
        gid_t const frontend_socket_gid = wago::privileges::get_gid_for_group(frontend_socket_group);
        uid_t const file_api_socket_uid = frontend_socket_uid;
        gid_t const file_api_socket_gid = frontend_socket_gid;
        uid_t const backend_socket_uid  = 0U;
        gid_t const backend_socket_gid  = wago::privileges::get_gid_for_group(backend_socket_group);
        parameter_service service(args.get_ordernumber(), args.get_firmware_version(),
                                  args.get_rest_api_base_path(), args.get_rest_api_socket(),
                                  args.get_file_api_base_path(), args.get_file_api_socket(),
                                  args.get_doc_base_path());
        service_instance = &service;
        auto &rest_api   = service.get_rest_api_server();
        auto &file_api   = service.get_file_api_server();

        // Create frontend stub which uses the original service frontend
        parameter_service_frontend_stub frontend_stub("paramd frontend", service.get_unauthorized_frontend(),
                                                    args.get_frontend_socket(), frontend_socket_uid, frontend_socket_gid,
                                                    for_route(trace_route::ipc_frontend_call));
        frontend_driver = &frontend_stub;

        // Create file api stub which uses the original service file api
        parameter_service_file_api_stub file_api_stub("paramd file api", service.get_unauthorized_file_api(),
                                                    args.get_file_api_ipc_socket(), file_api_socket_uid, file_api_socket_gid,
                                                    for_route(trace_route::ipc_file_api_call));
        file_api_driver = &file_api_stub;

        // Create backend stub which uses the original service backend
        parameter_service_backend_stub backend_stub("paramd backend", service.get_backend(),
                                                    args.get_backend_socket(), backend_socket_uid, backend_socket_gid,
                                                    for_route(trace_route::ipc_backend_call));
        backend_driver = &backend_stub;

#if LEGACY_DAEMON
        // Drop privileges
        uid_t const daemon_uid = wago::privileges::get_uid_for_user(daemon_user);
        gid_t const daemon_gid = wago::privileges::get_gid_for_group(daemon_group);
        wago::privileges::drop_privileges(daemon_uid, daemon_gid);
#endif

        std::thread frontend_stub_driver;
        std::thread file_api_stub_driver;
        std::thread backend_stub_driver;
        if (!single_threaded_mode)
        {
            frontend_stub_driver = std::thread([&frontend_stub]{
                while(!shutdown_requested)
                {
                    frontend_stub.run();
                }
            });
            rename_thread(frontend_stub_driver, "frontend-stub");

            file_api_stub_driver = std::thread([&file_api_stub]{
                while(!shutdown_requested)
                {
                    file_api_stub.run();
                }
            });
            rename_thread(file_api_stub_driver, "file-api-stub");

            backend_stub_driver = std::thread([&backend_stub]{
                while(!shutdown_requested)
                {
                    backend_stub.run();
                }
            });
            rename_thread(backend_stub_driver, "backend-stub");
        }
        // Start web service and wait for shutdown request
        std::vector<pollfd> poll_list = {
            { rest_api.get_native_handle(), POLLIN, 0 },
            { file_api.get_native_handle(), POLLIN, 0 }
        };
#if SYSTEMD_INTEGRATION
        systemd::get_instance().notify_ready();
#endif
        wc_log(log_level_t::debug, "Enter main loop.");
        while(!shutdown_requested)
        {
            if (single_threaded_mode)
            {
                for (auto handle : frontend_stub.get_native_handles())
                {
                    poll_list.push_back({handle, POLLIN, 0});
                }
                for (auto handle : file_api_stub.get_native_handles())
                {
                    poll_list.push_back({handle, POLLIN, 0});
                }
                for (auto handle : backend_stub.get_native_handles())
                {
                    poll_list.push_back({handle, POLLIN, 0});
                }
            }

            ::poll(poll_list.data(), poll_list.size(), -1);

            if(poll_list[0].revents & (POLLIN))
            {
                rest_api.receive_next();
            }
            if(poll_list[1].revents & (POLLIN))
            {
                file_api.receive_next();
            }

            if (single_threaded_mode)
            {
                while(frontend_stub.run_once()) {;}
                while(file_api_stub.run_once()) {;}
                while(backend_stub.run_once())  {;}
                std::vector<pollfd>::size_type const skipSize = 2;
                poll_list.erase(poll_list.begin() + std::min(skipSize, poll_list.size()), poll_list.end());
            }
        }
        wago::logging::close();

        frontend_stub_driver.join();
        file_api_stub_driver.join();
        backend_stub_driver.join();

        // Reset handlers signal
        // parasoft-begin-suppress CERT_C-CON37-a "Only used to react on signals from outside this process."
        sigaction(SIGINT, nullptr, nullptr);
        sigaction(SIGTERM, nullptr, nullptr);
        // parasoft-end-suppress CERT_C-CON37-a

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
void wc_Fail(char const * const szReason, // parasoft-suppress CERT_CPP-DCL60-a-3 "Intentionally replacing other as weak marked functions"
             char const * const szFile,
             char const * const szFunction,
             int const line)
{
    string problem = szReason;
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
