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
//------------------------------------------------------------------------------
#include "serial_file_provider.hpp"
#include "utils/provider_job.hpp"

namespace wago {
namespace wdx {
inline namespace file_transfer{

serial_file_provider::serial_file_provider(file_provider_i* wrapped_provider_)
: wrapped_provider(wrapped_provider_)
{}

serial_file_provider::~serial_file_provider() noexcept = default;

wago::future<file_read_response> serial_file_provider::read(uint64_t offset, size_t length)
{
    auto promise = std::make_shared<wago::promise<file_read_response>>();

    using read_job = provider_job<file_read_response>;
    auto job = std::make_shared<read_job>(promise, [provider=wrapped_provider, offset, length] () {
        return provider->read(offset, length);
    });
    queue.enqueue_job(job);

    return promise->get_future();
}
    
wago::future<response> serial_file_provider::write(uint64_t offset, std::vector<uint8_t> data)
{
    auto promise = std::make_shared<wago::promise<response>>();

    using write_job = provider_job<response>;
    auto job = std::make_shared<write_job>(promise, [provider=wrapped_provider, offset, data=std::move(data)] () {
        return provider->write(offset, std::move(data));
    });
    queue.enqueue_job(job);

    return promise->get_future();
}

    wago::future<file_info_response> serial_file_provider::get_file_info()
    {
        auto promise = std::make_shared<wago::promise<file_info_response>>();
    
        using info_job = provider_job<file_info_response>;
        auto job = std::make_shared<info_job>(promise, [provider=wrapped_provider] () {
            return provider->get_file_info();
        });
        queue.enqueue_job(job);
 
        return promise->get_future();
    }

    wago::future<response> serial_file_provider::create(uint64_t capacity)
    {
        auto promise = std::make_shared<wago::promise<response>>();
    
        using create_job = provider_job<response>;
        auto job = std::make_shared<create_job>(promise, [provider=wrapped_provider, capacity] () {
            return provider->create(capacity);
        });
        queue.enqueue_job(job);
 
        return promise->get_future();
    }

}}}
