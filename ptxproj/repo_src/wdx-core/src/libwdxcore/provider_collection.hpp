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
#ifndef SRC_LIBWDXCORE_PROVIDER_COLLECTION_HPP_
#define SRC_LIBWDXCORE_PROVIDER_COLLECTION_HPP_

#include <vector>
#include <memory>
#include <mutex>

namespace wago {
namespace wdx {

// TODO: Replace with std::map

template <typename T, typename TData>
class provider_collection
{

    struct mp
    {
        T* provider;
        TData data;
    };

    std::vector<mp> m_mps;
    std::mutex m_provider_mutex;

    mp* get_unsafe(const T* provider)
    {
        for(auto& m : m_mps) // parasoft-suppress CERT_C-CON43-a-3 "Explicitly unsafe method for internal usage only"
        {
            if(m.provider == provider) return &m;
        }
        return nullptr;
    }

    public:

    bool add(T* provider, TData data)
    {
        std::lock_guard<std::mutex> guard(m_provider_mutex);
        auto p = get_unsafe(provider);
        if(p) return false;
        m_mps.push_back(mp{provider, std::move(data)});
        return true;
    }

    bool update_data(T* provider, TData data)
    {
        std::lock_guard<std::mutex> guard(m_provider_mutex);
        auto p = get_unsafe(provider);
        if(!p) return false;
        p->data = std::move(data);
        return true;
    }

    bool remove(T* provider)
    {
        std::lock_guard<std::mutex> guard(m_provider_mutex);
        size_t idx = 0;
        for(auto& m : m_mps)
        {
            if(m.provider == provider)
            {
                m_mps.erase(m_mps.begin()+idx);
                return true;
            }
            idx++;
        }
        return false;
    }

    bool exists(T* provider)
    {
        std::lock_guard<std::mutex> guard(m_provider_mutex);
        return get_unsafe(provider);
    }

    void for_each(std::function<void(T* provider, TData& data)> action)
    {
        std::lock_guard<std::mutex> guard(m_provider_mutex);
        for(auto& p : m_mps)
        {
            action(p.provider, p.data);
        }
    }

    T* find(std::function<bool(T* provider, TData& data)> pred)
    {
        std::lock_guard<std::mutex> guard(m_provider_mutex);
        for(auto& p : m_mps)
        {
            if(pred(p.provider, p.data)) return p.provider;
        }
        return nullptr;
    }

    TData* find_data(std::function<bool(T* provider, TData& data)> pred)
    {
        std::lock_guard<std::mutex> guard(m_provider_mutex);
        for(auto& p : m_mps)
        {
            if(pred(p.provider, p.data)) return &p.data;
        }
        return nullptr;
    }

    size_t size()
    {
        std::lock_guard<std::mutex> guard(m_provider_mutex);
        return m_mps.size();
    }

};

}
}

#endif // SRC_LIBWDXCORE_PROVIDER_COLLECTION_HPP_
