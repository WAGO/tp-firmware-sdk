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
///
///  \brief    Parameter provider for parameter service related settings.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_SETTINGS_PROVIDER_HPP_
#define SRC_LIBWDXWDA_SETTINGS_PROVIDER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/base_parameter_provider.hpp>
#include <wago/wdx/wda/settings_store_i.hpp>

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

class settings_provider : public wdx::base_parameter_provider
{
public:
    settings_provider(std::shared_ptr<settings_store_i> const &settings_store);
    ~settings_provider() noexcept override = default;

private:
    std::shared_ptr<settings_store_i> settings_store_m;

// parameter provider
public:
    std::string display_name() override;
    parameter_selector_response get_provided_parameters() override;
    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameter_ids) override;
    wago::future<std::vector<set_parameter_response>> set_parameter_values(std::vector<value_request> value_requests) override;

// actual implementations
private:
    std::shared_ptr<parameter_value> load_allowed_unauthenticated_requests();
    void save_allowed_unauthenticated_requests(std::shared_ptr<parameter_value> const &value);
};

} // Namespace wda
} // Namespace wdx
} // Namespace wago

#endif /* SRC_LIBWDXWDA_SETTINGS_PROVIDER_HPP_ */
//---- End of source file ------------------------------------------------------
