//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    WDA check utilities for google test/mock.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_TEST_WDA_CHECK_HPP_
#define INC_WAGO_WDX_TEST_WDA_CHECK_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
#define PRINT_WDA_STATUS_INTO_STREAM(expected_status_code, actual_status_code) \
       "Expected status code: " << wago::wdx::to_string(expected_status_code) \
    << " (" << static_cast<unsigned>(expected_status_code) << ")" << std::endl \
    << "Actual status code:   " << wago::wdx::to_string(actual_status_code) \
    << " (" << static_cast<unsigned>(actual_status_code) << ")"

/// Check a wda status code to be successful
#define EXPECT_WDA_STATUS(expected_status_code, actual_status_code) \
    EXPECT_EQ(expected_status_code, actual_status_code) \
        << PRINT_WDA_STATUS_INTO_STREAM(expected_status_code, actual_status_code)

/// Ensure a wda status code to be successful
#define ASSERT_WDA_STATUS(expected_status_code, actual_status_code) \
    ASSERT_EQ(expected_status_code, actual_status_code) \
        << PRINT_WDA_STATUS_INTO_STREAM(expected_status_code, actual_status_code)

/// Check a wda response to be successful
#define EXPECT_WDA_SUCCESS(actual_response) \
    EXPECT_TRUE( (actual_response).is_determined()); \
    EXPECT_FALSE((actual_response).has_error()); \
    EXPECT_WDA_STATUS(wago::wdx::status_codes::success, (actual_response).status)

/// Ensure a wda response to be successful
#define ASSERT_WDA_SUCCESS(actual_response) \
    EXPECT_TRUE( (actual_response).is_determined()); \
    EXPECT_FALSE((actual_response).has_error()); \
    ASSERT_WDA_STATUS(wago::wdx::status_codes::success, (actual_response).status)

/// Ensure a wda response future to contain a value
#define ASSERT_FUTURE_VALUE(response_future) \
    ASSERT_TRUE( (response_future).valid()); \
    ASSERT_TRUE( (response_future).ready()); \
    EXPECT_FALSE((response_future).has_exception()); \
    ASSERT_TRUE( (response_future).has_value())

/// Ensure a wda response future to contain a value within a given duration
#define ASSERT_FUTURE_VALUE_WITHIN(response_future, duration) \
    ASSERT_TRUE( (response_future).valid()); \
    response_future.wait_for(duration); \
    ASSERT_TRUE( (response_future).ready()); \
    EXPECT_FALSE((response_future).has_exception()); \
    ASSERT_TRUE( (response_future).has_value())

/// Ensure a wda response future to contain an exception
#define ASSERT_FUTURE_EXCEPTION(response_future) \
    ASSERT_TRUE( (response_future).valid()); \
    ASSERT_TRUE( (response_future).ready()); \
    EXPECT_FALSE((response_future).has_value()); \
    ASSERT_TRUE( (response_future).has_exception())

/// Ensure a wda response future to contain an exception within a given duration
#define ASSERT_FUTURE_EXCEPTION_WITHIN(response_future, duration) \
    ASSERT_TRUE( (response_future).valid()); \
    response_future.wait_for(duration); \
    ASSERT_TRUE( (response_future).ready()); \
    EXPECT_FALSE((response_future).has_value()); \
    ASSERT_TRUE( (response_future).has_exception())


#endif // INC_WAGO_WDX_TEST_WDA_CHECK_HPP_

//---- End of source file ------------------------------------------------------
