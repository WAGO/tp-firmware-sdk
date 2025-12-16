//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------
#include "monitoring_list_collection.hpp"

namespace wago {
namespace wdx {

monitoring_list_info monitoring_list_collection::create_monitoring_list(
        std::shared_ptr<std::vector<parameter_instance*>>  parameter_instances,
        std::vector<parameter_response>                    results,
        uint16_t                                           timeout_seconds,
        status_codes                                      &status)
{
    cleanup_monitoring_lists();

    // find next free idx
    size_t next_idx = 0;
    for(auto& ml : m_monitoring_lists)
    {
        if(ml == nullptr)
        {
            break;
        }
        next_idx++;
    }

    // max. 100 monitoring_lists seems sensible
    if(next_idx > 100)
    {
        status = status_codes::monitoring_list_max_exceeded;
        return monitoring_list_collection::invalid_monitoring_list;
    }
    
    auto ml = std::make_shared<monitoring_list>();
    ml->id = m_next_id++;
    ml->timeout_seconds = timeout_seconds;
    ml->one_off = timeout_seconds == 0;
    ml->last_access = std::chrono::system_clock::now();
    ml->parameter_instances = std::move(parameter_instances);
    ml->results = std::move(results);
    
    if(next_idx == m_monitoring_lists.size())
    {
        m_monitoring_lists.push_back(ml);
    }
    else
    {
        m_monitoring_lists.at(next_idx) = ml;
    }
    status = status_codes::success;
    return monitoring_list_collection::to_info(*ml);
}

std::shared_ptr<monitoring_list_collection::monitoring_list> monitoring_list_collection::get_monitoring_list(monitoring_list_id_t id, status_codes& status)
{
    cleanup_monitoring_lists();

    for(size_t idx=0, e=m_monitoring_lists.size(); idx < e; idx++)
    {
        auto ml = m_monitoring_lists.at(idx);
        if(ml && ml->id == id)
        {
            status = status_codes::success;
            ml->last_access = std::chrono::system_clock::now();
            if(ml->one_off)
            {
                m_monitoring_lists.at(idx) = nullptr;
            }
            return ml;
        }
    }

    status = status_codes::unknown_monitoring_list;
    return nullptr;
}

status_codes monitoring_list_collection::delete_monitoring_list(monitoring_list_id_t id)
{
    cleanup_monitoring_lists();

    for(size_t idx=0, e=m_monitoring_lists.size(); idx < e; idx++)
    {
        auto ml = m_monitoring_lists.at(idx);
        if(ml && ml->id == id)
        {
            m_monitoring_lists.at(idx) = nullptr;
            return status_codes::success;
        }
    }

    return status_codes::unknown_monitoring_list;
}

void monitoring_list_collection::cleanup_monitoring_lists()
{
    for(size_t idx = 0, e = m_monitoring_lists.size(); idx < e; idx++)
    {
        auto& ml = m_monitoring_lists.at(idx);
        if(ml && !ml->one_off)
        {
            auto delta = std::chrono::system_clock::now() - ml->last_access;
            auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(delta).count();
            if(elapsed_seconds > ml->timeout_seconds)
            {
                m_monitoring_lists.at(idx) = nullptr;
            }
        }
    }
}

monitoring_list_info monitoring_list_collection::get_monitoring_list_info(monitoring_list_id_t id, status_codes& status) {
    auto ml = get_monitoring_list(id, status);
    if(ml)
    {
        return monitoring_list_collection::to_info(*ml);
    }
    else
    {
        return monitoring_list_collection::invalid_monitoring_list;
    }
}

std::vector<monitoring_list_info> monitoring_list_collection::get_monitoring_list_infos()
{
    cleanup_monitoring_lists();

    std::vector<monitoring_list_info> result;
    
    for(size_t idx = 0, e = m_monitoring_lists.size(); idx < e; idx++)
    {
        auto& ml = m_monitoring_lists.at(idx);
        if(ml)
        {
            result.push_back(monitoring_list_collection::to_info(*ml));
        }
    }

    return result;
}

monitoring_list_info monitoring_list_collection::to_info(monitoring_list_collection::monitoring_list& ml)
{
    return monitoring_list_info{ml.id, ml.one_off, ml.timeout_seconds};
}

monitoring_list_info monitoring_list_collection::invalid_monitoring_list = monitoring_list_info{0,false,0};

}}