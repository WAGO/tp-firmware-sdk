//------------------------------------------------------------------------------
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Test WAGO Parameter Service Core (basic behavior).
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "test_providers.hpp"
#include "parameter_service_core.hpp"
#include "mocks/mock_permissions.hpp"

#include <wago/wdx/test/wda_check.hpp>
#include <gtest/gtest.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;

class core_fixture : public ::testing::Test
{
private:
    std::unique_ptr<mock_permissions> permissions_mock_ptr = std::make_unique<mock_permissions>();

protected:
    mock_permissions &permissions_mock = *permissions_mock_ptr;

    std::unique_ptr<wago::wdx::parameter_service_i> core_m = std::make_unique<parameter_service_core>(std::move(permissions_mock_ptr));

protected:
    core_fixture() = default;
    ~core_fixture() override = default;
    void SetUp() override
    {
        permissions_mock.set_default_expectations();
    }
};

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST_F(core_fixture, ConstructDelete)
{
    // Everything is done with fixture SetUp
}

TEST_F(core_fixture, EmptyStringModel)
{
    wago::wdx::wdm_content_t test_model = "";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
}

TEST_F(core_fixture, EmptyModel)
{
    wago::wdx::wdm_content_t test_model = "{}";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
}

TEST_F(core_fixture, BrokenJsonModel)
{
    wago::wdx::wdm_content_t test_model = "{";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
    EXPECT_FALSE(responses.at(0).has_error()); // No error expected because model should be loaded after registration
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(0).status);
}

TEST_F(core_fixture, VersionOnlyModel)
{
    wago::wdx::wdm_content_t test_model = "{" \
                                              "\"Version\": \"1.0.0\"" \
                                          "}";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
    EXPECT_FALSE(responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(0).status);
}

TEST_F(core_fixture, HigherVersionModel)
{
    wago::wdx::wdm_content_t test_model = "{" \
                                              "\"Version\": \"999.99.9\"" \
                                          "}";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
    EXPECT_FALSE(responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(0).status);
}

TEST_F(core_fixture, UnknownModelElementTopLevel)
{
    wago::wdx::wdm_content_t test_model = "{" \
                                              "\"Version\": \"1.0.0\"," \
                                              "\"SomeUnknownElement\": 41" \
                                          "}";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
    EXPECT_FALSE(responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(0).status);
}

TEST_F(core_fixture, EmptyFeatureListModel)
{
    wago::wdx::wdm_content_t test_model = "{" \
                                              "\"Version\": \"1.0.0\"," \
                                              "\"Features\": []" \
                                          "}";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
    EXPECT_FALSE(responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(0).status);
}

TEST_F(core_fixture, EmptyParameterListModel)
{
    wago::wdx::wdm_content_t test_model = "{" \
                                              "\"Version\": \"1.0.0\"," \
                                              "\"Features\": " \
                                              "[" \
                                                  "{" \
                                                      "\"ID\": \"MyFeature\"," \
                                                      "\"Parameters\": []" \
                                                  "}" \
                                              "]" \
                                          "}";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
    EXPECT_FALSE(responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(0).status);
}

TEST_F(core_fixture, SingleParameterModel)
{
    wago::wdx::wdm_content_t test_model = "{" \
                                              "\"Version\": \"1.0.0\"," \
                                              "\"Features\": " \
                                              "[" \
                                                  "{" \
                                                      "\"ID\": \"MyFeature\"," \
                                                      "\"Parameters\":" \
                                                      "[" \
                                                          "{" \
                                                              "\"ID\": 93," \
                                                              "\"Path\": \"Some/Path\"," \
                                                              "\"Type\": \"String\"" \
                                                          "}" \
                                                      "]" \
                                                  "}" \
                                              "]" \
                                          "}";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
    EXPECT_FALSE(responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(0).status);
}

TEST_F(core_fixture, MissingIdForParameterModel)
{
    wago::wdx::wdm_content_t test_model = "{" \
                                              "\"Version\": \"1.0.0\"," \
                                              "\"Features\": " \
                                              "[" \
                                                  "{" \
                                                      "\"ID\": \"MyFeature\"," \
                                                      "\"Parameters\":" \
                                                      "[" \
                                                          "{" \
                                                              "\"Path\": \"Some/Path\"," \
                                                              "\"Type\": \"String\"" \
                                                          "}" \
                                                      "]" \
                                                  "}" \
                                              "]" \
                                          "}";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
    EXPECT_FALSE(responses.at(0).has_error()); // No error expected because model should be loaded after registration
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(0).status);
}

TEST_F(core_fixture, MissingPathForParameterModel)
{
    wago::wdx::wdm_content_t test_model = "{" \
                                              "\"Version\": \"1.0.0\"," \
                                              "\"Features\": " \
                                              "[" \
                                                  "{" \
                                                      "\"ID\": \"MyFeature\"," \
                                                      "\"Parameters\":" \
                                                      "[" \
                                                          "{" \
                                                              "\"ID\": 93," \
                                                              "\"Type\": \"String\"" \
                                                          "}" \
                                                      "]" \
                                                  "}" \
                                              "]" \
                                          "}";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
    EXPECT_FALSE(responses.at(0).has_error()); // No error expected because model should be loaded after registration
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(0).status);
}

TEST_F(core_fixture, UnknownModelElementParameterTypeModel)
{
    wago::wdx::wdm_content_t test_model = "{" \
                                              "\"Version\": \"1.0.0\"," \
                                              "\"Features\": " \
                                              "[" \
                                                  "{" \
                                                      "\"ID\": \"MyFeature\"," \
                                                      "\"Parameters\":" \
                                                      "[" \
                                                          "{" \
                                                              "\"ID\": 93," \
                                                              "\"Path\": \"Some/Path\"," \
                                                              "\"Type\": \"SomeUnknownThing\"" \
                                                          "}" \
                                                      "]" \
                                                  "}" \
                                              "]" \
                                          "}";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
    EXPECT_FALSE(responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(0).status);
}

TEST_F(core_fixture, UnknownModelElementParameterAttributeModel)
{
    wago::wdx::wdm_content_t test_model = "{" \
                                              "\"Version\": \"1.0.0\"," \
                                              "\"Features\": " \
                                              "[" \
                                                  "{" \
                                                      "\"ID\": \"MyFeature\"," \
                                                      "\"Parameters\":" \
                                                      "[" \
                                                          "{" \
                                                              "\"ID\": 93," \
                                                              "\"Path\": \"Some/Path\"," \
                                                              "\"UnknownAttribute\": \"Some scary thing\"," \
                                                              "\"Type\": \"String\"" \
                                                          "}" \
                                                      "]" \
                                                  "}" \
                                              "]" \
                                          "}";

    test_model_provider model_provider(test_model);
    auto future_responses = core_m->register_model_providers({ &model_provider });
    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
    EXPECT_FALSE(responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(0).status);
}


//---- End of source file ------------------------------------------------------
