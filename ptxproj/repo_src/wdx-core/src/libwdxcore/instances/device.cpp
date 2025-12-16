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
#include "device.hpp"
#include "wago/wdx/parameter_exception.hpp"
#include "loader/device_description_loader.hpp"

#include <wc/log.h>

namespace wago {
namespace wdx {

device::device(device_id const &id_, std::string const &order_number_, std::string const &firmware_version_)
    : id(id_)
    , m_order_number(order_number_)
    , m_firmware_version(firmware_version_)
{
}

std::string device::get_order_number() const
{
    return m_order_number;
}

std::string device::get_firmware_version() const
{
    return m_firmware_version;
}

void device::add_description(device_description const &extension_description, device_model& device_model_)
{
    auto ddl = std::make_unique<device_description_loader>(device_model_, *this);
    ddl->add_description(extension_description);
    if(ddl->was_model_incomplete())
    {
        wc_log(log_level_t::warning, "Missing model information - Not all device_description information could be processed.");
        m_incomplete_loaders.push_back(std::move(ddl));
    }
}

void device::add_wdd(std::string wdd_content, device_model& device_model_)
{
    auto ddl = std::make_unique<device_description_loader>(device_model_, *this);
    ddl->load(wdd_content);
    if(ddl->was_model_incomplete())
    {
        wc_log(log_level_t::warning, "Missing model information - Not all device_extension information could be processed");
        m_incomplete_loaders.push_back(std::move(ddl));
    }
}

void device::retry_unfinished_device_informations()
{
    if(m_incomplete_loaders.empty())
    {
        return;
    }
    wc_log(log_level_t::info, "New model information arrived - retrying " + std::to_string(m_incomplete_loaders.size()) + " device information sources");
    for(auto& loader : m_incomplete_loaders)
    {
        loader->retry_with_updated_model();
    }
    m_incomplete_loaders.erase(std::remove_if(m_incomplete_loaders.begin(), m_incomplete_loaders.end(), [](auto& l){ return !l->was_model_incomplete(); }), m_incomplete_loaders.end());
    wc_log(log_level_t::info, std::to_string(m_incomplete_loaders.size()) + " incomplete device information sources remaining.");
}

}
}
