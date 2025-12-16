//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of debug log helpers
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   Röh : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "debug_log.hpp"
#include "definitions.hpp"
#include "wda_ipc/representation.hpp"

#include <wago/wdx/requests.hpp>
#include <wago/wdx/responses.hpp>
#include <wago/wdx/file_transfer/file_transfer_definitions.hpp>
#include <wago/wdx/parameter_service_backend_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

char const sep[] = ", ";
char const bra[] = "[";
char const ket[] = "]";

// Templates for basic types
template<>
std::string to_string(bool const &value)
{
    return value == true ? "true" : "false"; // NOLINT(readability-simplify-boolean-expr)
}

template<>
std::string to_string(std::string const &str)
{
    return str;
}

template<>
std::string to_string(uint64_t const &number)
{
    return std::to_string(number);
}

template<>
std::string to_string(uint32_t const &number)
{
    return std::to_string(number);
}

template<>
std::string to_string(uint16_t const &number)
{
    return std::to_string(static_cast<unsigned>(number));
}

template<>
std::string to_string(uint8_t const &number)
{
    return std::to_string(static_cast<unsigned>(number));
}

//Templates for WDA types
template<>
std::string to_string(std::shared_ptr<wdx::parameter_value> const &value)
{
    return "wdx::parameter_value ("
          + wda_ipc::to_string(value->get_type()) 
          + (value->get_rank() == parameter_value_rank::array ? "[])" : ")");
}

template<>
std::string to_string(wdx::device_id const &id)
{
    return "wdx::device_id: " + wdx::to_short_description(id);
}

template<>
std::string to_string(wdx::parameter_filter const &filter)
{
    return "wdx::parameter_filter: " + std::string(bra)
           + "only_beta: "         + to_string(filter._only_beta)         + sep
           + "only_file_ids:"      + to_string(filter._only_file_ids)     + sep
           + "only_methods: "      + to_string(filter._only_methods)      + sep
           + "only_usersettings: " + to_string(filter._only_usersettings) + sep
           + "without_beta: "      + to_string(filter._without_beta)      + sep
           + "without_file_ids: "  + to_string(filter._without_file_ids)  + sep
           + "without_methods: "   + to_string(filter._without_methods)   + ket;
}

template<>
std::string to_string(wdx::parameter_instance_id const &instance_id)
{
    return "wdx::parameter_instance_id: " + wdx::to_short_description(instance_id);
}

template<>
std::string to_string(wdx::parameter_instance_path const &path)
{
    return "wdx::parameter_instance_path " + path.device_path + "-" + path.parameter_path;
}

template<>
std::string to_string(wdx::provider_call_mode const &mode)
{
    std::string return_string = "wdx::provider_call_mode: ";
    switch(mode)
    {
        case wdx::provider_call_mode::concurrent:
        {
            return_string += "concurrent";
            break;
        }
        case wdx::provider_call_mode::serialized:
        {
            return_string += "serialized";
            break;
        }
    }
    return return_string;
}

template<>
std::string to_string(wdx::register_device_request const &request)
{
    return "wdx::register_device_request: " + std::string(bra) + to_string(request.device_id)
           + sep +  request.firmware_version + sep + request.order_number + ket;
}

template<>
std::string to_string(wdx::value_path_request const &request)
{
  return "wdx::value_path_request: " +  std::string(bra) + to_string(request.param_path.parameter_path)+ sep
                                                         + to_string(request.param_path.device_path)   + ket;
}

template<>
std::string to_string(wdx::value_request const &request)
{
    return "wdx::value_request: " + std::string(bra) + to_string(request.param_id) + ket;
}

template<>
std::string to_string(wdx::response const &response)
{
    return "wdx::response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::device_collection_response const &response)
{
    return "wdx::device_collection_response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::device_extension_response const &response)
{
    return "wdx::device_extension_response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::device_selector_response const &response)
{
    return "wdx::device_selector_response; " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::file_transfer::file_info_response const &response)
{
    return "wdx::file_transfer::file_info_response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::file_transfer::file_id_response const &response)
{
    return "wdx::file_transfer::file_id_response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::file_transfer::file_read_response const &response)
{
    return "wdx::file_transfer::file_read_response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::method_invocation_named_response const &response)
{
    return "wdx::method_invocation_named_response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::method_invocation_response const &response)
{
    return "wdx::method_invocation_response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::parameter_response const &response)
{
    return "wdx::parameter_response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::parameter_response_list_response const &response)
{
    return "wdx::parameter_response_list_response " + wdx::to_string(response.status)
         + ", amount of parameter: " + to_string(response.param_responses.size());
}

template<>
std::string to_string(wdx::parameter_selector_response const &response)
{
    return "wdx::parameter_selector_response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::set_parameter_response const &response)
{
    return "wdx::set_parameter_response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::wdd_response const &response)
{
    return "wdx::wdd_response " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::wdm_response const &response)
{
    return "wdx::wdm_response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wdx::value_response const &response)
{
    return "wdx::value_response: " + wdx::to_string(response.status);
}

template<>
std::string to_string(wago::wdx::linuxos::com::no_return const &)
{
    return "wago::wdx::linuxos::com::no_return";
}

template<>
std::string debug_log_args_list()
{
    return "";
}

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
