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
#ifndef SRC_LIBWDXCORE_UTILS_PATH_TREE_NODE_HPP_
#define SRC_LIBWDXCORE_UTILS_PATH_TREE_NODE_HPP_

#include <stdexcept>
#include <set>
#include <string>
#include <algorithm>
#include "path.hpp"

namespace wago {
namespace wdx {

template <typename T>
class path_tree_node
{
    std::string m_name;
    T m_value = nullptr; // this means T has to be of pointer type, which is okay since we use it for parameter_instance* only

public:
    path_tree_node() = default;

    path_tree_node(path_tree_node const &) = default;
    path_tree_node& operator=(path_tree_node const &) = default;

    path_tree_node(std::string name)
        : m_name(name)
    { }

    ~path_tree_node() noexcept
    {
        for (auto& item : items)
        {
            delete item;
        }
    }

    // TODO: https://en.cppreference.com/w/cpp/language/rule_of_three

    void add_value(path path, T value)
    {
        path_tree_node* node = this;
        for (auto& segment : path.get_segments())
        {
            auto newNode = node->add_or_reuse_node(segment);
            node = newNode;
        }
        node->m_value = value;
    }

    path_tree_node* get_node_under_path(path& path, std::vector<T>& visited_values, bool backOnDeadEnd = false)
    {
        path_tree_node* node = this;

        std::string instanceId;

        for (auto& segment : path.get_segments())
        {
            // defer instanceId (see how ParameterInstanceCollection.hpp makes the path)
            if (isdigit(static_cast<unsigned char>(segment[0])))
            {
                instanceId = segment;
                continue;
            }

            auto new_node = node->get_node(segment);
            if (new_node != nullptr)
            {
                if(node->m_value)
                    visited_values.push_back(node->m_value);
                node = new_node;
            }
            else
            {
                return nullptr;
            }
        }
        if (node != nullptr)
        {
            if (instanceId.empty())
            {
                return node;
            }
            else
            {
                auto last_node = node->get_node(instanceId);
                if(!last_node)
                    last_node = node->get_node(std::to_string(DYNAMIC_PLACEHOLDER_INSTANCE_ID));
                if(!last_node && backOnDeadEnd)
                    return node;
                return last_node;
            }
        }
        else
        {
            return nullptr;
        }
    }

    T get_value(path path)
    {
        std::vector<T> dummy;
        auto node = get_node_under_path(path, dummy);
        return node ? node->m_value : nullptr; // this is actually only possible when T is pointer type
    }

    void gather_values(path_tree_node* node, std::vector<T>& gathered_values) {
        if(node->m_value)
            gathered_values.push_back(node->m_value);
        for(auto it : node->items) {
            gather_values(it, gathered_values);            
        }
    }

private:

    struct comp
    {
        bool operator()(const path_tree_node* lhs, const path_tree_node* rhs) const
        {
            // case insensitive comparison
            std::string s1 = lhs->m_name;
            std::string s2 = rhs->m_name;
            std::transform(s1.begin(), s1.end(), s1.begin(), [](unsigned char c){ return std::tolower(c); });
            std::transform(s2.begin(), s2.end(), s2.begin(), [](unsigned char c){ return std::tolower(c); });

            return s1 < s2;
        }
    };

    std::set<path_tree_node<T>*, comp> items; // TODO: optimize memory usage by using more compact item container

    path_tree_node* get_node(std::string name) const
    {
        auto temp = new path_tree_node(name); // TODO: this is ugly
        auto match = items.find(temp);
        delete temp;
        if (match != items.end())
        {
            return *match;
        }
        else
        {
            return nullptr;
        }
    }

    path_tree_node* add_or_reuse_node(std::string name)
    {
        auto temp = new path_tree_node(name); // TODO: this is ugly
        auto match = items.find(temp);
        delete temp;
        if (match != items.end())
        {
            return *match;
        }
        else
        {
            auto node = new path_tree_node(name);
            items.insert(node);
            return node;
        }
    }
};

}
}
#endif // SRC_LIBWDXCORE_UTILS_PATH_TREE_NODE_HPP_
