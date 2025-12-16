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
#include "wago/wdx/parameter_selector.hpp"

namespace wago {
namespace wdx {

const device_selector device_selector::any = device_selector();

device_selector device_selector::headstation() noexcept
{
    return device_selector(device_id::headstation, false, false);
}

device_selector device_selector::all_of(device_collections device_collection) noexcept
{
    return device_selector(device_id{0, device_collection}, false, true);
}

device_selector device_selector::specific(device_id device) noexcept
{
    return device_selector(device, false, false);
}

device_selector::device_selector() noexcept
: device_selector(device_id(0, 0), true, true) // Not using device_id::headstation because maybe not initialized in early / static context
{ }

device_selector::device_selector(device_id selected_device, bool unspecific, bool collection_selector) noexcept
: device(selected_device)
, is_unspecific(unspecific)
, is_collection(collection_selector)
{ }

device_id device_selector::get_selected_device() const noexcept
{
    return device;
}

bool device_selector::is_any_selector() const noexcept
{
    return is_unspecific;
}

bool device_selector::is_collection_selector() const noexcept
{
    return is_collection;
}

bool operator==(device_selector const &lhs, device_selector const &rhs) noexcept
{
    return    lhs.get_selected_device()    == rhs.get_selected_device()
           && lhs.is_any_selector()        == rhs.is_any_selector()
           && lhs.is_collection_selector() == rhs.is_collection_selector();
}

bool operator!=(device_selector const &lhs, device_selector const &rhs) noexcept
{
    return !(lhs == rhs);
}

parameter_selector parameter_selector::all_of_feature(std::string feature, device_selector devices) {
    return parameter_selector(parameter_selector_type::feature, feature, 0, devices);
}

parameter_selector parameter_selector::all_of_class(std::string cls, device_selector devices) {
    return parameter_selector(parameter_selector_type::cls, cls, 0, devices);
}

parameter_selector parameter_selector::all_with_definition(parameter_id_t definition, device_selector devices) {
    return parameter_selector(parameter_selector_type::parameter_definition, "", definition, devices);
}

parameter_selector parameter_selector::all_of_devices(device_selector devices) {
    return parameter_selector(parameter_selector_type::device_collection, "", 0, devices);
}

parameter_selector_type parameter_selector::get_selector_type() const noexcept
{
    return selector_type;
}

std::string parameter_selector::get_selected_name() const noexcept
{
    return name_selection;
}

parameter_id_t parameter_selector::get_selected_id() const noexcept
{
    return parameter_id;
}

device_selector parameter_selector::get_selected_devices() const noexcept
{
    return devices;
}

parameter_selector::parameter_selector()
: selector_type(parameter_selector_type::feature)
, parameter_id(0)
{ }

parameter_selector::parameter_selector(parameter_selector_type type,
                                       std::string             selected_name,
                                       parameter_id_t          selected_id,
                                       device_selector         selected_devices)
: selector_type(type)
, name_selection(selected_name)
, parameter_id(selected_id)
, devices(selected_devices)
{ }

}
}
