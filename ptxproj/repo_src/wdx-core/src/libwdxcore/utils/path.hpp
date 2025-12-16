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
#ifndef SRC_LIBWDXCORE_UTILS_PATH_HPP_
#define SRC_LIBWDXCORE_UTILS_PATH_HPP_

#include <vector>
#include <string>

namespace wago {
namespace wdx {

struct path
{
    static constexpr char const DELIMITER = '/';

    std::string m_path;

    path(std::string path);
    path(path const &path1, path const &path2);
    path(path const &path1, path const &path2, path const &path3);

    // TODO: use input generator instead of creating a vector
    std::vector<std::string> get_segments() const;
};

inline bool operator==(const path& lhs, const path& rhs) noexcept
{
    return lhs.m_path == rhs.m_path;
}

}}

#endif // SRC_LIBWDXCORE_UTILS_PATH_HPP_
