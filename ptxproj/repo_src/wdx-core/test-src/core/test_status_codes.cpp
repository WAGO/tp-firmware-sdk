//------------------------------------------------------------------------------
// Copyright (c) 2024 WAGO GmbH & Co. KG
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
///  \brief    Test parameter filter.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "wago/wdx/status_codes.hpp"

struct status_code_test_data {
    wago::wdx::status_codes status_code;
    // feasible values to keep the test data in the end of this file simple
    bool considered_determined        = true;
    bool considered_success_generally = false;
    bool considered_success_on_read   = false;
    bool considered_success_on_write  = false;
    bool considered_error_generally   = true;
    bool considered_error_on_read     = true;
    bool considered_error_on_write    = true;
};

class status_code_fixture: public testing::TestWithParam<status_code_test_data> {
protected:
    status_code_test_data test_data = GetParam();
};

TEST_P(status_code_fixture, has_string_representation)
{
    auto string_representation = wago::wdx::to_string(test_data.status_code);
    EXPECT_FALSE(string_representation.empty());

    auto status_code_from_string = wago::wdx::from_string(string_representation);
    EXPECT_EQ(status_code_from_string, test_data.status_code);
}

TEST_P(status_code_fixture, is_considered_determined_correctly)
{
    EXPECT_EQ(wago::wdx::is_determined(test_data.status_code), test_data.considered_determined);
}

TEST_P(status_code_fixture, is_considered_success_correctly)
{
    EXPECT_EQ(wago::wdx::is_success(test_data.status_code, wago::wdx::status_codes_context::general),           test_data.considered_success_generally);
    EXPECT_EQ(wago::wdx::is_success(test_data.status_code, wago::wdx::status_codes_context::parameter_read),    test_data.considered_success_on_read);
    EXPECT_EQ(wago::wdx::is_success(test_data.status_code, wago::wdx::status_codes_context::parameter_write),   test_data.considered_success_on_write);
}

TEST_P(status_code_fixture, is_considered_error_correctly)
{
    EXPECT_EQ(wago::wdx::is_success(test_data.status_code, wago::wdx::status_codes_context::general),           test_data.considered_success_generally);
    EXPECT_EQ(wago::wdx::is_success(test_data.status_code, wago::wdx::status_codes_context::parameter_read),    test_data.considered_success_on_read);
    EXPECT_EQ(wago::wdx::is_success(test_data.status_code, wago::wdx::status_codes_context::parameter_write),   test_data.considered_success_on_write);
}

INSTANTIATE_TEST_CASE_P(status_codes, status_code_fixture, testing::Values(
    status_code_test_data { wago::wdx::status_codes::success,                             true,  true,  true,  true, false, false, false },
    status_code_test_data { wago::wdx::status_codes::no_error_yet,                       false, false, false, false, false, false, false },
    status_code_test_data { wago::wdx::status_codes::internal_error                                                                      },
    status_code_test_data { wago::wdx::status_codes::not_implemented                                                                     },
    status_code_test_data { wago::wdx::status_codes::unknown_device_collection                                                           },
    status_code_test_data { wago::wdx::status_codes::unknown_device                                                                      },
    status_code_test_data { wago::wdx::status_codes::device_already_exists                                                               },
    status_code_test_data { wago::wdx::status_codes::device_description_inaccessible                                                     },
    status_code_test_data { wago::wdx::status_codes::device_description_parse_error                                                      },
    status_code_test_data { wago::wdx::status_codes::device_model_inaccessible                                                           },
    status_code_test_data { wago::wdx::status_codes::device_model_parse_error                                                            },
    status_code_test_data { wago::wdx::status_codes::unknown_include                                                                     },
    status_code_test_data { wago::wdx::status_codes::ambiguous_base_path                                                                 },
    status_code_test_data { wago::wdx::status_codes::unknown_parameter_id                                                                },
    status_code_test_data { wago::wdx::status_codes::parameter_already_provided                                                          },
    status_code_test_data { wago::wdx::status_codes::parameter_value_unavailable                                                         },
    status_code_test_data { wago::wdx::status_codes::parameter_not_provided                                                              },
    status_code_test_data { wago::wdx::status_codes::unknown_parameter_path                                                              },
    status_code_test_data { wago::wdx::status_codes::unknown_class_instance_path                                                         },
    status_code_test_data { wago::wdx::status_codes::not_a_method                                                                        },
    status_code_test_data { wago::wdx::status_codes::wrong_argument_count                                                                },
    status_code_test_data { wago::wdx::status_codes::could_not_set_parameter                                                             },
    status_code_test_data { wago::wdx::status_codes::missing_argument                                                                    },
    status_code_test_data { wago::wdx::status_codes::wrong_out_argument_count                                                            },
    status_code_test_data { wago::wdx::status_codes::wrong_value_type                                                                    },
    status_code_test_data { wago::wdx::status_codes::wrong_value_representation                                                          },
    status_code_test_data { wago::wdx::status_codes::could_not_invoke_method                                                             },
    status_code_test_data { wago::wdx::status_codes::provider_not_operational                                                            },
    status_code_test_data { wago::wdx::status_codes::monitoring_list_max_exceeded                                                        },
    status_code_test_data { wago::wdx::status_codes::unknown_monitoring_list                                                             },
    status_code_test_data { wago::wdx::status_codes::wrong_value_pattern                                                                 },
    status_code_test_data { wago::wdx::status_codes::parameter_not_writeable                                                             },
    status_code_test_data { wago::wdx::status_codes::value_not_possible                                                                  },
    status_code_test_data { wago::wdx::status_codes::wdmm_version_not_supported                                                          },
    status_code_test_data { wago::wdx::status_codes::invalid_device_collection                                                           },
    status_code_test_data { wago::wdx::status_codes::invalid_device_slot                                                                 },
    status_code_test_data { wago::wdx::status_codes::value_null                                                                          },
    status_code_test_data { wago::wdx::status_codes::unknown_file_id                                                                     },
    status_code_test_data { wago::wdx::status_codes::file_not_accessible                                                                 },
    status_code_test_data { wago::wdx::status_codes::invalid_value                                                                       },
    status_code_test_data { wago::wdx::status_codes::file_size_exceeded                                                                  },
    status_code_test_data { wago::wdx::status_codes::other_invalid_value_in_set                                                          },
    status_code_test_data { wago::wdx::status_codes::ignored                                                                             },
    status_code_test_data { wago::wdx::status_codes::wda_connection_changes_deferred,     true, false, false, false,  true,  true, false },
    status_code_test_data { wago::wdx::status_codes::methods_do_not_have_value                                                           },
    status_code_test_data { wago::wdx::status_codes::not_a_file_id                                                                       },
    status_code_test_data { wago::wdx::status_codes::file_id_mismatch                                                                    },
    status_code_test_data { wago::wdx::status_codes::logic_error                                                                         },
    status_code_test_data { wago::wdx::status_codes::upload_id_max_exceeded                                                              },
    status_code_test_data { wago::wdx::status_codes::status_value_unavailable,            true, false, false, false,  true,  false, true },
    status_code_test_data { wago::wdx::status_codes::unknown_enum_name                                                                   },
    status_code_test_data { wago::wdx::status_codes::unknown_feature_name                                                                },
    status_code_test_data { wago::wdx::status_codes::feature_not_available                                                               },
    status_code_test_data { wago::wdx::status_codes::instance_key_not_writeable                                                          },
    status_code_test_data { wago::wdx::status_codes::missing_parameter_for_instantiation                                                 },
    status_code_test_data { wago::wdx::status_codes::not_existing_for_instance                                                           },
    status_code_test_data { wago::wdx::status_codes::success_but_value_adjusted,          true, false, false,  true,  true,  true, false }
));
