//------------------------------------------------------------------------------
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
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
///  \brief    Test how "Overrides" should behave in complex class hierarchies.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "loader/device_model_loader.hpp"
#include "loader/device_description_loader.hpp"
#include "instances/device.hpp"
#include "parameter_service_core.hpp"
#include "common/log.hpp"

#include <gtest/gtest.h>
#include <string>
#include <memory>

//------------------------------------------------------------------------------
// constants, defines and functions
//------------------------------------------------------------------------------

using namespace wago::wdx;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
struct test_overrides_fixture : public ::testing::Test
{
private:

    //       O
    //       |
    //       A
    //      / \
    //     B   C
    //    /|  /
    //   D-X /
    //    \ /
    //     X
    // Rule 1) Overrides have precedence if they're in a more specific class
    // Rule 2) Overrides have precedence if they're in a class which is listed 
    //         earlier in an "Include" field than another
    // Rule 3) In case of conflicting precedence, Rule 1 applies in favor of 
    //         Rule 2.
    // Rule 4) Classes named in WDD-defined instances are considered to be
    //         processed in the same order as if they were listed in the 
    //         "Include" field of a hypothetic class, where "AdditionalClasses"
    //         are appended right after "Class". This is compatible to Rule 1-3.
    // Rule 5) Overrides listed in the global "Overrides" field of a WDD have
    //         precedence over Rule 1-3.
    // Rule 6) Overrides listed in the specific "Overrides" field for an
    //         instance defined in a WDD have precedence over Rule 1-4.
    std::string test_wdm = R"#(
        {
            "Name": "Test",
            "WDMMVersion": "1.0.0",
            "Features": [
                {
                    "ID": "ClassHierarchyWithOverrides",
                    "Classes": ["O", "A", "B", "C", "D", "X_CD", "X_DC", "X_BD", "X_DB"]
                }
            ],
            "Classes": [
                {
                    "ID": "O",
                    "BaseID": 1,
                    "BasePath": "Tests",
                    "Parameters": [
                        {
                            "ID": 11,
                            "Path": "Param",
                            "Type": "String",
                            "DefaultValue": "O",
                            "Pattern": "O"
                        }
                    ]
                },
                {
                    "ID": "A",
                    "Includes": ["O"],
                    "Overrides": [
                        {
                            "ID": 11,
                            "DefaultValue": "A",
                            "Pattern": "A"
                        }
                    ]
                },
                {
                    "ID": "B",
                    "Includes": ["A"],
                    "Overrides": [
                        {
                            "ID": 11,
                            "DefaultValue": "B"
                        }
                    ]
                },
                {
                    "ID": "C",
                    "Includes": ["A"],
                    "Overrides": [
                        {
                            "ID": 11,
                            "DefaultValue": "C",
                            "Pattern": "C"
                        }
                    ]
                },
                {
                    "ID": "D",
                    "Includes": ["B"],
                    "Overrides": [
                        {
                            "ID": 11,
                            "DefaultValue": "D"
                        }
                    ]
                },
                {
                    "ID": "X_A",
                    "Includes": ["A"]
                },
                {
                    "ID": "X_B",
                    "Includes": ["B"]
                },
                {
                    "ID": "X_CD",
                    "Includes": ["C", "D"]
                },
                {
                    "ID": "X_DC",
                    "Includes": ["D", "C"]
                },
                {
                    "ID": "X_BD",
                    "Includes": ["B", "D"]
                },
                {
                    "ID": "X_DB",
                    "Includes": ["D", "B"]
                }
            ]
        }
    )#";
    std::string test_wdd = R"#(
        {
            "WDMMVersion": "1.0.0",
            "ModelReference": "Test",
            "Features": [ "ClassHierarchyWithOverrides" ],
            "Instantiations": [
                {
                    "Class": "O",
                    "Instances": [
                        { "ID": 1 }
                    ]
                },
                {
                    "Class": "A",
                    "Instances": [
                        { "ID": 2 }
                    ]
                },
                {
                    "Class": "B",
                    "Instances": [
                        { "ID": 3 }
                    ]
                },
                {
                    "Class": "C",
                    "Instances": [
                        { "ID": 4 }
                    ]
                },
                {
                    "Class": "D",
                    "Instances": [
                        { "ID": 5 }
                    ]
                },
                {
                    "Class": "X_A",
                    "Instances": [
                        { "ID": 6 }
                    ]
                },
                {
                    "Class": "X_B",
                    "Instances": [
                        { "ID": 7 }
                    ]
                },
                {
                    "Class": "X_CD",
                    "Instances": [
                        { "ID": 8 }
                    ]
                },
                {
                    "Class": "X_DC",
                    "Instances": [
                        { "ID": 9 }
                    ]
                },
                {
                    "Class": "X_BD",
                    "Instances": [
                        { "ID": 10 }
                    ]
                },
                {
                    "Class": "X_DB",
                    "Instances": [
                        { "ID": 11 }
                    ]
                },
                {
                    "Class": "O",
                    "AdditionalClasses": ["A"],
                    "Instances": [
                        { "ID": 12 }
                    ]
                },
                {
                    "Class": "O",
                    "AdditionalClasses": ["B"],
                    "Instances": [
                        { "ID": 13 }
                    ]
                },
                {
                    "Class": "O",
                    "AdditionalClasses": ["C", "D"],
                    "Instances": [
                        { "ID": 14 }
                    ]
                },
                {
                    "Class": "O",
                    "AdditionalClasses": ["D", "C"],
                    "Instances": [
                        { "ID": 15 }
                    ]
                },
                {
                    "Class": "O",
                    "AdditionalClasses": ["B", "D"],
                    "Instances": [
                        { "ID": 16 }
                    ]
                },
                {
                    "Class": "O",
                    "AdditionalClasses": ["D", "B"],
                    "Instances": [
                        { "ID": 17 }
                    ]
                },
                {
                    "Class": "A",
                    "AdditionalClasses": [
                        "B"
                    ],
                    "Instances": [
                        {
                            "ID": 18
                        }
                    ]
                },
                {
                    "Class": "A",
                    "AdditionalClasses": [
                        "C"
                    ],
                    "Instances": [
                        {
                            "ID": 19
                        }
                    ]
                },
                {
                    "Class": "B",
                    "AdditionalClasses": [
                        "A"
                    ],
                    "Instances": [
                        {
                            "ID": 20
                        }
                    ]
                },
                {
                    "Class": "C",
                    "AdditionalClasses": [
                        "A"
                    ],
                    "Instances": [
                        {
                            "ID": 21
                        }
                    ]
                },
                {
                    "Class": "O",
                    "Instances": [
                        {
                            "ID": 22,
                            "Overrides": [
                                {
                                    "ID": 11,
                                    "DefaultValue": "I"
                                }
                            ]
                        }
                    ]
                },
                {
                    "Class": "O",
                    "AdditionalClasses": [
                        "A"
                    ],
                    "Instances": [
                        {
                            "ID": 23,
                            "Overrides": [
                                {
                                    "ID": 11,
                                    "DefaultValue": "I"
                                }
                            ]
                        }
                    ]
                },
                {
                    "Class": "A",
                    "Instances": [
                        {
                            "ID": 24,
                            "Overrides": [
                                {
                                    "ID": 11,
                                    "DefaultValue": "I"
                                }
                            ]
                        }
                    ]
                }
            ]
        }
    )#";

    std::string test_device_ordernumber = "0123-4567";
    std::string test_device_fwversion   = "01.23.45";

    // Live model
    std::unique_ptr<device_model> test_device_model;

public:

    // Test subject
    std::unique_ptr<device>       test_device;

    // actual instance paths as given by WDD instantiations
    std::string i1_O     = "Tests/1/Param";
    std::string i2_A     = "Tests/2/Param";
    std::string i3_B     = "Tests/3/Param";
    std::string i4_C     = "Tests/4/Param";
    std::string i5_D     = "Tests/5/Param";
    std::string i6_X_A   = "Tests/6/Param";
    std::string i7_X_B   = "Tests/7/Param";
    std::string i8_X_CD  = "Tests/8/Param";
    std::string i9_X_DC  = "Tests/9/Param";
    std::string i10_X_BD = "Tests/10/Param";
    std::string i11_X_DB = "Tests/11/Param";
    std::string i12_O_A  = "Tests/12/Param";
    std::string i13_O_B  = "Tests/13/Param";
    std::string i14_O_CD = "Tests/14/Param";
    std::string i15_O_DC = "Tests/15/Param";
    std::string i16_O_BD = "Tests/16/Param";
    std::string i17_O_DB = "Tests/17/Param";
    std::string i18_A_B  = "Tests/18/Param";
    std::string i19_A_C  = "Tests/19/Param";
    std::string i20_B_A  = "Tests/20/Param";
    std::string i21_C_A  = "Tests/21/Param";
    std::string i22_I_O  = "Tests/22/Param";
    std::string i23_I_O_A= "Tests/23/Param";
    std::string i24_I_A  = "Tests/24/Param";

    virtual void SetUp() override 
    {
        current_log_mode = lax;

        // Load WDM
        test_device_model = std::make_unique<device_model>();
        device_model_loader wdm_loader;
        wdm_loader.load(test_wdm, *test_device_model);

        // Create Device
        test_device = std::make_unique<device>(device_id(0, 0), test_device_ordernumber, test_device_fwversion);

        // Load WDD
        device_description_loader wdd_loader(*test_device_model, *test_device);
        wdd_loader.load(test_wdd);
    }

    virtual void TearDown() override 
    {
        test_device_model.release();
        test_device.release();
    }

    void test_default_value_is_correct(std::string instance_path, std::string expected_value)
    {
        auto param_instance = test_device->parameter_instances.get_instance(instance_path);
        ASSERT_NE(param_instance, nullptr);

        auto default_value  = param_instance->definition->overrideables.default_value;
        EXPECT_EQ(default_value->get_string(), expected_value);
    }

    void test_pattern_is_correct(std::string instance_path, std::string expected_value)
    {
        auto param_instance = test_device->parameter_instances.get_instance(instance_path);
        ASSERT_NE(param_instance, nullptr);

        auto pattern  = param_instance->definition->overrideables.pattern;
        EXPECT_EQ(pattern, expected_value);
    }
};

TEST_F(test_overrides_fixture, i01_O_has_correct_overrides)
{
    test_default_value_is_correct(i1_O, "O");
    test_pattern_is_correct(i1_O, "O");
}

TEST_F(test_overrides_fixture, i02_A_has_correct_overrides)
{
    test_default_value_is_correct(i2_A, "A");
    test_pattern_is_correct(i2_A, "A");
}

TEST_F(test_overrides_fixture, i03_B_has_correct_overrides)
{
    test_default_value_is_correct(i3_B, "B");
    test_pattern_is_correct(i3_B, "A");
}

TEST_F(test_overrides_fixture, i04_C_has_correct_overrides)
{
    test_default_value_is_correct(i4_C, "C");
    test_pattern_is_correct(i4_C, "C");
}

TEST_F(test_overrides_fixture, i05_D_has_correct_overrides)
{
    test_default_value_is_correct(i5_D, "D");
    test_pattern_is_correct(i5_D, "A");
}

TEST_F(test_overrides_fixture, i06_X_A_has_correct_overrides)
{
    test_default_value_is_correct(i6_X_A, "A");
    test_pattern_is_correct(i6_X_A, "A");
}


TEST_F(test_overrides_fixture, i07_X_B_has_correct_overrides)
{
    test_default_value_is_correct(i7_X_B, "B");
    test_pattern_is_correct(i7_X_B, "A");
}


TEST_F(test_overrides_fixture, i08_X_CD_has_correct_overrides)
{
    test_default_value_is_correct(i8_X_CD, "C");
    test_pattern_is_correct(i8_X_CD, "C");
}

TEST_F(test_overrides_fixture, i09_X_DC_has_correct_overrides)
{
    test_default_value_is_correct(i9_X_DC, "D");
    test_pattern_is_correct(i9_X_DC, "C");
}

TEST_F(test_overrides_fixture, i10_X_BD_has_correct_overrides)
{
    test_default_value_is_correct(i10_X_BD, "D");
    test_pattern_is_correct(i10_X_BD, "A");
}

TEST_F(test_overrides_fixture, i11_X_DB_has_correct_overrides)
{
    test_default_value_is_correct(i11_X_DB, "D");
    test_pattern_is_correct(i11_X_DB, "A");
}

TEST_F(test_overrides_fixture, i12_O_A_has_correct_overrides)
{
    test_default_value_is_correct(i12_O_A, "A");
    test_pattern_is_correct(i12_O_A, "A");
}

TEST_F(test_overrides_fixture, i13_O_B_has_correct_overrides)
{
    test_default_value_is_correct(i13_O_B, "B");
    test_pattern_is_correct(i13_O_B, "A");
}

TEST_F(test_overrides_fixture, i14_O_CD_has_correct_overrides)
{
    test_default_value_is_correct(i14_O_CD, "C");
    test_pattern_is_correct(i14_O_CD, "C");
}

TEST_F(test_overrides_fixture, i15_O_DC_has_correct_overrides)
{
    test_default_value_is_correct(i15_O_DC, "D"); 
    test_pattern_is_correct(i15_O_DC, "C");
}

TEST_F(test_overrides_fixture, i16_O_BD_has_correct_overrides)
{
    test_default_value_is_correct(i16_O_BD, "D");
    test_pattern_is_correct(i16_O_BD, "A");
}

TEST_F(test_overrides_fixture, i17_O_DB_has_correct_overrides)
{
    test_default_value_is_correct(i17_O_DB, "D");
    test_pattern_is_correct(i17_O_DB, "A");
}

TEST_F(test_overrides_fixture, i18_A_B_has_correct_overrides)
{
    test_default_value_is_correct(i18_A_B, "B");
    test_pattern_is_correct(i18_A_B, "A");
}

TEST_F(test_overrides_fixture, i19_A_C_has_correct_overrides)
{
    test_default_value_is_correct(i19_A_C, "C"); 
    test_pattern_is_correct(i19_A_C, "C");
}

TEST_F(test_overrides_fixture, i20_B_A_has_correct_overrides)
{
    test_default_value_is_correct(i20_B_A, "B");
    test_pattern_is_correct(i20_B_A, "A");
}

TEST_F(test_overrides_fixture, i21_C_A_has_correct_overrides)
{
    test_default_value_is_correct(i21_C_A, "C");
    test_pattern_is_correct(i21_C_A, "C");
}

TEST_F(test_overrides_fixture, i22_I_O_has_correct_overrides)
{
    test_default_value_is_correct(i22_I_O, "I");
    test_pattern_is_correct(i22_I_O, "O");
}

TEST_F(test_overrides_fixture, i23_I_O_A_has_correct_overrides)
{
    test_default_value_is_correct(i23_I_O_A, "I");
    test_pattern_is_correct(i23_I_O_A, "A");
}

TEST_F(test_overrides_fixture, i24_I_A_has_correct_overrides)
{
    test_default_value_is_correct(i24_I_A, "I");
    test_pattern_is_correct(i24_I_A, "A");
}
