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

#ifndef SRC_LIBWDXCORE_FILE_TRANSFER_SERIAL_FILE_PROVIDER_HPP_
#define SRC_LIBWDXCORE_FILE_TRANSFER_SERIAL_FILE_PROVIDER_HPP_

#include "wago/wdx/file_transfer/file_provider_i.hpp"
#include "utils/job_queue.hpp"

namespace wago {
namespace wdx {
inline namespace file_transfer {

class job;

/**
Wraps a `file_provider_i`-Implementation so that its methods are not called in parallel.
 */
class serial_file_provider : public file_provider_i
{
public:
    serial_file_provider(serial_file_provider const &) noexcept = default;
    serial_file_provider &operator=(serial_file_provider const &) noexcept = default;

    serial_file_provider(file_provider_i* wrapped_provider);
    ~serial_file_provider() noexcept override;

    wago::future<file_read_response> read(uint64_t offset, size_t length) override;
    wago::future<response> write(uint64_t offset, std::vector<uint8_t> data) override;
    wago::future<file_info_response> get_file_info() override;
    wago::future<response> create(uint64_t capacity) override;

private:
    file_provider_i* wrapped_provider;
    job_queue queue;
};

}}}

#endif // SRC_LIBWDXCORE_FILE_TRANSFER_SERIAL_FILE_PROVIDER_HPP_
