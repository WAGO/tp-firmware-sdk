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
///  \brief    Boost ASIO testclient send/receive some simple requests.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/adapter_i.hpp"
#include "common/receiver_i.hpp"
#include "asio/asio_client.hpp"

#include <wc/structuring.h>
#include <wc/assertion.h>
#include <wc/log.h>

#include <memory>
#include <iostream>
#include <csignal>
#include <cassert>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
std::atomic<bool> shutdown_requested(false);

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
void myhandler(int WC_UNUSED_PARAM(arg))
{
    shutdown_requested = true;
}


using wago::wdx::linuxos::com::adapter_i;
using wago::wdx::linuxos::com::receiver_i;
using wago::wdx::linuxos::com::message_data;
using wago::wdx::linuxos::com::asio_client;
using wago::wdx::linuxos::com::connection_handler;

class my_echo_client : public receiver_i
{
private:
    std::unique_ptr<adapter_i> adapter_m;

public:
    my_echo_client(std::unique_ptr<adapter_i> adapter)
    : adapter_m(std::move(adapter))
    {

    }

    ~my_echo_client() noexcept override
    {

    }

    void send(message_data &&message)
    {
        adapter_m->send(std::move(message), [](std::string error_message){
            if(!error_message.empty())
            {
                throw std::runtime_error("Failed to send message: " + error_message + "  --> Terminate now...");
            }
            else
            {
                std::cout << "Message was send, we should receive an echo..." << std::endl;
            }
        });
    }

    void receive(message_data message) override
    {
        std::cout << "Got my echo of " << message.size() << " bytes" << std::endl;
    }
};
}

int main(int WC_UNUSED_PARAM(argc), char ** WC_UNUSED_PARAM(argv))
{
    std::cout << "Testclient started" << std::endl;

    // Generate some trash to fill messages with
    size_t const trash_size = 5 * 1024 * 1024;
    message_data message_trash;
    for(size_t i = 0; i < trash_size; ++i)
    {
        message_trash.push_back(static_cast<uint8_t>(i % (UINT8_MAX + 1)));
    }

    char const socket_path[] = "/tmp/wdx.asio.test.socket";
    asio_client client("ASIO test client", socket_path);

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = myhandler;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    // Prepare to receive incoming messages
    connection_handler<my_echo_client> handler = [](std::unique_ptr<adapter_i> adapter){
        auto echo_client = std::make_unique<my_echo_client>(std::move(adapter));
        return echo_client;
    };
    auto &echo_client = client.connect(handler, nullptr);

    // Prepare test messages
    constexpr uint8_t const message_count = 5;
    message_data messages[message_count];
    for(uint8_t i = 0; i < message_count; ++i)
    {
        message_trash.insert(message_trash.begin(), i);
        messages[i] = message_trash;
    }

    // Send some test messages
    bool const use_threads = true;
    for(uint8_t i = 0; i < message_count; ++i)
    {
        message_data message = messages[i];
        auto send_task = [&echo_client, message = std::move(message)]() mutable {
            echo_client.send(std::move(message));
            return;
        };
        if(use_threads)
        {
            std::thread new_task(send_task);
            new_task.detach();
        }
        else
        {
            send_task();
        }
    }

    while(!shutdown_requested)
    {
        bool const something_done = client.run_once(100);
        WC_UNUSED_DATA(something_done);
        //std::cout << "ASIO client did " << (something_done ? "something" : "nothing") << std::endl;
    }
}

void wc_log_output(log_level_t  const WC_UNUSED_PARAM(log_level),
                   char const * const message) noexcept
{
    std::cout << "Log: " << message << std::endl;
}

log_level_t wc_get_log_level() noexcept
{
    return log_level_t::debug;
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
    std::cerr << problem << std::endl;

    // Trigger standard assert
    assert(false);
}
GNUC_DIAGNOSTIC_POP


//---- End of source file ------------------------------------------------------
