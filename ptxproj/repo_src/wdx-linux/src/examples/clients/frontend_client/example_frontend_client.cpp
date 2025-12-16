//------------------------------------------------------------------------------
// Copyright (c) 2021-2024 WAGO GmbH & Co. KG
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
///  \brief    This is an example frontend client to show how to use the
///            parameter service frontend in an own executable.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/parameter_service_frontend_proxy.hpp" // From external projects you should use include via <> instead
#include <wago/logging/log.hpp>
#include "providers/example_providers.hpp"

#include <wc/log.h>
#include <wc/structuring.h>

//#include <CLI/CLI.hpp>
#include <string>
#include <memory>
#include <atomic>
#include <cassert>
#include <csignal>
#include <syslog.h>

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

void read_single_parameter(wago::wdx::linuxos::com::parameter_service_frontend_proxy       &service,
                           std::string                                      const &parameter_path)
{
    service.get_frontend().get_parameters_by_path({ wago::wdx::parameter_instance_path(parameter_path) })
        .set_notifier([](std::vector<wago::wdx::parameter_response> &&responses){
        if(responses.size() != 1)
        {
            std::cerr << "Error in parameter response!" << std::endl;
        }
        else if(responses.at(0).has_error())
        {
            std::cout << "Got negative parameter response:" << std::endl;
            std::cout << "ID:            " << wago::wdx::to_description(responses.at(0).id) << std::endl;
            std::cout << "Device:        " << responses.at(0).path.device_path << std::endl;
            std::cout << "Parameter:     " << responses.at(0).path.parameter_path << std::endl;
            std::cout << "Status:        " << wago::wdx::to_string(responses.at(0).status)
                                           << " (" << static_cast<unsigned>(responses.at(0).status) << ")" << std::endl;
            if(responses.at(0).domain_specific_status_code != 0)
            {
                std::cout << "Domain Status: " << responses.at(0).domain_specific_status_code << std::endl;
            }
            if(!responses.at(0).get_message().empty())
            {
                std::cout << "Detail:        " << responses.at(0).get_message() << std::endl;
            }
        }
        else
        {
            std::cout << "Got parameter response:" << std::endl;
            std::cout << "ID:        " << wago::wdx::to_description(responses.at(0).id) << std::endl;
            std::cout << "Device:    " << responses.at(0).path.device_path << std::endl;
            std::cout << "Parameter: " << responses.at(0).path.parameter_path << std::endl;
            std::cout << "Value:     " << responses.at(0).value->to_string() << std::endl;
        }
        shutdown_handler(0);
    });
}

void request_device_list(wago::wdx::linuxos::com::parameter_service_frontend_proxy &service)
{
    service.get_frontend().get_all_devices().set_notifier([](wago::wdx::device_collection_response &&response){
        if(response.has_error())
        {
            std::cerr << "Error in device collection response!" << std::endl;
        }
        else
        {
            std::cout << "Got device collection response:" << std::endl;
            for(auto &device : response.devices)
            {
                std::cout << "Device " << device.order_number
                          << " at position " << static_cast<unsigned>(device.id.device_collection_id) << "." << device.id.slot << std::endl;
            }
        }
        shutdown_handler(0);
    });
}
}

//------------------------------------------------------------------------------
/// Main function of example executable.
///
/// \return
///   This examples returns 0 in every case.
//------------------------------------------------------------------------------
int main(int    argc,
         char** argv)
{
    // Setup signals for shutdown request
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = shutdown_handler;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    std::string const service_name = "Example frontend client";

    // Evaluate program options
    (void)argc; (void)argv;
    unsigned verbose_count = 0;
    std::string parameter_path;
//    CLI::App client(service_name);
//    client.add_flag("-v,--verbose,--no-verbose{false}", verbose_count, "Program verbosity, may be stacked up to 3 times (-vvv)");
//    client.add_option("-r,--read", parameter_path, "Read single parameter value from given parameter path");
//    CLI11_PARSE(client, argc, argv);
    verbose_count = (verbose_count > 3)? 3 : verbose_count;

    // Setup parameter service
    wago::logging::set_log_function(wago::logging::log_stdout);
    wago::logging::set_log_level(static_cast<log_level_t>((static_cast<unsigned>(log_level_t::error) + verbose_count)));
    wago::wdx::linuxos::com::parameter_service_frontend_proxy service(service_name);
    driver = &service;

    if(!parameter_path.empty())
    {
        read_single_parameter(service, parameter_path);
    }
    else
    {
        request_device_list(service);
    }

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
