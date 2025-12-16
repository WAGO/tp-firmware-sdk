//------------------------------------------------------------------------------
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
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
///  \brief    Basic test definitions for API test.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_APITEST_DEFINES_HPP_
#define SRC_APITEST_DEFINES_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/wdmm/prefix.hpp>

#include <wc/assertion.h>


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

// REST-API Test-ID definitions
constexpr char const *   const g_api_test_id_prefix_path = "APITest";

// Explicit Test-IDs for REST-API
constexpr char const * g_api_test_feature_apitest = "APITest";
constexpr parameter_id_t const g_api_test_id_ready                    = apply_prefix(prefix::apitest, 100);

constexpr char const * g_api_test_feature_examples = "Examples";
constexpr parameter_id_t const g_api_test_id_example_readonly         = apply_prefix(prefix::apitest, 1);
constexpr parameter_id_t const g_api_test_id_example_create_fileid    = apply_prefix(prefix::apitest, 2);
constexpr parameter_id_t const g_api_test_id_example_delete_fileid    = apply_prefix(prefix::apitest, 3);
constexpr parameter_id_t const g_api_test_id_example_writeflag        = apply_prefix(prefix::apitest, 4);
constexpr parameter_id_t const g_api_test_id_example_calculation      = apply_prefix(prefix::apitest, 5);

constexpr char const * g_api_test_feature_basics = "Basic";
constexpr parameter_id_t const g_api_test_id_basic_string             = apply_prefix(prefix::apitest, 101);
constexpr parameter_id_t const g_api_test_id_basic_boolean            = apply_prefix(prefix::apitest, 102);
constexpr parameter_id_t const g_api_test_id_basic_float32            = apply_prefix(prefix::apitest, 103);
constexpr parameter_id_t const g_api_test_id_basic_float64            = apply_prefix(prefix::apitest, 104);
constexpr parameter_id_t const g_api_test_id_basic_bytes              = apply_prefix(prefix::apitest, 105);
constexpr parameter_id_t const g_api_test_id_basic_uint8              = apply_prefix(prefix::apitest, 106);
constexpr parameter_id_t const g_api_test_id_basic_uint16             = apply_prefix(prefix::apitest, 107);
constexpr parameter_id_t const g_api_test_id_basic_uint32             = apply_prefix(prefix::apitest, 108);
constexpr parameter_id_t const g_api_test_id_basic_uint64             = apply_prefix(prefix::apitest, 109);
constexpr parameter_id_t const g_api_test_id_basic_int8               = apply_prefix(prefix::apitest, 110);
constexpr parameter_id_t const g_api_test_id_basic_int16              = apply_prefix(prefix::apitest, 111);
constexpr parameter_id_t const g_api_test_id_basic_int32              = apply_prefix(prefix::apitest, 112);
constexpr parameter_id_t const g_api_test_id_basic_int64              = apply_prefix(prefix::apitest, 113);
constexpr parameter_id_t const g_api_test_id_basic_enum               = apply_prefix(prefix::apitest, 114);

constexpr char const * g_api_test_feature_arrays = "Array";
constexpr parameter_id_t const g_api_test_id_array_string             = apply_prefix(prefix::apitest, 151);
constexpr parameter_id_t const g_api_test_id_array_boolean            = apply_prefix(prefix::apitest, 152);
constexpr parameter_id_t const g_api_test_id_array_float32            = apply_prefix(prefix::apitest, 153);
constexpr parameter_id_t const g_api_test_id_array_float64            = apply_prefix(prefix::apitest, 154);
constexpr parameter_id_t const g_api_test_id_array_bytes              = apply_prefix(prefix::apitest, 155);
constexpr parameter_id_t const g_api_test_id_array_uint8              = apply_prefix(prefix::apitest, 156);
constexpr parameter_id_t const g_api_test_id_array_uint16             = apply_prefix(prefix::apitest, 157);
constexpr parameter_id_t const g_api_test_id_array_uint32             = apply_prefix(prefix::apitest, 158);
constexpr parameter_id_t const g_api_test_id_array_uint64             = apply_prefix(prefix::apitest, 159);
constexpr parameter_id_t const g_api_test_id_array_int8               = apply_prefix(prefix::apitest, 160);
constexpr parameter_id_t const g_api_test_id_array_int16              = apply_prefix(prefix::apitest, 161);
constexpr parameter_id_t const g_api_test_id_array_int32              = apply_prefix(prefix::apitest, 162);
constexpr parameter_id_t const g_api_test_id_array_int64              = apply_prefix(prefix::apitest, 163);
constexpr parameter_id_t const g_api_test_id_array_enum               = apply_prefix(prefix::apitest, 164);

constexpr char const * g_api_test_feature_files = "Files";
constexpr parameter_id_t const g_api_test_id_files_static_readonly    = apply_prefix(prefix::apitest, 201);
constexpr parameter_id_t const g_api_test_id_files_static_writeable   = apply_prefix(prefix::apitest, 202);
constexpr parameter_id_t const g_api_test_id_files_static_failed_validation
                                                                      = apply_prefix(prefix::apitest, 203);

constexpr char const * g_api_test_feature_frontend_callback = "FrontendCallback";
constexpr parameter_id_t const g_api_test_id_frontend_callback_read   = apply_prefix(prefix::apitest, 301);
constexpr parameter_id_t const g_api_test_id_frontend_callback_write  = apply_prefix(prefix::apitest, 302);
constexpr parameter_id_t const g_api_test_id_frontend_callback_method = apply_prefix(prefix::apitest, 303);

constexpr char const * g_api_test_feature_file_api_callback = "FileApiCallback";
constexpr parameter_id_t const g_api_test_id_file_read_callback       = apply_prefix(prefix::apitest, 501);
constexpr parameter_id_t const g_api_test_id_file_write_callback      = apply_prefix(prefix::apitest, 502);
constexpr parameter_id_t const g_api_test_id_file_get_info_callback   = apply_prefix(prefix::apitest, 503);
constexpr parameter_id_t const g_api_test_id_file_create_callback     = apply_prefix(prefix::apitest, 504);

constexpr char const * g_api_test_feature_methods = "Methods";
constexpr parameter_id_t const g_api_test_id_methods_simple           = apply_prefix(prefix::apitest, 401);
constexpr parameter_id_t const g_api_test_id_methods_multiply         = apply_prefix(prefix::apitest, 402);
constexpr parameter_id_t const g_api_test_id_methods_not_provided     = apply_prefix(prefix::apitest, 403);
constexpr parameter_id_t const g_api_test_id_methods_wait             = apply_prefix(prefix::apitest, 404);

constexpr char const * g_api_test_feature_core_status_code = "CoreStatusCode";
constexpr parameter_id_t const g_api_test_id_core_status_code_unknown_parameter = apply_prefix(prefix::apitest, 64401); // NOT DEFINED IN MODEL!
constexpr parameter_id_t const g_api_test_id_core_status_code_not_provided      = apply_prefix(prefix::apitest, 64402);
constexpr parameter_id_t const g_api_test_id_core_status_code_not_writable      = apply_prefix(prefix::apitest, 64403);

constexpr char const * g_api_test_feature_domain_status_code = "DomainStatusCode";
constexpr parameter_id_t const g_api_test_id_domain_status_code_read   = apply_prefix(prefix::apitest, 64601);
constexpr parameter_id_t const g_api_test_id_domain_status_code_write  = apply_prefix(prefix::apitest, 64602);
constexpr parameter_id_t const g_api_test_id_domain_status_code_method = apply_prefix(prefix::apitest, 64603);

constexpr char const * g_api_test_feature_deferred_parameter = "DeferredParameter";
constexpr parameter_id_t const g_api_test_id_deferred_parameter = apply_prefix(prefix::apitest, 64701);

constexpr char const * g_api_test_feature_unavailable_status_value = "UnavailableStatusValue";
constexpr parameter_id_t const g_api_test_id_unavailable_status_value = apply_prefix(prefix::apitest, 64702);

constexpr char const * g_api_test_feature_beta = "Beta";
constexpr parameter_id_t const g_api_test_id_beta_parameter = apply_prefix(prefix::apitest, 64703);
constexpr parameter_id_t const g_api_test_id_beta_method    = apply_prefix(prefix::apitest, 64704);

constexpr char const * g_api_test_feature_deprecated = "Deprecated";
constexpr parameter_id_t const g_api_test_id_deprecated_parameter = apply_prefix(prefix::apitest, 64705);
constexpr parameter_id_t const g_api_test_id_deprecated_method    = apply_prefix(prefix::apitest, 64706);

constexpr char const * g_api_test_feature_adjusted_parameter_value = "AdjustedParameter";
constexpr parameter_id_t const g_api_test_id_adjusted_parameter_value = apply_prefix(prefix::apitest, 64707);

constexpr char const * g_api_test_feature_exception = "Exception";
constexpr parameter_id_t const g_api_test_id_exception_throw_read    = apply_prefix(prefix::apitest, 64801);
constexpr parameter_id_t const g_api_test_id_exception_throw_write   = apply_prefix(prefix::apitest, 64802);
constexpr parameter_id_t const g_api_test_id_exception_throw_method  = apply_prefix(prefix::apitest, 64803);
constexpr parameter_id_t const g_api_test_id_exception_notify_read   = apply_prefix(prefix::apitest, 64811);
constexpr parameter_id_t const g_api_test_id_exception_notify_write  = apply_prefix(prefix::apitest, 64812);
constexpr parameter_id_t const g_api_test_id_exception_notify_method = apply_prefix(prefix::apitest, 64813);

constexpr char const * g_api_test_feature_terminate = "Terminate";
constexpr parameter_id_t const g_api_test_id_terminate_concurrent_parameter_sync  = apply_prefix(prefix::apitest, 64901);
constexpr parameter_id_t const g_api_test_id_terminate_concurrent_parameter_async = apply_prefix(prefix::apitest, 64902);
constexpr parameter_id_t const g_api_test_id_terminate_concurrent_method_sync     = apply_prefix(prefix::apitest, 64903);
constexpr parameter_id_t const g_api_test_id_terminate_concurrent_method_async    = apply_prefix(prefix::apitest, 64904);
constexpr parameter_id_t const g_api_test_id_terminate_serialized_parameter_sync  = apply_prefix(prefix::apitest, 64905);
constexpr parameter_id_t const g_api_test_id_terminate_serialized_parameter_async = apply_prefix(prefix::apitest, 64906);
constexpr parameter_id_t const g_api_test_id_terminate_serialized_method_sync     = apply_prefix(prefix::apitest, 64907);
constexpr parameter_id_t const g_api_test_id_terminate_serialized_method_async    = apply_prefix(prefix::apitest, 64908);

constexpr char const * g_api_test_feature_classes = "Classes";
constexpr parameter_id_t const g_api_test_id_classes_static                                      = apply_prefix(prefix::apitest, 610);
constexpr parameter_id_t const g_api_test_id_classes_static_parameter                            = apply_prefix(prefix::apitest, 611);
constexpr parameter_id_t const g_api_test_id_classes_static_method                               = apply_prefix(prefix::apitest, 612);
constexpr parameter_id_t const g_api_test_id_classes_static_string                               = apply_prefix(prefix::apitest, 613);
constexpr parameter_id_t const g_api_test_id_classes_static_no_fourty_two_on_instance_number_two = apply_prefix(prefix::apitest, 614);
constexpr parameter_id_t const g_api_test_id_classes_dynamic                                     = apply_prefix(prefix::apitest, 620);
constexpr parameter_id_t const g_api_test_id_classes_dynamic_parameter                           = apply_prefix(prefix::apitest, 621);
constexpr parameter_id_t const g_api_test_id_classes_dynamic_method                              = apply_prefix(prefix::apitest, 622);
constexpr parameter_id_t const g_api_test_id_classes_dynamic_string                              = apply_prefix(prefix::apitest, 623);

constexpr char const * g_api_test_feature_references = "References";
constexpr parameter_id_t const g_api_test_id_references_refparam                = apply_prefix(prefix::apitest, 701);
constexpr parameter_id_t const g_api_test_id_references_danglingrefparam        = apply_prefix(prefix::apitest, 702);
constexpr parameter_id_t const g_api_test_id_references_refparamarray           = apply_prefix(prefix::apitest, 703);
constexpr parameter_id_t const g_api_test_id_references_class                   = apply_prefix(prefix::apitest, 710);
constexpr parameter_id_t const g_api_test_id_references_class_instance_key      = apply_prefix(prefix::apitest, 711);
constexpr parameter_id_t const g_api_test_id_references_otherclass              = apply_prefix(prefix::apitest, 720);
constexpr parameter_id_t const g_api_test_id_references_otherclass_instance_key = apply_prefix(prefix::apitest, 721);


} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_APITEST_DEFINES_HPP_
//---- End of source file ------------------------------------------------------
