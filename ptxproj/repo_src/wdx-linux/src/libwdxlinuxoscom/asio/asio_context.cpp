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
///  \brief    Implementation of a Boost ASIO communication context (real IPC)
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "asio_context.hpp"

#include <wc/assertion.h>
#include <wc/log.h>

#include <algorithm>
#include <exception>
#include <string>
#include <unistd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
asio_context::asio_context(std::shared_ptr<io_context> context)
: context_m(std::move(context))
{ }

bool asio_context::run_once(uint32_t timeout_ms) // NOLINT(google-default-arguments)
{
    try
    {
        if(context_m->stopped())
        {
            context_m->restart();
        }
        if(timeout_ms > 0)
        {
            std::chrono::milliseconds timeout(timeout_ms);
            bool result = context_m->run_one_for(timeout) > 0;
            uint32_t try_again_time_ms = timeout_ms;
            while(!result && context_m->stopped() && (try_again_time_ms > 0))
            {
                // Sleep max. 100 ms to avoid long blocking times
                uint32_t sleeptime_ms = std::min(try_again_time_ms, 100U);
                usleep(sleeptime_ms * 1000); // parasoft-suppress CERT_CPP-ERR59-a-1 "usleep does not throw"
                try_again_time_ms -= sleeptime_ms;
                result = run_once();
            }
            return result;
        }
        return context_m->poll_one() > 0;
    }
    catch(std::exception &e)
    {
        std::string const error_message = std::string(SERVICE_LOG_PREFIX "Caught unhandled exception on run once: ") + e.what();
        WC_FAIL(error_message.c_str());
        wc_log(error, error_message.c_str());
    }
    return false;
}

bool asio_context::run()
{
    try
    {
        if(context_m->stopped())
        {
            context_m->restart();
        }
        return context_m->run() > 0;
    }
    catch(std::exception &e)
    {
        std::string const error_message = std::string(SERVICE_LOG_PREFIX "Caught unhandled exception on run: ") + e.what();
        WC_FAIL(error_message.c_str());
        wc_log(error, error_message.c_str());
    }
    return false;
}

void asio_context::stop()
{
    context_m->stop();
}

void asio_context::on_receive(message_data                  const &message,
                              std::string                   const &error_message,
                              std::shared_ptr<asio_adapter>        adapter)       // NOLINT(performance-unnecessary-value-param)
{
    if(!error_message.empty())
    {
        wc_log(info, (SERVICE_LOG_PREFIX "Disconnected after: " + error_message).c_str());
        on_disconnect(adapter);
    }
    else
    {
        WC_DEBUG_LOG(SERVICE_LOG_PREFIX "Prepare to receive new message");
        adapter->receive([this, adapter](message_data const &new_message, std::string new_error_message){
            on_receive(new_message, new_error_message, adapter);
        });

        // handle incoming message
        auto receiver = get_receiver(adapter);
        receiver->receive(message);
    }
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
