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
//------------------------------------------------------------------------------
#ifndef TEST_SRC_CORE_WDM_HPP_
#define TEST_SRC_CORE_WDM_HPP_

constexpr char const test_wdm[] = R"#(
{
    "WDMMVersion": "1.3.0",
    "Name": "WAGO",
    "Classes": [
        {
            "ID": "EthernetPort",
            "BasePath": "Network/EthernetPorts",
            "BaseID": 2008,
            "Parameters": [
                {
                    "Path": "Name",
                    "ID": 1041,
                    "Type": "String"
                },
                {
                    "Path": "MacAddresses",
                    "ID": 1040,
                    "Type": "String",
                    "Rank": "Array",
                    "OnlyOnline": true
                }
            ]
        },
        {
            "ID": "IPv4",
            "BasePath": "Network/IPv4",
            "BaseID": 2009,
            "Parameters": [
                {
                    "Path": "IpAddress/Static",
                    "ID": 1038,
                    "Type": "String"
                },
                {
                    "Path": "IpAddress/Current",
                    "ID": 1037,
                    "Type": "String",
                    "OnlyOnline": true
                },
                {
                    "Path": "SubnetMask/Static",
                    "ID": 1039,
                    "Type": "String"
                },
                {
                    "Path": "SubnetMask/Current",
                    "ID": 1048,
                    "Type": "String",
                    "OnlyOnline": true
                },
                {
                    "Path": "Interface/Source",
                    "ID": 1053,
                    "Type": "Enum",
                    "Enum": "IPSources"
                }
            ]
        },
        {
            "ID": "IPService",
            "BasePath": "Services",
            "BaseID": 2010,
            "Parameters": [
                {
                    "Path": "Port",
                    "ID": 2302,
                    "Type": "UInt16"
                },
                {
                    "Path": "Enable",
                    "ID": 2300,
                    "Type": "Boolean",
                    "Writeable": true
                },
                {
                    "Path": "Name",
                    "ID": 2301,
                    "Type": "String"
                }
            ]
        },
        {
            "ID": "SshService",
            "Includes": [ "IPService" ],
            "Parameters": [
                {
                    "Path": "AllowRootLogin",
                    "ID": 2303,
                    "Type": "Boolean"
                },
                {
                    "Path": "AllowPasswordLogin",
                    "ID": 2304,
                    "Type": "Boolean"
                }
            ]
        },
        {
            "ID": "Channel",
            "BasePath": "Channels",
            "BaseID": 2006
        },
        {
            "ID": "DIChannel",
            "Includes": [ "Channel" ],
            "Parameters": [
                {
                    "Path": "Value/IoValue",
                    "ID": 1110,
                    "Type": "Boolean"
                }
            ]
        },
        {
            "ID": "DOChannel",
            "Includes": [ "Channel" ],
            "Parameters": [
                {
                    "Path": "Value/IoValue",
                    "ID": 1100,
                    "Type": "Boolean"
                }
            ]
        },
        {
            "ID": "Connector",
            "BasePath": "Connectors",
            "BaseID": 2005,
            "Parameters": [
                {
                    "Path": "CrossSection",
                    "ID": 2001,
                    "Type": "Float32"
                },
                {
                    "Path": "Name",
                    "ID": 2002,
                    "Type": "String"
                }
            ]
        },
        {
            "ID": "TestBaseClass",
            "BasePath": "TestClasses",
            "BaseID": 2007,
            "Parameters": [
                {
                    "Path": "Param1",
                    "ID": 20000,
                    "Type": "String",
                    "Writeable": false,
                    "Pattern": "Hallo|Hello"
                },
                {
                    "ID": 20008,
                    "Path": "Name",
                    "Type": "String",
                    "Writeable": false,
                    "InstanceKey": true
                }
            ]
        },
        {
            "ID": "TestClass",
            "Includes": ["TestBaseClass"],
            "Parameters": [
                {
                    "Path": "OtherParam",
                    "ID": 20001,
                    "Type": "UInt16",
                    "Unit": "mA"
                }
            ]
        },
        {
            "ID": "TestClass2",
            "BasePath": "TestClass2",
            "BaseID": 10035,
            "Parameters": [
                {
                    "Path": "ClassParamC2",
                    "ID": 10036,
                    "Type": "String"
                }
            ]
        },
        {
            "ID": "TestClass3",
            "BasePath": "TestClass3",
            "BaseID": 10039,
            "Parameters": [
                {
                    "Path": "ClassParamC3",
                    "ID": 10040,
                    "Type": "String"
                }
            ]
        },
        {
            "ID": "TestClass4",
            "BasePath": "TestClass4",
            "BaseID": 10041,
            "Parameters": [
                {
                    "Path": "ClassParamC4",
                    "ID": 10042,
                    "Type": "String"
                }
            ]
        },
        {
            "ID": "DynamicClass",
            "BasePath": "Dynamics",
            "BaseID": 100001,
            "Dynamic": true,
            "Writeable": true,
            "Parameters": [
                {
                    "Path": "Param0",
                    "ID": 20003,
                    "Type": "String",
                    "Writeable": true
                }
            ]
        },
        {
            "ID": "DynamicSpecialClass",
            "Includes": ["DynamicClass"],
            "Parameters": [
                {
                    "Path": "Param1",
                    "ID": 20002,
                    "Type": "String",
                    "Writeable": true
                }
            ]
        },
        {
            "ID": "DynamicSpecialClass2",
            "Includes": ["DynamicSpecialClass"]
        },
        {
            "ID": "DynamicReadOnlyClass",
            "BasePath": "DynamicStuff/DynamicReadonly",
            "BaseID": 100002,
            "Dynamic": true,
            "Writeable": false,
            "Parameters": [
                {
                    "Path": "SomeLevel/StatusParam",
                    "ID": 20006,
                    "Type": "String",
                    "OnlyOnline": true
                },
                {
                    "Path": "SomeLevel/Action",
                    "ID": 20007,
                    "Type": "Method"
                }
            ]
        },
        {
            "ID": "DynamicSpecialClassB",
            "Includes": ["DynamicClass"],
            "Parameters": [
                {
                    "Path": "Param2",
                    "ID": 20004,
                    "Type": "String",
                    "Writeable": true
                },
                {
                    "Path": "StaticParam3",
                    "ID": 20005,
                    "Type": "String"
                }
            ]
        },
        {
            "ID": "BetaClass",
            "BasePath": "BetaClasses",
            "BaseID": 10025,
            "Parameters": [
                {
                    "ID": 10026,
                    "Path": "BetaClassParam",
                    "Type": "String"
                }
            ]
        },
        {
            "ID": "DeprecatedClass",
            "BasePath": "DeprecatedClasses",
            "BaseID": 10028,
            "Parameters": [
                {
                    "ID": 10029,
                    "Path": "DeprecatedClassParam",
                    "Type": "String"
                }
            ]
        },
        {
            "ID": "NotDeprecatedClass",
            "BasePath": "NotDeprecatedClasses",
            "BaseID": 10030,
            "Parameters": [
                {
                    "ID": 10031,
                    "Path": "NotDeprecatedClassParam",
                    "Type": "String"
                }
            ]
        },
        {
            "ID": "DynamicClassWithInstanceKey",
            "BasePath": "DynamicClassWithInstanceKey",
            "BaseID": 60800,
            "Dynamic": true,
            "UserSetting": true,
            "Writeable": true,
            "Parameters": [
                {
                    "ID": 60801,
                    "Path": "Name",
                    "Type": "String",
                    "UserSetting": true,
                    "Writeable": true
                },
                {
                    "ID": 60802,
                    "Path": "Id",
                    "Type": "String",
                    "InstanceKey": true
                }
            ]
        },
        {
            "ID": "DynamicClassAsOwnInstanceKey",
            "BasePath": "DynamicClassAsOwnInstanceKey",
            "BaseID": 60803,
            "Dynamic": true,
            "UserSetting": true,
            "Writeable": true,
            "InstanceKey": true,
            "Parameters": [
                {
                    "ID": 60804,
                    "Path": "Name",
                    "Type": "String",
                    "UserSetting": true,
                    "Writeable": true
                }
            ]
        }
    ],
    "Features": [
        {
            "ID": "TestFeature",
            "Notes": "Nur zum Testen",
            "Classes": ["TestClass", "TestBaseClass", "DynamicClass", "DynamicSpecialClass", "DynamicSpecialClassB", "DynamicReadOnlyClass"],
            "Parameters": [
                {
                    "ID": 10000,
                    "Path": "Test/ProvidedParam",
                    "Type": "Boolean"
                },
                {
                    "ID": 10001,
                    "Path": "Test/WriteableParamWithPattern",
                    "Type": "String",
                    "Writeable": true,
                    "Pattern": "Hallo (Du|Sie)"
                },
                {
                    "ID": 10002,
                    "Path": "Test/MethodWithOutArgs",
                    "Type": "Method",
                    "InArgs": [],
                    "OutArgs": [
                        {
                            "Name": "Data",
                            "Type": "String"
                        }
                    ]
                },
                {
                    "ID": 10003,
                    "Path": "Test/EnumParam",
                    "Type": "Enum",
                    "Enum": "TestEnum",
                    "Writeable": true
                },
                {
                    "ID": 10004,
                    "Path": "Test/ReadonlyParamWithDefault",
                    "Type": "UInt16",
                    "DefaultValue": 42
                },
                {
                    "ID": 10005,
                    "Path": "Test/OverriddenByFeature",
                    "Type": "UInt16",
                    "Writeable": true,
                    "UserSetting": true
                },
                {
                    "ID": 10006,
                    "Path": "Test/OverriddenByDevice",
                    "Type": "UInt16",
                    "Writeable": true
                },
                {
                    "ID": 10007,
                    "Path": "Test/MethodWithDefaultArg",
                    "Type": "Method",
                    "InArgs": [
                        {
                            "Name": "Arg1",
                            "Type": "String"
                        },
                        {
                            "Name": "Arg2",
                            "Type": "String",
                            "DefaultValue": "MyDefault"
                        }
                    ],
                    "OutArgs": [
                        {
                            "Name": "Result",
                            "Type": "String"
                        }
                    ]
                },
                {
                    "ID": 10008,
                    "Path": "Test/ExceptionParam",
                    "Type": "String",
                    "Writeable": true
                },
                {
                    "ID": 10009,
                    "Path": "Test/InstanceRefParam",
                    "Type": "InstanceRef",
                    "RefClass": "TestClass",
                    "RefSemantics": "Organizes",
                    "Writeable": true
                },
                {
                    "ID": 10011,
                    "Path": "Test/IpParam",
                    "Type": "IPv4Address",
                    "Writeable": true,
                    "DefaultValue": "1.2.3.4"
                },
                {
                    "ID": 10012,
                    "Path": "Test/IpParamArray",
                    "Type": "IPv4Address",
                    "Rank": "Array",
                    "Writeable": true
                },
                {
                    "ID": 10013,
                    "Path": "Test/InstanceRefArray",
                    "Type": "InstanceRef",
                    "Rank": "Array",
                    "RefClass": "TestClass",
                    "RefSemantics": "Organizes",
                    "DefaultValue": [1,3,4]
                },
                {
                    "ID": 10014,
                    "Path": "Test/UserParam",
                    "Type": "String",
                    "UserSetting": true,
                    "Writeable": true
                },
                {
                    "ID": 10015,
                    "Path": "Test/InstanceRefArrayProvided",
                    "Type": "InstanceRef",
                    "Rank": "Array",
                    "RefClass": "TestClass",
                    "RefSemantics": "Organizes"
                },
                {
                    "ID": 10016,
                    "Path": "Test/ParamWithAllowedValues",
                    "Type": "Int8",
                    "Writeable": true,
                    "AllowedValues": {
                        "Min": 0,
                        "Max": 100,
                        "List": [1,2,3,5],
                        "Not": [2,4]
                    }
                },
                {
                    "ID": 10017,
                    "Path": "Test/ArrayWithAllowedValues",
                    "Type": "Int8",
                    "Rank": "Array",
                    "Writeable": true,
                    "AllowedValues": {
                        "Min": 0,
                        "Max": 100,
                        "List": [1,2,3,5],
                        "Not": [2,4]
                    }
                },
                {
                    "ID": 10018,
                    "Path": "Test/ParamWithAllowedValuesInWDD",
                    "Type": "Enum",
                    "Enum": "IPSources",
                    "Writeable": true
                },
                {
                    "ID": 10019,
                    "Path": "Test/ParamWithAllowedLengthOverriddenInWDD",
                    "Type": "String",
                    "Rank": "Array",
                    "Writeable": true,
                    "AllowedLength": {"Max": 2}
                },
                {
                    "ID": 10020,
                    "Path": "Test/ParamWithDomainSpecificError",
                    "Type": "String",
                    "Writeable": true
                },
                {
                    "ID": 10021,
                    "Path": "Test/ParamWithEvilAnswer",
                    "Type": "String"
                },
                {
                    "ID": 10022,
                    "Path": "Test/MethodWithEnumArgs",
                    "Type": "Method",
                    "InArgs": [
                        {
                            "Name": "Ping",
                            "Type": "Enum",
                            "Enum": "TestEnum"
                        }
                    ],
                    "OutArgs": [
                        {
                            "Name": "Pongs",
                            "Type": "Enum",
                            "Rank": "Array",
                            "Enum": "TestEnum"
                        },
                        {
                            "Name": "PingID",
                            "Type": "UInt16"
                        }
                    ]
                },
                {
                    "ID": 10037,
                    "Path": "Test/MethodWithInstanceRefArgs",
                    "Type": "Method",
                    "InArgs": [
                        {
                            "Name": "Ping",
                            "Type": "InstanceRef",
                            "RefClass": "TestClass"
                        }
                    ],
                    "OutArgs": [
                        {
                            "Name": "Pongs",
                            "Type": "InstanceRef",
                            "Rank": "Array",
                            "RefClass": "TestClass"
                        },
                        {
                            "Name": "PingID",
                            "Type": "UInt16"
                        }
                    ]
                },
                {
                    "ID": 10023,
                    "Path": "Test/MethodWithException",
                    "Type": "Method"
                },
                { 
                  "ID": 10027,
                  "Path": "Test/InstanceIdentity",
                  "Type": "InstanceIdentityRef",
                  "RefClasses": ["TestClass", "DynamicClassWithInstanceKey", "DynamicClassAsOwnInstanceKey"],
                  "Writeable": true,
                  "UserSetting": true
                }
            ]
        },
        {
            "ID": "FileFeature",
            "Parameters": [
                {
                    "ID": 10010,
                    "Path": "Test/FileParam",
                    "Type": "FileID",
                    "OnlyOnline": true
                }
            ]
        },
        {
            "ID": "SpecializedTestFeature",
            "Includes": ["TestFeature"],
            "Overrides": [
                {
                    "ID": 10005,
                    "Inactive": true
                }
            ]
        },
        {
            "ID": "BetaFeature",
            "Includes": ["TestFeature"],
            "Beta": true,
            "Parameters": [
                {
                    "ID": 10024,
                    "Path": "Test/BetaParam",
                    "Type": "String"
                }
            ],
            "Classes": ["BetaClass"]
        },
        {
            "ID": "DeprecatedFeature",
            "Includes": ["TestFeature"],
            "Deprecated": true,
            "Parameters": [
                {
                    "ID": 10032,
                    "Path": "Test/DeprecatedParam",
                    "Type": "String"
                }
            ],
            "Classes": ["DeprecatedClass", "NotDeprecatedClass"]
        },
        {
            "ID": "SimpleBaseFeature",
            "Parameters": [
                {
                    "ID": 10033,
                    "Path": "Base/Param",
                    "Type": "String"
                }
            ],
            "Classes": ["TestClass2"]
        },
        {
            "ID": "SimpleBaseFeature2",
            "Parameters": [
                {
                    "ID": 10038,
                    "Path": "Base2/Param",
                    "Type": "String"
                }
            ],
            "Classes": ["TestClass3", "TestClass4"]
        },
        {
            "ID": "ExtendedFeature",
            "Includes": ["SimpleBaseFeature", "SimpleBaseFeature2"],
            "Parameters": [
                {
                    "ID": 10034,
                    "Path": "Extended/Param",
                    "Type": "String"
                }
            ]
        },
        {
            "ID": "WAGODevice",
            "Parameters": [
                {
                    "Path": "Identity/OrderNumber",
                    "ID": 1,
                    "Type": "String"
                },
                {
                    "Path": "Identity/SerialNumber",
                    "ID": 1006,
                    "Type": "String",
                    "OnlyOnline": true
                },
                {
                    "Path": "Identity/Description",
                    "ID": 1001,
                    "Type": "String"
                },
                {
                    "Path": "Version/SoftwareReleaseIndex",
                    "ID": 1002,
                    "Type": "String",
                    "OnlyOnline": true
                },
                {
                    "Path": "Version/HardwareReleaseIndex",
                    "ID": 1003,
                    "Type": "String",
                    "OnlyOnline": true
                },
                {
                    "Path": "Version/SoftwareVersion",
                    "ID": 6,
                    "Type": "String",
                    "OnlyOnline": true,
                    "Pattern": "\\d\\d.\\d\\d.\\d\\d"
                },
                {
                    "Path": "System/Faulty",
                    "ID": 1014,
                    "Type": "Boolean",
                    "OnlyOnline": true
                }
            ]
        },
        {
            "ID": "HasIPv4",
            "Classes": [
                "IPv4",
                "IPService",
                "SshService"
            ],
            "Parameters": [
                {
                    "Path": "Network/DNS/StaticIPs",
                    "ID": 1080,
                    "Type": "String",
                    "Rank": "Array"
                }
            ]
        },
        {
            "ID": "HasEthernetPorts",
            "Classes": [
                "EthernetPort"
            ]
        },
        {
            "ID": "HasEthernetSwitch",
            "Includes": [ "HasEthernetPorts" ],
            "Parameters": [
                {
                    "Path": "Network/Switch/SeparateInterfaces",
                    "ID": 1035,
                    "Type": "Boolean"
                }
            ]
        },
        {
            "ID": "Headstation",
            "Includes": [ "WAGODevice" ],
            "Parameters": [
                {
                    "Path": "System/ResetDevice",
                    "ID": 4011,
                    "Type": "Method",
                    "InArgs": [],
                    "OutArgs": [
                        {
                            "Name": "WaitTime",
                            "Type": "UInt16"
                        }
                    ],
                    "StatusCodes": [
                        {
                            "Code": 1,
                            "Name": "BadInvalidState"
                        }
                    ]
                }
            ]
        },
        {
            "ID": "WAGOModule",
            "Parameters": [
                {
                    "Path": "DetachableWiringUnit",
                    "ID": 2003,
                    "Type": "Boolean"
                },
                {
                    "Path": "ManualOverrideActive",
                    "ID": 2004,
                    "Type": "Boolean",
                    "Writeable": true,
                    "OnlyOnline": true
                }
            ]
        },
        {
            "ID": "RocketLBT",
            "Includes": [ "WAGOModule" ]
        },
        {
            "ID": "IOChannels",
            "Classes": [
                "Channel",
                "DIChannel",
                "DOChannel"
            ]
        },
        {
            "ID": "HasConnectors",
            "Classes": [ "Connector" ]
        },
        {
            "ID": "GW750Tunnel",
            "Parameters": [
                {
                    "Path": "ServiceCommunication750/Protocol1288Request",
                    "ID": 4012,
                    "Type": "Method",
                    "InArgs": [
                        {
                            "Name": "RequestData",
                            "Type": "Bytes"
                        }
                    ],
                    "OutArgs": [
                        {
                            "Name": "ResponseData",
                            "Type": "Bytes"
                        }
                    ],
                    "StatusCodes": [
                        {
                            "Code": 32945,
                            "Name": "BadNoGW750Present"
                        }
                    ]
                }
            ]
        },
        {
            "ID": "DynamicClassWithInstanceKeyFeature",
            "Notes": "Test feature für InstanceKey",
            "Classes": [
                "DynamicClassWithInstanceKey"
            ]
        },
        {
            "ID": "DynamicClassAsOwnInstanceKeyFeature",
            "Notes": "Test feature für InstanceKey",
            "Classes": [
                "DynamicClassAsOwnInstanceKey"
            ]
        }
    ],
    "Enums": [
        {
            "Name": "TestEnum",
            "Members": [
                {
                    "ID": 1,
                    "Name": "Du"
                },
                {
                    "ID": 3,
                    "Name": "Ich"
                },
                {
                    "ID": 100,
                    "Name": "ProvoziertFehler"
                }
            ]
        },
        {
            "Name": "IPSources",
            "Members": [
                {
                    "ID": 1,
                    "Name": "DHCP"
                },
                {
                    "ID": 5,
                    "Name": "Static"
                }
            ]
        }
    ]
}
)#";

constexpr char const test_wdd_0750_82xx[] = R"#(
{
    "WDMMVersion": "1.0.0",
    "ModelReference": "WAGO",
    "Features": [ "Headstation", "HasIPv4", "HasEthernetSwitch", "GW750Tunnel" ],
    "Instantiations": [
        {
            "Class": "EthernetPort",
            "Instances": [
                {
                    "ID": 1,
                    "ParameterValues": [
                        {
                            "ID": 1041,
                            "Value": "X1"
                        }
                    ]    
                },
                {
                    "ID": 2,
                    "ParameterValues": [
                        {
                            "ID": 1041,
                            "Value": "X2"
                        }
                    ]
                }
            ]
        },
        {
            "Class": "IPv4",
            "Instances": [
                { "ID": 1 },
                { "ID": 2 }
            ]
        },
        {
            "Class": "IPService",
            "Instances": [
                {
                    "ID": 1,
                    "ParameterValues": [
                        {
                            "ID": 2301,
                            "Value": "Telnet"
                        }
                    ]
                },
                {
                    "ID": 2,
                    "ParameterValues": [
                        {
                            "ID": 2301,
                            "Value": "FTPS"
                        }
                    ]
                },
                {
                    "ID": 3,
                    "ParameterValues": [
                        {
                            "ID": 2301,
                            "Value": "OPC UA"
                        }
                    ]
                }
            ]
        },
        {
            "Class": "SshService",
            "Instances": [
                {
                    "ID": 4,
                    "ParameterValues": [
                        {
                            "ID": 2301,
                            "Value": "SSH"
                        }
                    ]
                }
            ]
        }
    ],
    "ParameterValues": []
}
)#";

constexpr char const test_wdd_0768_3301[] = R"#(
{
    "WDMMVersion": "1.0.0",
    "ModelReference": "WAGO",
    "Features": [ "Headstation", "HasIPv4", "HasEthernetSwitch", "GW750Tunnel", "BetaFeature", "DeprecatedFeature", "SpecializedTestFeature", "FileFeature", "DynamicClassWithInstanceKeyFeature", "DynamicClassAsOwnInstanceKeyFeature", "ExtendedFeature" ],
    "Overrides": [
        {
            "ID": 10006,
            "Inactive": true,
            "DefaultValue": 5
        },
        {
            "ID": 10018,
            "AllowedValues": {
                "List": [5]
            }
        },
        {
            "ID": 10019,
            "AllowedLength": {"Max": 1}
        },
        {
            "ID": 4011,
            "Inactive": true
        }
    ],
    "Instantiations": [
        {
            "Class": "TestBaseClass",
            "Instances": [
                {
                    "ID": 1,
                    "ParameterValues": [
                        {
                            "ID": 20008,
                            "Value": "Test1"
                        }
                    ]
                }
            ]
        },
        {
            "Class": "BetaClass",
            "Instances": [
                {
                    "ID": 1
                }
            ]
        },
        {
            "Class": "DeprecatedClass",
            "Instances": [
                {
                    "ID": 1
                }
            ]
        },
        {
            "Class": "NotDeprecatedClass",
            "Instances": [
                {
                    "ID": 1
                }
            ]
        },
        {
            "Class": "TestClass",
            "Instances": [
                {
                    "ID": 2,
                    "ParameterValues": [
                        {
                            "ID": 20008,
                            "Value": "Test2"
                        }
                    ]
                }
            ]
        },
        {
            "Class": "TestClass2",
            "Instances": [
                {
                    "ID": 1
                }
            ]
        },
        {
            "Class": "TestClass3",
            "Instances": [
                {
                    "ID": 1,
                    "ParameterValues": [
                        {
                            "ID": 10040,
                            "Value": "TestClass3Param"
                        }
                    ]
                }
            ]
        },
        {
            "Class": "TestClass4",
            "Instances": [
                {
                    "ID": 1,
                    "ParameterValues": [
                        {
                            "ID": 10042,
                            "Value": "TestClass4Param"
                        }
                    ]
                }
            ]
        },
        {
            "Class": "EthernetPort",
            "Instances": [
                {
                    "ID": 1,
                    "ParameterValues": [
                        {
                            "ID": 1041,
                            "Value": "X1"
                        }
                    ]
                },
                {
                    "ID": 2,
                    "ParameterValues": [
                        {
                            "ID": 1041,
                            "Value": "X2"
                        }
                    ]
                }
            ]
        },
        {
            "Class": "IPv4",
            "Instances": [
                { "ID": 1 },
                { "ID": 2 }
            ]
        },
        {
            "Class": "IPService",
            "Instances": [
                {
                    "ID": 1,
                    "ParameterValues": [
                        {
                            "ID": 2301,
                            "Value": "Telnet"
                        }
                    ]    
                },
                {
                    "ID": 2,
                    "ParameterValues": [
                        {
                            "ID": 2301,
                            "Value": "FTPS"
                        }
                    ]
                },
                {
                    "ID": 3,
                    "ParameterValues": [
                        {
                            "ID": 2301,
                            "Value": "OPC UA"
                        }
                    ]
                }
            ]
        },
        {
            "Class": "SshService",
            "Instances": [
                {
                    "ID": 4,
                    "ParameterValues": [
                        {
                            "ID": 2301,
                            "Value": "SSH"
                        }
                    ]
                }
            ]
        }
    ],
    "ParameterValues": [
        {
            "ID": 1,
            "Value": "0768-3301"
        },
        {
            "ID": 1001,
            "Value": "Prototype: Rocket PAC100 based on PFC firmare and runtime"
        },
        {
            "ID": 1002,
            "Value": "1"
        },
        {
            "ID": 6,
            "Value": "00.00.01"
        }
    ]
}
)#";

constexpr char const test_wdd_0763_1108[] = R"#(
{
    "WDMMVersion": "1.0.0",
    "ModelReference": "WAGO",
    "Features": ["WAGODevice", "RocketLBT", "IOChannels", "HasConnectors"],
    "Instantiations": [
        {
            "Class": "DIChannel",
            "Instances": [
                { "ID": 1 },
                { "ID": 2 },
                { "ID": 3 },
                { "ID": 4 },
                { "ID": 5 },
                { "ID": 6 },
                { "ID": 7 },
                { "ID": 8 }
            ]
        },
        {
            "Class": "Connector",
            "Instances": [
                {
                    "ID": 1,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "DI 1"
                        }
                    ]
                },
                {
                    "ID": 2,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "+"
                        }
                    ]
                },
                {
                    "ID": 3,
                    "ParameterValues":[
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "DI 2"
                        }
                    ]
                },
                {
                    "ID": 4,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "+"
                        }
                    ]
                },
                {
                    "ID": 5,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "DI 3"
                        }
                    ]
                },
                {
                    "ID": 6,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "+"
                        }
                    ]
                },
                {
                    "ID": 7,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "DI 4"
                        }
                    ]
                },
                {
                    "ID": 8,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "+"
                        }
                    ]
                },
                {
                    "ID": 9,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "DI 5"
                        }
                    ]
                },
                {
                    "ID": 10,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "+"
                        }
                    ]
                },
                {
                    "ID": 11,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "DI 6"
                        }
                    ]
                },
                {
                    "ID": 12,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "+"
                        }
                    ]
                },
                {
                    "ID": 13,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "DI 7"
                        }
                    ]
                },
                {
                    "ID": 14,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "+"
                        }
                    ]
                },
                {
                    "ID": 15,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "DI 8"
                        }
                    ]
                },
                {
                    "ID": 16,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "+"
                        }
                    ]
                }
            ]
        }
    ],
    "ParameterValues": [
        {
            "ID": 1,
            "Value": "0763-1108"
        },
        {
            "ID": 1001,
            "Value": "8DI High Speed"
        },
        {
            "ID": 2003,
            "Value": true
        }
    ]
}
)#";

constexpr char const test_wdd_0763_1508[] = R"#(
{
    "WDMMVersion": "1.0.0",
    "ModelReference": "WAGO",
    "Features": ["WAGODevice", "RocketLBT", "IOChannels", "HasConnectors"],
    "Instantiations": [
        {
            "Class": "DOChannel",
            "Instances": [
                { "ID": 1 },
                { "ID": 2 },
                { "ID": 3 },
                { "ID": 4 },
                { "ID": 5 },
                { "ID": 6 },
                { "ID": 7 },
                { "ID": 8 }
            ]
        },
        {
            "Class": "Connector",
            "Instances": [
                {
                    "ID": 1,
                    "ParameterValues": [
                        {
                            "ID": 2001,
                            "Value": 1.5
                        },
                        {
                            "ID": 2002,
                            "Value": "DO 1"
                        }
                    ]
                },
                {
                    "ID": 2,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "+"
                      }
                    ]
                },
                {
                    "ID": 3,
                    "ParameterValues":[
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "DO 2"
                      }
                    ]
                },
                {
                    "ID": 4,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "+"
                      }
                    ]
                },
                {
                    "ID": 5,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "DO 3"
                      }
                    ]
                },
                {
                    "ID": 6,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "+"
                      }
                    ]
                },
                {
                    "ID": 7,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "DO 4"
                      }
                    ]
                },
                {
                    "ID": 8,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "+"
                      }
                    ]
                },
                {
                    "ID": 9,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "DO 5"
                      }
                    ]
                },
                {
                    "ID": 10,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "+"
                      }
                    ]
                },
                {
                    "ID": 11,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "DO 6"
                      }
                    ]
                },
                {
                    "ID": 12,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "+"
                      }
                    ]
                },
                {
                    "ID": 13,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "DO 7"
                      }
                    ]
                },
                {
                    "ID": 14,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "+"
                      }
                    ]
                },
                {
                    "ID": 15,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "DO 8"
                      }
                    ]
                },
                {
                    "ID": 16,
                    "ParameterValues": [
                      {
                        "ID": 2001,
                        "Value": 1.5
                      },
                      {
                        "ID": 2002,
                        "Value": "+"
                      }
                    ]
                }
            ]
        }
    ],
    "ParameterValues": [
        {
            "ID": 1,
            "Value": "0763-1508"
        },
        {
            "ID": 1001,
            "Value": "8DO High Speed"
        },
        {
            "ID": 2003,
            "Value": true
        }
    ]
}
)#";


#endif // TEST_SRC_CORE_WDM_HPP_