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
#ifndef SRC_LIBWDXCORE_JSON_HELPERS_HPP_
#define SRC_LIBWDXCORE_JSON_HELPERS_HPP_

#include <nlohmann/json.hpp>
#include <vector>

namespace wago {
namespace wdx {

template <typename T>
std::vector<T> json_to_vector(nlohmann::json const &node, T (*f)(nlohmann::json const &))
{
    std::vector<T> result;
    for (auto const &n : node)
    {
        result.push_back(f(n));
    }
    return result;
}

// RPC
template <typename T>
nlohmann::json vector_to_json(const std::vector<T> vec, nlohmann::json (*f)(const T&))
{
    nlohmann::json result = nlohmann::json::array();
    for (auto& item : vec)
    {
        result.push_back(f(item));
    }
    return result;
}

}
}
#endif // SRC_LIBWDXCORE_JSON_HELPERS_HPP_
