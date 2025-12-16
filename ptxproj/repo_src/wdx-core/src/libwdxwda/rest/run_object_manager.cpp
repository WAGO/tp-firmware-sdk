//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of manager to handle run objects.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "run_object_manager.hpp"
#include "http/http_exception.hpp"

#include <wc/assertion.h>

#include <chrono>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace {
    uint64_t    get_timestamp_seconds(uint64_t since = 0) noexcept;
    std::string base36_encode(uint64_t number);
    // FIXME: can probably be deleted, because we don't use it
    //uint64_t    base36_decode(std::string const &str);
    uint64_t    next_method_invocation_id_g = get_timestamp_seconds(1577836800ull); // timestamp since 01.01.2020
    std::string generate_run_id();
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
uint32_t method_run_object::get_timeout_left() const
{
    if(timeout_time == UINT64_MAX)
    {
        return timeout_span;
    }

    uint64_t now          = get_timestamp_seconds();
    uint64_t timeout_left = (timeout_time > now) ? timeout_time - now : 0;
    WC_ASSERT_RETURN(timeout_left <= UINT16_MAX, UINT32_MAX);

    return static_cast<uint32_t>(timeout_left);
}

std::string method_run_object::get_id() const
{
    return id;
}

std::string run_object_manager::add_run(wdx::parameter_instance_path          const  &method_instance_path,
                                        uint16_t                                      timeout_span,
                                        wdx::method_invocation_named_response       &&response)
{
    uint64_t now          = get_timestamp_seconds();
    uint64_t timeout_time = now + timeout_span;

    std::lock_guard<std::mutex> guard(change_mutex_m);
    auto run_id     = generate_run_id();
    auto run_object = std::make_shared<method_run_object>(run_id, method_instance_path, timeout_span, timeout_time, std::move(response));
    runs_map_m.emplace(run_id, run_object);

    return run_id;
}

std::string run_object_manager::add_run(wdx::parameter_instance_path                  const  &method_instance_path,
                                        uint16_t                                              timeout_span,
                                        future<wdx::method_invocation_named_response>       &&invoke_future,
                                        ready_handler                                         handler)
{
    std::string                        run_id;
    std::shared_ptr<method_run_object> run_object;
    {
        std::lock_guard<std::mutex> guard(change_mutex_m);
        run_id     = generate_run_id();
        run_object = std::make_shared<method_run_object>(run_id, method_instance_path, timeout_span);
        runs_map_m.emplace(run_id, run_object);
    }

    invoke_future.set_notifier([run_id, run_object, handler](wdx::method_invocation_named_response &&response){
        uint64_t now                    = get_timestamp_seconds();
        run_object->timeout_time        = now + run_object->timeout_span;
        run_object->invocation_response = std::make_shared<wdx::method_invocation_named_response>(std::move(response));
        handler(run_id);
    });
    invoke_future.set_exception_notifier([run_id, run_object, handler](auto e_ptr){
        WC_FAIL("No exception is expected on core frontend calls");
        try
        {
            if(e_ptr)
            {
                std::rethrow_exception(e_ptr);
            }
        }
        catch(std::exception &e)
        {
            uint64_t now                    = get_timestamp_seconds();
            run_object->timeout_time        = now + run_object->timeout_span;
            run_object->invocation_response = std::make_shared<wdx::method_invocation_named_response>(wdx::status_codes::internal_error, e.what());
        }
        handler(run_id);
    });

    return run_id;
}

std::shared_ptr<method_run_object> run_object_manager::get_run(wdx::parameter_instance_path const &method_instance_path,
                                                               std::string                         run_id)
{
    std::lock_guard<std::mutex> guard(change_mutex_m);

    auto iterator = runs_map_m.find(run_id);
    if(iterator == runs_map_m.end() || (iterator->second->method != method_instance_path))
    {
        throw http::http_exception("Run object not found.", http::http_status_code::not_found);
    }
    return iterator->second;
}

std::vector<std::shared_ptr<method_run_object>> run_object_manager::get_runs(wdx::parameter_instance_path const &method_instance_path)
{
    std::lock_guard<std::mutex> guard(change_mutex_m);

    // collect matching runs by method_instance_path
    std::vector<std::shared_ptr<method_run_object>> runs;
    for (auto const &run : runs_map_m)
    {
        if (run.second->method == method_instance_path)
        {
            runs.push_back(run.second);
        }
    }
    return runs;
}

void run_object_manager::remove_run(wdx::parameter_instance_path const &method_instance_path,
                                    std::string                         run_id)
{
    std::lock_guard<std::mutex> guard(change_mutex_m);

    auto iterator = runs_map_m.find(run_id);
    if(iterator == runs_map_m.end() || (iterator->second->method != method_instance_path))
    {
        throw http::http_exception("Run object not found.", http::http_status_code::not_found);
    }

    runs_map_m.erase(run_id);
}

void run_object_manager::clean_runs()
{
    uint64_t const cleanup_time = get_timestamp_seconds();

    std::lock_guard<std::mutex> guard(change_mutex_m);

    for(auto iterator = runs_map_m.begin(); iterator != runs_map_m.end();)
    {
        if(iterator->second->timeout_time <= cleanup_time)
        {
            iterator = runs_map_m.erase(iterator);
        }
        else
        {
            ++iterator;
        }
    }
}

bool run_object_manager::max_runs_reached()
{
    std::lock_guard<std::mutex> guard(change_mutex_m);
    return (runs_map_m.size() >= max_runs);
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
namespace {
    uint64_t get_timestamp_seconds(uint64_t since) noexcept
    {
        auto time_since_epoch = std::chrono::steady_clock::now().time_since_epoch();
        auto seconds = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(time_since_epoch).count());
        return since > seconds ? 0 : (seconds - since);
    }

    std::string base36_encode(uint64_t number)
    {
        std::string str;
        do
        {
            uint64_t remainder_value = number % 36; // uint64_t to avoid compiler warning
            char digit;
            if(remainder_value < 10)
            {
                digit = char(remainder_value + '0');
            }
            else
            {
                digit = char((remainder_value - 10) + 'a');
            }
            str = digit + str;
        }
        while ((number /= 36) > 0); // parasoft-suppress CERT_C-EXP45-d-2 "Compliant solution - Intentional assignment. Parasoft detects a false-positive here."

        return str;
    }

    // FIXME: can probably be deleted, because we don't use it
    // 
    // uint64_t base36_decode(std::string const &str)
    // {
    //     uint64_t result = 0;
    //     for (char c : str)
    //     {
    //         if (c > 'a')
    //         {
    //             c -= 10; // there are 10 numeric characters 0-9
    //         }
    //         if (c > 'z')
    //         {
    //             throw http::http_exception("Unknown method invocation ID.",
    //                                        http::http_status_code::not_found);
    //         }
    //         int digit = c - '0';
            
    //         result *= 36;
    //         result += digit;
    //     }
    //     return result;
    // }

    std::string generate_run_id()
    {
        return base36_encode(next_method_invocation_id_g++);
    }
}


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
