//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of file parameter handler for file parameters.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/file/file_parameter_handler.hpp"
#include "wago/wdx/linuxos/file/exception.hpp"

#include <wago/wdx/parameter_service_backend_i.hpp>
#include <wc/assertion.h>
#include <wc/log.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {

//------------------------------------------------------------------------------
// internal function declaration
//------------------------------------------------------------------------------
namespace {
file::exception create_exception_from_ptr(std::exception_ptr        e_ptr,
                                          std::string        const &error_message_prefix);
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
file_parameter_handler::state::state(wdx::parameter_id_t                  const  file_parameter_id,
                                     wdx::parameter_service_backend_i           &backend,
                                     std::function<file_provider_factory>        provider_factory)
: file_parameter_id_m(file_parameter_id)
, backend_m(backend)
, provider_factory_m(provider_factory)
{ }

file_parameter_handler::file_parameter_handler(wdx::parameter_id_t                  const  file_parameter_id,
                                               wdx::parameter_service_backend_i           &backend,
                                               std::function<file_provider_factory>        provider_factory)
: state_m(std::make_shared<state>(file_parameter_id, backend, std::move(provider_factory)))
{
    // Initial provider for read access
    std::string const initial_content = "Initial file content";
    state_m->read_provider_m = state_m->provider_factory_m(true);
    auto future_register_response = state_m->backend_m.register_file_provider(state_m->read_provider_m.get(), state_m->file_parameter_id_m);
    future_register_response.set_notifier([state = state_m](wdx::register_file_provider_response register_response){
        std::lock_guard<std::mutex> read_provider_lock(state->read_provider_mutex_m);
        if(register_response.has_error())
        {
            state->read_provider_m.reset();
            std::string error_message = "Failed to register file provider for read purposes, code "
                                      + wdx::to_string(register_response.status);
            wc_log(log_level::error, error_message.c_str());
        }
        else
        {
            state->read_file_id_m = wdx::parameter_value::create_file_id(register_response.registered_file_id);
        }
        for(auto &notifier : state->read_file_id_notifier_m)
        {
            notifier(state->read_file_id_m);
        }
        state->read_file_id_notifier_m.clear();
    });
    future_register_response.set_exception_notifier([state = state_m](std::exception_ptr e_ptr){
        auto exception = create_exception_from_ptr(e_ptr, "Failed to register file provider for read purposes: ");
        wc_log(log_level::error, exception.what());

        std::lock_guard<std::mutex> read_provider_lock(state->read_provider_mutex_m);
        for(auto &notifier : state->read_file_id_notifier_m)
        {
            notifier(file_id_value());
        }
        state->read_file_id_notifier_m.clear();
    });
}

file_parameter_handler::~file_parameter_handler() noexcept
{
    // Destructor should not throw!
    try
    {
        std::lock_guard<std::mutex> write_provider_lock(state_m->write_provider_mutex_m);
        std::lock_guard<std::mutex> read_provider_lock(state_m->read_provider_mutex_m);
        if(state_m->read_provider_m.get() != nullptr)
        {
            state_m->backend_m.unregister_file_provider(state_m->read_provider_m.get());
            state_m->read_file_id_m.reset();
        }
        if(state_m->write_provider_m.get() != nullptr)
        {
            state_m->backend_m.unregister_file_provider(state_m->write_provider_m.get());
            state_m->write_file_id_m.reset();
        }
    }
    catch(std::exception const &e)
    {
        std::string error_message = "Caught exception while destructing file parameter handler for parameter ID "
                                  + std::to_string(state_m->file_parameter_id_m) + ": " + e.what();
        wc_log(log_level::warning, error_message.c_str());
    }
    catch(...)
    {
        WC_FAIL(("Caught something other than an exception while destructing file parameter handler for parameter ID "
               + std::to_string(state_m->file_parameter_id_m)).c_str());
    }
}

future<file_id_value> file_parameter_handler::get_file_id()
{
    std::lock_guard<std::mutex> read_provider_lock(state_m->read_provider_mutex_m);

    if(state_m->read_file_id_m.get() != nullptr)
    {
        auto result = state_m->read_file_id_m;
        return resolved_future(std::move(result));
    }
    else if(state_m->read_provider_m.get() == nullptr)
    {
        std::string const error_message = "Failed to register file provider for read purposes";
        return resolved_future<file_id_value>(std::make_exception_ptr(wdx::linuxos::file::exception(wdx::status_codes::internal_error, error_message)));
    }

    // Wait for read provider registration result
    auto result_promise = std::make_shared<promise<std::shared_ptr<wdx::parameter_value>>>();
    state_m->read_file_id_notifier_m.emplace_back([result_promise](file_id_value id_value){
        if(id_value.get() != nullptr)
        {
            result_promise->set_value(std::move(id_value));
        }
        else
        {
            std::string const error_message = "Failed to register file provider for read purposes";
            result_promise->set_exception(std::make_exception_ptr(wdx::linuxos::file::exception(wdx::status_codes::internal_error, error_message)));
        }
    });

    return result_promise->get_future();
}

future<file_id_value> file_parameter_handler::create_file_id_for_write(wdx::parameter_id_t const context)
{
    auto response_promise = std::make_shared<promise<std::shared_ptr<wdx::parameter_value>>>();
    std::unique_lock<std::mutex> write_provider_lock(state_m->write_provider_mutex_m);

    // Unregister existing file provider for write operations
    if(state_m->write_provider_m.get() != nullptr)
    {
        state_m->backend_m.unregister_file_provider(state_m->write_provider_m.get());
    }
    state_m->write_provider_m = state_m->provider_factory_m(false);
    auto future_register_response = state_m->backend_m.register_file_provider(state_m->write_provider_m.get(), context);
    write_provider_lock.unlock(); // Avoid self-lock in case notifier is called immediately
    future_register_response.set_notifier([response_promise, state = state_m](wdx::register_file_provider_response register_response){
        if(register_response.has_error())
        {
            std::string error_message = "Error while creating file ID: Failed to register file provider for write purposes";
            wc_log(log_level::error, error_message.c_str());
            response_promise->set_exception(std::make_exception_ptr(wdx::linuxos::file::exception(wdx::status_codes::internal_error, error_message)));
        }

        std::lock_guard<std::mutex> write_id_lock(state->write_provider_mutex_m);
        state->write_file_id_m = wdx::parameter_value::create_file_id(register_response.registered_file_id);
        auto result = state->write_file_id_m;
        return response_promise->set_value(std::move(result));
    });
    future_register_response.set_exception_notifier([response_promise](std::exception_ptr e_ptr){
        auto exception = create_exception_from_ptr(e_ptr, "Error while creating file ID: Failed to register file provider for write purposes: ");
        wc_log(log_level::error, exception.what());
        response_promise->set_exception(std::make_exception_ptr(exception));
    });

    return response_promise->get_future();
}

void file_parameter_handler::remove_file_id_for_write(wdx::file_id const id)
{
    std::lock_guard<std::mutex> write_provider_lock(state_m->write_provider_mutex_m);
    if(state_m->write_file_id_m.get() == nullptr)
    {
        std::string error_message = "No write operation was initiated before";
        throw wdx::linuxos::file::exception(wdx::status_codes::logic_error, error_message);
    }
    else if(id != state_m->write_file_id_m->get_file_id())
    {
        std::string error_message = "Given file ID for write is not the currently active write file ID";
        throw wdx::linuxos::file::exception(wdx::status_codes::file_id_mismatch, error_message);
    }

    state_m->backend_m.unregister_file_provider(state_m->write_provider_m.get());
    state_m->write_provider_m.reset();
    state_m->write_file_id_m.reset();
}

future<void> file_parameter_handler::set_file_id(file_id_value  value,
                                                 file_validator validator)
{
    promise<void> set_promise;
    try
    {
        std::lock_guard<std::mutex> write_provider_lock(state_m->write_provider_mutex_m);
        if(state_m->write_file_id_m.get() == nullptr)
        {
            std::string error_message = "No write operation was initiated before";
            throw wdx::linuxos::file::exception(wdx::status_codes::logic_error, error_message);
        }
        else if(value->get_file_id() != state_m->write_file_id_m->get_file_id())
        {
            std::string error_message = "Given file ID for write is not the currently active write file ID";
            throw wdx::linuxos::file::exception(wdx::status_codes::file_id_mismatch, error_message);
        }
        // TODO: Call validation async
        else if(!state_m->write_provider_m->is_complete().get())
        {
            std::string error_message = "Written data for file ID " + state_m->write_file_id_m->get_file_id() + " is not complete";
            throw wdx::linuxos::file::exception(wdx::status_codes::invalid_value, error_message);
        }
        // TODO: Call validation async
        try
        {
            state_m->write_provider_m->validate(validator).get();
        }
        catch(std::exception const &e)
        {
            std::string error_message = "Failed to validate written data for file ID " + state_m->write_file_id_m->get_file_id()
                                        + ": " + e.what();
            wc_log(log_level::warning, error_message.c_str());
            throw wdx::linuxos::file::exception(wdx::status_codes::invalid_value, error_message);
        }
        // TODO: Call finish async
        try
        {
            state_m->write_provider_m->finish().get();
        }
        catch(std::exception const &e)
        {
            std::string error_message = "Failed to finally finish write for file ID " + state_m->write_file_id_m->get_file_id()
                                            + ": " + e.what();
            wc_log(log_level::warning, error_message.c_str());
            throw wdx::linuxos::file::exception(wdx::status_codes::internal_error, error_message);
        }

        std::lock_guard<std::mutex> read_provider_lock(state_m->read_provider_mutex_m);
        state_m->backend_m.unregister_file_provider(state_m->read_provider_m.get());
        state_m->read_file_id_m  = std::move(state_m->write_file_id_m);
        state_m->read_provider_m = std::move(state_m->write_provider_m);
        set_promise.set_value();
    }
    catch(std::exception const &)
    {
        set_promise.set_exception(std::current_exception());
    }

    return set_promise.get_future();
}

//------------------------------------------------------------------------------
// internal function implementation
//------------------------------------------------------------------------------
namespace {
wdx::linuxos::file::exception create_exception_from_ptr(std::exception_ptr        e_ptr,
                                                        std::string        const &error_message_prefix)
{
    try
    {
        if(e_ptr)
        {
            std::rethrow_exception(e_ptr);
        }
        throw std::runtime_error("Ask to get exception message from nullptr");
    }
    catch(std::exception const &e)
    {
        std::string error_message = error_message_prefix + e.what();
        return wdx::linuxos::file::exception(wdx::status_codes::internal_error, error_message);
    }
}
}


} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago



//---- End of source file ------------------------------------------------------
