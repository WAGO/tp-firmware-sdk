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
#include "path.hpp"
#include "string_util.hpp"

namespace wago {
namespace wdx {

constexpr char const path::DELIMITER;

path::path(std::string path_)
    : m_path(std::move(path_))
{
}

path::path(path const &path1, path const &path2)
{
    if (path1.m_path.empty())
    {
        m_path = path2.m_path;
    }
    else if (path2.m_path.empty())
    {
        m_path = path1.m_path;
    }
    else
    {
        m_path = path1.m_path + DELIMITER + path2.m_path; // TODO: make sure DELIMITER is always correct
    }
}

path::path(path const &path1, path const &path2, path const &path3)
    : path(path(path1, path2), path3)
{
}

std::vector<std::string> path::get_segments() const
{
    return split_string(m_path, DELIMITER);
}

}
}
