// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "tc.hpp"

namespace wago::libswitchconfig {
    namespace {
    ::std::string ingress_filter_json = R"([ {
        "protocol": "all",
        "pref": 49150,
        "kind": "flower",
        "chain": 0
    },{
        "protocol": "all",
        "pref": 49150,
        "kind": "flower",
        "chain": 0,
        "options": {
            "handle": 1,
            "keys": {
                "dst_mac": "01:00:00:00:00:00/01:00:00:00:00:00"
            },
            "skip_sw": true,
            "in_hw": true,
            "in_hw_count": 1,
            "actions": [ {
                    "order": 1,
                    "kind": "police",
                    "index": 2,
                    "pkts_rate": 20000,
                    "pkts_burst": 0,
                    "control_action": {
                        "type": "drop"
                    },
                    "overhead": 0,
                    "ref": 1,
                    "bind": 1
                } ]
        }
    },{
        "protocol": "all",
        "pref": 49151,
        "kind": "matchall",
        "chain": 0
    },{
        "protocol": "all",
        "pref": 49151,
        "kind": "matchall",
        "chain": 0,
        "options": {
            "handle": 1,
            "not_in_hw": true,
            "actions": [ {
                    "order": 1,
                    "kind": "mirred",
                    "mirred_action": "mirror",
                    "direction": "egress",
                    "to_dev": "ethX1",
                    "control_action": {
                        "type": "pipe"
                    },
                    "index": 2,
                    "ref": 1,
                    "bind": 1
                } ]
        }
    },{
        "protocol": "all",
        "pref": 49152,
        "kind": "flower",
        "chain": 0
    },{
        "protocol": "all",
        "pref": 49152,
        "kind": "flower",
        "chain": 0,
        "options": {
            "handle": 1,
            "keys": {
                "dst_mac": "ff:ff:ff:ff:ff:ff"
            },
            "skip_sw": true,
            "in_hw": true,
            "in_hw_count": 1,
            "actions": [ {
                    "order": 1,
                    "kind": "police",
                    "index": 1,
                    "pkts_rate": 4000,
                    "pkts_burst": 0,
                    "control_action": {
                        "type": "drop"
                    },
                    "overhead": 0,
                    "ref": 1,
                    "bind": 1
                } ]
        }
    } ]
    )";
    }

    TEST(parse_filter_json, get_broadcast_ratelimit_pref) {
        auto j = nlohmann::json::parse(ingress_filter_json);
        auto pref = get_ingress_ratelimit_filter_ref(j, "ff:ff:ff:ff:ff:ff");
        ASSERT_TRUE(pref.has_value());
        EXPECT_EQ(pref.value().pref, 49152);
        EXPECT_EQ(pref.value().handle, 1);
    }

     TEST(parse_filter_json, get_multicast_ratelimit_pref) {
        auto j = nlohmann::json::parse(ingress_filter_json);
        auto pref = get_ingress_ratelimit_filter_ref(j, "01:00:00:00:00:00/01:00:00:00:00:00");
        ASSERT_TRUE(pref.has_value());
        EXPECT_EQ(pref.value().pref, 49150);
        EXPECT_EQ(pref.value().handle, 1);
    }

    TEST(parse_filter_json, get_matchall_mirror_ref) {
        auto j = nlohmann::json::parse(ingress_filter_json);
        auto pref = get_mirror_filter_ref(j);
        ASSERT_TRUE(pref.has_value());
        EXPECT_EQ(pref.value().pref, 49151);
        EXPECT_EQ(pref.value().handle, 1);
    }
}
