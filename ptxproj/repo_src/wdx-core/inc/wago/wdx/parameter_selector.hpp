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

#ifndef INC_WAGO_WDX_PARAMETER_SELECTOR_HPP_
#define INC_WAGO_WDX_PARAMETER_SELECTOR_HPP_

#include <string>
#include "wago/wdx/parameter_instance_id.hpp"

namespace wago {
namespace wdx {

enum class parameter_selector_type {
    feature,
    cls,
    parameter_definition,
    device_collection
};

/**
    A selector for devices, independent of when the device will actually be registered.
 * */
class device_selector {
public:
    /**
    Selects every device, regardless of when it will be registered.
    */
    static const device_selector any;

    /**
    Selects the headstation, even if it is only registered later.
    */
    static device_selector headstation() noexcept;

    /**
    Selects all device of the given device collection (including devices that are registered later).
    */
    static device_selector all_of(device_collections device_collection) noexcept;

    /**
    Selects the device at a specific location, even if it is only registered later.
    */
    static device_selector specific(device_id device) noexcept;

    device_id get_selected_device() const noexcept;

    bool is_any_selector() const noexcept;
    bool is_collection_selector() const noexcept;

    device_selector() noexcept;

protected:
    device_selector(device_id selected_device,
                    bool      unspecific,
                    bool      collection_selector) noexcept;

    device_id device;
    bool      is_unspecific;
    bool      is_collection;
};

bool operator==(device_selector const &lhs, device_selector const &rhs) noexcept;
bool operator!=(device_selector const &lhs, device_selector const &rhs) noexcept;

/**
    A selector for a subset of parameter instances.
 * */
class parameter_selector {
public:
    /**
    Selects all parameter instances of a given `feature`, optionally filtered by `device_selector`.
    Providing for such a selector includes devices that might be registered later.
    */
    static parameter_selector all_of_feature(std::string feature, device_selector devices = device_selector::any);

    /**
    Selects all parameter instances of a given `class`, regardless of `instance_id`, optionally filtered by `device_selector`.
    Providing for such a selector includes devices that might be registered later.
    */
    static parameter_selector all_of_class(std::string cls, device_selector devices = device_selector::any);

    /**
    Selects all parameter instances of a given `definition`, regardless of `instance_id`, optionally filtered by `device_selector`.
    Providing for such a selector includes devices that might be registered later.
    */
    static parameter_selector all_with_definition(parameter_id_t definition, device_selector devices = device_selector::any);

    /**
    Selects all parameter instances for all devices that match the device_selector `device`.
    Providing for such a selector includes devices that might be registered later.
    */
    static parameter_selector all_of_devices(device_selector devices);

    parameter_selector_type get_selector_type() const noexcept;

    std::string     get_selected_name() const noexcept;
    parameter_id_t  get_selected_id() const noexcept;
    device_selector get_selected_devices() const noexcept;

    parameter_selector();

protected:
    parameter_selector(parameter_selector_type type,
                       std::string             selected_name,
                       parameter_id_t          selected_id,
                       device_selector         selected_devices);

private:
    parameter_selector_type selector_type;
    std::string             name_selection;
    parameter_id_t          parameter_id;
    device_selector         devices;
};


}}

#endif // INC_WAGO_WDX_PARAMETER_SELECTOR_HPP_
