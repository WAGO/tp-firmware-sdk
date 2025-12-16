//------------------------------------------------------------------------------
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    This is an example backend client to show how to use the
///            parameter service backend in an own executable.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/parameter_service_backend_proxy.hpp" // From external projects you should use include via <> instead
#include "wago/logging/log.hpp" // From external projects you should use include via <> instead
#include "providers/example_providers.hpp"

#include <wc/log.h>
#include <wc/structuring.h>

#include <string>
#include <memory>
#include <atomic>
#include <cassert>
#include <csignal>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

namespace {
using wago::wdx::linuxos::com::driver_i;
std::atomic<bool>       shutdown_requested(false);
std::atomic<driver_i *> driver(nullptr);

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
void shutdown_handler(int WC_UNUSED_PARAM(arg))
{
    // this will break the main loop
    shutdown_requested = true;
    if (driver)
    {
        driver.load()->stop();
    }
}
}

//------------------------------------------------------------------------------
/// Main function of example executable.
///
/// \return
///   This examples returns 0 in every case.
//------------------------------------------------------------------------------
int main(int    WC_UNUSED_PARAM(argc),
         char** WC_UNUSED_PARAM(argv))
{
    // Setup signals for shutdown request
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = shutdown_handler;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    std::string const service_name = "Example backend client";

    // Setup syslog log functionality
    wago::logging::init(service_name.c_str());
    wago::logging::set_log_function(wago::logging::log_syslog);
    wago::logging::set_log_level(log_level_t::warning);

    // Setup parameter service
    wago::wdx::linuxos::com::parameter_service_backend_proxy service(service_name);
    driver = &service;

    // Setup example providers
    std::unique_ptr<wago::wdx::examples::example_providers> providers(nullptr);
    service.get_notification_manager().add_notifier([&providers](wago::wdx::parameter_service_backend_i &backend){
        if(!providers)
        {
            // Create and register providers only on first connection, skip on re-connects
            providers = std::make_unique<wago::wdx::examples::example_providers>(backend);
        }
    });

    // Start service and wait for shutdown request
    while(!shutdown_requested)
    {
        bool const something_done = service.run_once(1000);
        if(something_done)
        {
            wc_log(log_level_t::info, "One task done!");
        }
        else
        {
            wc_log(log_level_t::info, "Nothing to do within the last second...");
        }
        // Alternatively simply use: service.run();
    }

    // Close syslog log functionality
    wago::logging::close();

    return 0;
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
    wc_log(fatal, problem.c_str());

    // Trigger standard assert
    assert(false);
}
GNUC_DIAGNOSTIC_POP


//---- End of source file ------------------------------------------------------
