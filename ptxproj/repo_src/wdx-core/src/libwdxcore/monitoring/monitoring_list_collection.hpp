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
#ifndef SRC_LIBWDXCORE_MONITORING_MONITORING_LIST_COLLECTION_HPP_
#define SRC_LIBWDXCORE_MONITORING_MONITORING_LIST_COLLECTION_HPP_

#include <chrono>
#include "wago/wdx/responses.hpp"
#include "instances/parameter_instance.hpp"

namespace wago {
namespace wdx {

class monitoring_list_collection {
public:
    struct monitoring_list {
        monitoring_list_id_t id;
        bool one_off;
        uint16_t timeout_seconds;
        std::chrono::time_point<std::chrono::system_clock> last_access;
        std::shared_ptr<std::vector<parameter_instance*>> parameter_instances;
        std::vector<parameter_response> results;
    };

    monitoring_list_info create_monitoring_list(
        std::shared_ptr<std::vector<parameter_instance*>> parameter_instances,
        std::vector<parameter_response> results,
        uint16_t timeout_seconds,
        status_codes& status);

    // NOTE: here we return a shared_ptr, because a monitoring_list could be removed by the monitoring_list_collection,
    // and we don't want the caller to have a invalid reference
    std::shared_ptr<monitoring_list> get_monitoring_list(monitoring_list_id_t id, status_codes& status);
    monitoring_list_info get_monitoring_list_info(monitoring_list_id_t id, status_codes& status);
    std::vector<monitoring_list_info> get_monitoring_list_infos();
    status_codes delete_monitoring_list(monitoring_list_id_t id);
    void cleanup_monitoring_lists();
    static monitoring_list_info invalid_monitoring_list;

private:
    std::vector<std::shared_ptr<monitoring_list>> m_monitoring_lists;
    monitoring_list_id_t m_next_id = 1;
    static monitoring_list_info to_info(monitoring_list& ml);

};


}}
#endif // SRC_LIBWDXCORE_MONITORING_MONITORING_LIST_COLLECTION_HPP_
