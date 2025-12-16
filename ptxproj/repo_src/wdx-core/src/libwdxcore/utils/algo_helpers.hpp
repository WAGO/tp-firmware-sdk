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
#pragma once

#include <algorithm>
#include <functional>
#include <vector>
#include <set>
#include <iterator>

namespace wago {
namespace wdx {

template<class T>
bool contains(const std::vector<T>& v, std::function<bool(const T& x)> pred)
{
    return end(v) != std::find_if(begin(v), end(v), pred);
}

template<class T>
bool contains(const std::set<T>& v, const T& el)
{
    return end(v) != v.find(el);
}

template<typename T, typename TKey>
bool push_back_unique(std::vector<T>& v, T item, std::function<TKey(const T& x)> keyFn)
{
    auto key = keyFn(item);
    if(!contains<T>(v, [&](auto& x) {
        return key == keyFn(x);
    }))
    {
        v.push_back(item);
        return true;
    }
    else return false;
}

template<class T>
void append(std::vector<T>&dest, const std::vector<T>&src) {
    dest.insert(dest.end(), src.begin(), src.end());
}

template<class T>
void append_if(std::vector<T>&dest, const std::vector<T>&src, std::function<bool(const T& x)> pred) {
    for(auto& it : src) {
        if(pred(it)) {
            dest.push_back(it);
        }
    }
}

template<typename T, typename TKey>
void append_unique(std::vector<T>&dest, const std::vector<T>&src, std::function<TKey(const T& x)> keyFn, std::function<void(const T& x)> collisionFn = [](const T& x){(void)(x);}) {
    for(auto& item : src) {
        if(!push_back_unique(dest, item, keyFn))
            collisionFn(item);
    }
}

template <typename T>
void insert_unique(std::vector<T>& dest, std::vector<T>& src)
{
    // order preserving insert without duplicates
    // vectors have to be sorted
    // https://stackoverflow.com/a/3633142
    auto mid = dest.size();
    std::copy(src.begin(), src.end(), std::back_inserter(dest));
    std::inplace_merge(dest.begin(), dest.begin() + mid, dest.end());
    dest.erase(unique(dest.begin(), dest.end()), dest.end());
}

}}
