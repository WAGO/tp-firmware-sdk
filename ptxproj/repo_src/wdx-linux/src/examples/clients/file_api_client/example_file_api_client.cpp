//------------------------------------------------------------------------------
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
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
///  \brief    This is an example file api client to show how to use the
///            parameter service file api in an own executable.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/parameter_service_file_api_proxy.hpp" // From external projects you should use include via <> instead
#include <wago/logging/log.hpp>

#include <wc/assertion.h>
#include <wc/structuring.h>
#include <wc/log.h>

#include <string>
#include <memory>
#include <atomic>
#include <utility>
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
    if(driver != nullptr)
    {
        driver.load()->stop();
    }
}

void read_file(wago::wdx::linuxos::com::parameter_service_file_api_proxy       &service,
               wago::wdx::file_id                               const &file_id,
               uint64_t                                                file_offset,
               size_t                                                  file_length,
               bool                                                    verbose)
{
    WC_ASSERT(file_length <= std::numeric_limits<ssize_t>::max());
    auto future = service.get_file_api().file_read(file_id, file_offset, file_length);
    future.set_exception_notifier([](std::exception_ptr e_ptr) {
        try
        {
            if(e_ptr)
            {
                std::rethrow_exception(e_ptr);
            }
        }
        catch (std::exception const &ex)
        {
            std::cerr << "Exception during file_read: " << ex.what() << std::endl;
        }
    });
    future.set_notifier([verbose] (wago::wdx::file_read_response &&read_response) {
            if(read_response.has_error())
            {
                std::cerr << "Error in file_read_response: " << wago::wdx::to_string(read_response.status) << std::endl;
            }
            else
            {
                if(verbose)
                {
                    std::cout << "Successfully read " << std::to_string(read_response.data.size()) + " bytes from file: " << std::endl;
                    std::cout << "--- content begins ---" << std::endl;
                }
                std::cout.write(reinterpret_cast<char const *>(read_response.data.data()), static_cast<ssize_t>(read_response.data.size()));
                if(verbose)
                {
                    std::cout << std::endl << "--- content ends ---" << std::endl;
                }
            }
            shutdown_handler(0);
        });
}

void write_file(wago::wdx::linuxos::com::parameter_service_file_api_proxy       &service,
                wago::wdx::file_id                               const &file_id,
                uint64_t                                                file_offset,
                std::vector<uint8_t>                                    file_data)
{
    std::cout << "Will write " << std::to_string(file_data.size()) + " bytes to file." << std::endl;

    auto file_size = file_data.size();
    auto future = service.get_file_api().file_write(file_id, file_offset, std::move(file_data));
    future.set_exception_notifier([](std::exception_ptr e_ptr) {
        try
        {
            if(e_ptr)
            {
                std::rethrow_exception(e_ptr);
            }
        }
        catch (std::exception const &ex)
        {
            std::cerr << "Exception during file_write: " << ex.what() << std::endl;
        }
    });
    future.set_notifier([file_size] (wago::wdx::response &&write_response) {
            if(write_response.has_error())
            {
                std::cerr << "Error in file_write_response: " << wago::wdx::to_string(write_response.status) << std::endl;
            }
            else
            {
                std::cout << "Successfully written " << std::to_string(file_size) + " bytes" << std::endl;
            }
            shutdown_handler(0);
        });
}

void create_file(wago::wdx::linuxos::com::parameter_service_file_api_proxy       &service,
                 wago::wdx::file_id                               const &file_id,
                 uint64_t                                                file_capacity)
{
    auto future = service.get_file_api().file_create(file_id, file_capacity);
    future.set_exception_notifier([](std::exception_ptr e_ptr) {
        try
        {
            if(e_ptr)
            {
                std::rethrow_exception(e_ptr);
            }
        }
        catch (std::exception const &ex)
        {
            std::cerr << "Exception during file_create: " << ex.what() << std::endl;
        }
    });
    future.set_notifier([file_capacity] (wago::wdx::response &&create_response) {
            if(create_response.has_error())
            {
                std::cerr << "Error in file_create_response: " << wago::wdx::to_string(create_response.status) << std::endl;
            }
            else
            {
                std::cout << "Successfully created file with capacity of " << std::to_string(file_capacity) + " bytes" << std::endl;
            }
            shutdown_handler(0);
        });
}

void get_file_info(wago::wdx::linuxos::com::parameter_service_file_api_proxy       &service,
                   wago::wdx::file_id                               const &file_id)
{
    auto future = service.get_file_api().file_get_info(file_id);
    future.set_exception_notifier([](std::exception_ptr e_ptr) {
        try 
        {
            if(e_ptr)
            {
                std::rethrow_exception(e_ptr);
            }
        }
        catch (std::exception const &ex)
        {
            std::cerr << "Exception during file_get_info: " << ex.what() << std::endl;
        }
    });
    future.set_notifier([] (wago::wdx::file_info_response &&info_response) {
            if(info_response.has_error())
            {
                std::cerr << "Error in file_info_response: " << wago::wdx::to_string(info_response.status) << std::endl;
            }
            else
            {
                std::cout << "Successfully got file info with file size of " << std::to_string(info_response.file_size) + " bytes" << std::endl;
            }
            shutdown_handler(0);
        });
}

void print_usage(std::ostream       &out,
                 std::string  const &program)
{
    out << "usage: " << program << " <command>" << std::endl;
    out << "  commands: " << " read   <file-id> <offset> <length>" << std::endl;
    out << "            " << " write  <file-id> <offset>  (data to write will be read from stdin)" << std::endl;
    out << "            " << " info   <file-id>" << std::endl;
    out << "            " << " create <file-id> <capacity>" << std::endl;
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
    if (argc < 2)
    {
        print_usage(std::cerr, argv[0]);
        exit(1);
    }

    // Setup signals for shutdown request
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = shutdown_handler;
    sigaction(SIGINT, &act, nullptr);
    sigaction(SIGTERM, &act, nullptr);

    std::string const service_name = "Example file API client";

    // Evaluate program options
    // TODO: Currently no verbose flag is available in program options
    unsigned verbose_count = 0;
    std::string parameter_path;
    verbose_count = (verbose_count > 3) ? 3 : verbose_count;

    // Setup parameter service
    wago::logging::set_log_function(wago::logging::log_stdout);
    wago::logging::set_log_level(static_cast<log_level_t>((static_cast<unsigned>(log_level_t::error) + verbose_count)));
    wago::wdx::linuxos::com::parameter_service_file_api_proxy service(service_name);
    driver = &service;

    std::string command = argv[1];
    if (command == "read" && (argc > 4))
    {
        std::string const file_id     = argv[2];
        uint64_t    const file_offset = std::strtoull(argv[3], nullptr, 10);
        uint32_t    const file_length = std::strtoul(argv[4], nullptr, 10);
        read_file(service, file_id, file_offset, file_length, (verbose_count > 0));
    }
    else if (command == "write" && (argc > 3))
    {
        std::string const file_id     = argv[2];
        uint64_t    const file_offset = std::strtoull(argv[3], nullptr, 10);

        std::vector<uint8_t> file_data;

        uint8_t buffer[1024];
        size_t buffer_size = sizeof(buffer);
        WC_ASSERT(buffer_size <= std::numeric_limits<ssize_t>::max());
        do
        {
            std::cin.read(reinterpret_cast<char*>(buffer), static_cast<ssize_t>(buffer_size));
            file_data.insert(file_data.end(), buffer, buffer + std::cin.gcount());
        } 
        while(std::cin);

        write_file(service, file_id, file_offset, file_data);
    }
    else if (command == "create" && (argc > 3))
    {
        std::string const file_id       = argv[2];
        uint64_t    const file_capacity = std::strtoull(argv[3], nullptr, 10);
        create_file(service, file_id, file_capacity);
    }
    else if (command == "info" && (argc > 2))
    {
        std::string const file_id = argv[2];
        get_file_info(service, file_id);
    }
    else
    {
        print_usage(std::cerr, argv[0]);
        exit(1);
    }

    // Start service and wait for shutdown request
    while(!shutdown_requested)
    {
        service.run();
        // Alternatively use single steps:
//        bool const something_done = service.run_once(1000);
//        if(something_done)
//        {
//            wc_log(log_level_t::info, "One task done!");
//        }
//        else
//        {
//            wc_log(log_level_t::info, "Nothing to do within the last second...");
//        }
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
