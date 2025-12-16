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
//------------------------------------------------------------------------------

#ifndef INC_WAGO_WDX_PARAMETER_VALUE_HPP_
#define INC_WAGO_WDX_PARAMETER_VALUE_HPP_

#include <string>
#include <memory>
#include <vector>
#include <variant>
#include "wago/wdx/wdmm/base_types.hpp"
#include "wago/wdx/wdmm/class_instantiation.hpp"

namespace wago {
namespace wdx {

using json_string_t = std::string;
using bytes_t = std::vector<uint8_t>;

/**
 * Container for a value with one of the `parameter_value_types`, typically of parameter_value_rank::scalar, but can also be of parameter_value_rank::array.
 * Use one of the factory methods to create.
 */
class parameter_value
{
public:
    /** Do not use directly */
    parameter_value();
    /** Do not use directly */
    parameter_value(parameter_value const &value) = default;
    parameter_value& operator=(const parameter_value&) = default;
    parameter_value(parameter_value&&) noexcept = default;
    parameter_value& operator=(parameter_value&&) noexcept = default;
    ~parameter_value() noexcept = default;

    /** Do not use directly */
    explicit parameter_value(std::string value, parameter_value_types string_type = parameter_value_types::string);
    /** Do not use directly */
    explicit parameter_value(char const *value);
    /** Do not use directly */
    explicit parameter_value(bool value);
    /** Do not use directly */
    explicit parameter_value(uint64_t value, parameter_value_types integer_type = parameter_value_types::uint64);
    /** Do not use directly */
    explicit parameter_value(double value, parameter_value_types float_type = parameter_value_types::float64);
    /** Do not use directly */
    explicit parameter_value(bytes_t value);
    /** Do not use directly */
    explicit parameter_value(std::vector<parameter_value> value, parameter_value_types type);

    /** Creates a `parameter_value` of type `::string` */
    static std::shared_ptr<parameter_value> create(std::string value);
    /** Creates a `parameter_value` of type `::string` */
    static std::shared_ptr<parameter_value> create(char const *value);
    /** Creates a `parameter_value` of type `::boolean` */
    static std::shared_ptr<parameter_value> create(bool value);

    /** Creates a `parameter_value` of type `::string` */
    static std::shared_ptr<parameter_value> create_string(std::string value);
    /** Creates a `parameter_value` of type `::string` */
    static std::shared_ptr<parameter_value> create_string(char const *value);
    /** Creates a `parameter_value` of type `::boolean` */
    static std::shared_ptr<parameter_value> create_boolean(bool value);
    /** Creates a `parameter_value` of type `::float32` */
    static std::shared_ptr<parameter_value> create_float32(float value);
    /** Creates a `parameter_value` of type `::float64` */
    static std::shared_ptr<parameter_value> create_float64(double value);
    /** Creates a `parameter_value` of type `::uint8` */
    static std::shared_ptr<parameter_value> create_uint8(uint8_t value);
    /** Creates a `parameter_value` of type `::uint16` */
    static std::shared_ptr<parameter_value> create_uint16(uint16_t value);
    /** Creates a `parameter_value` of type `::uint32` */
    static std::shared_ptr<parameter_value> create_uint32(uint32_t value);
    /** Creates a `parameter_value` of type `::uint64` */
    static std::shared_ptr<parameter_value> create_uint64(uint64_t value);
    /** Creates a `parameter_value` of type `::int8` */
    static std::shared_ptr<parameter_value> create_int8(int8_t value);
    /** Creates a `parameter_value` of type `::int16` */
    static std::shared_ptr<parameter_value> create_int16(int16_t value);
    /** Creates a `parameter_value` of type `::int32` */
    static std::shared_ptr<parameter_value> create_int32(int32_t value);
    /** Creates a `parameter_value` of type `::int64` */
    static std::shared_ptr<parameter_value> create_int64(int64_t value);
    /** Creates a `parameter_value` of type `::ipv4address`.*/
    static std::shared_ptr<parameter_value> create_ipv4address(ipv4address address);
    /** Creates a `parameter_value` of type `::file_id`.*/
    static std::shared_ptr<parameter_value> create_file_id(file_id id);
    /** Creates a `parameter_value` of type `::instantiations`.*/
    static std::shared_ptr<parameter_value> create_instantiations(std::vector<class_instantiation> instantiations);
    /** Creates a `parameter_value` of type `::instance_ref`. The value corresponds to the id of the class instance.*/
    static std::shared_ptr<parameter_value> create_instance_ref(instance_id_t instance_id);
    /** Creates a `parameter_value` of type `::instance_ref`. The value represents an unset reference, meaning it refers to no instance. */
    static std::shared_ptr<parameter_value> create_unset_instance_ref();
    /** Creates a `parameter_value` of type `::instance_identity_ref`. The value has to be the complete path of the class instance.*/
    static std::shared_ptr<parameter_value> create_instance_identity_ref(std::string instance_path);
    /** Creates a `parameter_value` of type `::instance_identity_ref`. The ID value corresponds to the id of the class instance.*/
    static std::shared_ptr<parameter_value> create_instance_identity_ref(std::string base_path, instance_id_t instance_id);
    /** Creates a `parameter_value` of type `::instance_identity_ref`. The value represents an unset reference, meaning it refers to no instance. */
    static std::shared_ptr<parameter_value> create_unset_instance_identity_ref();
    /** Creates a `parameter_value` of type `::enum_value`. The value corresponds to the id of the member in the `enum_definition`. */
    static std::shared_ptr<parameter_value> create_enum_value(uint16_t value);
    /** Creates a `parameter_value` of type `::bytes` */
    static std::shared_ptr<parameter_value> create_bytes(bytes_t value);

    /** Creates a `parameter_value` of type `::string` and rank ::array */
    static std::shared_ptr<parameter_value> create_string_array(std::vector<std::string> const &values);
    /** Creates a `parameter_value` of type `::boolean` and rank ::array */
    static std::shared_ptr<parameter_value> create_boolean_array(std::vector<bool> const &values);
    /** Creates a `parameter_value` of type `::float32` and rank ::array */
    static std::shared_ptr<parameter_value> create_float32_array(std::vector<float> const &values);
    /** Creates a `parameter_value` of type `::float64` and rank ::array */
    static std::shared_ptr<parameter_value> create_float64_array(std::vector<double> const &values);
    /** Creates a `parameter_value` of type `::uint8` and rank ::array */
    static std::shared_ptr<parameter_value> create_uint8_array(std::vector<uint8_t> const &values);
    /** Creates a `parameter_value` of type `::uint16` and rank ::array */
    static std::shared_ptr<parameter_value> create_uint16_array(std::vector<uint16_t> const &values);
    /** Creates a `parameter_value` of type `::uint32` and rank ::array */
    static std::shared_ptr<parameter_value> create_uint32_array(std::vector<uint32_t> const &values);
    /** Creates a `parameter_value` of type `::uint64` and rank ::array */
    static std::shared_ptr<parameter_value> create_uint64_array(std::vector<uint64_t> const &values);
    /** Creates a `parameter_value` of type `::int8` and rank ::array */
    static std::shared_ptr<parameter_value> create_int8_array(std::vector<int8_t> const &values);
    /** Creates a `parameter_value` of type `::int16` and rank ::array */
    static std::shared_ptr<parameter_value> create_int16_array(std::vector<int16_t> const &values);
    /** Creates a `parameter_value` of type `::int32` and rank ::array */
    static std::shared_ptr<parameter_value> create_int32_array(std::vector<int32_t> const &values);
    /** Creates a `parameter_value` of type `::int64` and rank ::array */
    static std::shared_ptr<parameter_value> create_int64_array(std::vector<int64_t> const &values);
    /** Creates a `parameter_value` of type `::ipv4address` and rank ::array.*/
    static std::shared_ptr<parameter_value> create_ipv4address_array(std::vector<ipv4address> const &addresses);
    /** Creates a `parameter_value` of type `::file_id` and rank ::array.*/
    static std::shared_ptr<parameter_value> create_file_id_array(std::vector<file_id> const &ids);
    /** Creates a `parameter_value` of type `::instance_ref` and rank ::array. The value corresponds to the id of the member in the `enum_definition`. */
    static std::shared_ptr<parameter_value> create_instance_ref_array(std::vector<instance_id_t> const &instance_ids);
    /** Creates a `parameter_value` of type `::instance_identity_ref` and rank ::array. The value have to be complete parameter paths. */
    static std::shared_ptr<parameter_value> create_instance_identity_ref_array(std::vector<std::string> const &instance_paths);
    /** Creates a `parameter_value` of type `::instance_identity_ref` and rank ::array. The string value have to be the referenced base class path. */
    static std::shared_ptr<parameter_value> create_instance_identity_ref_array(std::vector<std::pair<std::string,instance_id_t>> const &instances);
    /** Creates a `parameter_value` of type `::enum_value` and rank ::array. The value corresponds to the id of the member in the `enum_definition`. */
    static std::shared_ptr<parameter_value> create_enum_value_array(std::vector<uint16_t> const &values);
    /** Creates a `parameter_value` of type `::bytes` and rank ::array */
    static std::shared_ptr<parameter_value> create_bytes_array(std::vector<bytes_t> const &values);

    /** Creates a `parameter_value` of given rank and type. The value is parsed from the given json string. */
    static std::shared_ptr<parameter_value> create_with_json(parameter_value_types type,
                                                            parameter_value_rank   rank,
                                                            json_string_t          value);

    /** Creates a `parameter_value` where the rank and type is not yet known (typically because it comes with no type information from IPC).
    The parameter service will later perform a lookup of the corresponding `parameter_definition` and use `set_type_internal` to qualify the value.
    */
    static std::shared_ptr<parameter_value> create_with_unknown_type(json_string_t value);

    /** Gets the type */
    parameter_value_types get_type() const;
    /** Gets the rank */
    parameter_value_rank get_rank() const;
    /** True if the rank is `::array` */
    bool is_array() const;

    /** Returns the string value if the type is `::string`. Otherwise, it will throw. */
    std::string get_string() const;
    /** Returns the bool value if the type is `::boolean`. Otherwise, it will throw. */
    bool get_boolean() const;
    /** Returns the value if the type is `::float32`. Otherwise, it will throw. */
    float get_float32() const;
    /** Returns the value if the type is `::float64`. Otherwise, it will throw. */
    double get_float64() const;
    /** Returns the value if the type is `::uint8`. Otherwise, it will throw. */
    uint8_t get_uint8() const;
    /** Returns the value if the type is `::uint16`. Otherwise, it will throw. */
    uint16_t get_uint16() const;
    /** Returns the value if the type is `::uint32`. Otherwise, it will throw. */
    uint32_t get_uint32() const;
    /** Returns the value if the type is `::uint64`. Otherwise, it will throw. */
    uint64_t get_uint64() const;
    /** Returns the value if the type is `::int8`. Otherwise, it will throw. */
    int8_t get_int8() const;
    /** Returns the value if the type is `::int16`. Otherwise, it will throw. */
    int16_t get_int16() const;
    /** Returns the value if the type is `::int32`. Otherwise, it will throw. */
    int32_t get_int32() const;
    /** Returns the value if the type is `::int64`. Otherwise, it will throw. */
    int64_t get_int64() const;
    /** Returns the value if the type is `::bytes`. Otherwise, it will throw. */
    bytes_t get_bytes() const;
    /** Returns the value if the type is `::ipv4address`. Otherwise, it will throw. */
    ipv4address get_ipv4address() const;
    /** Returns the value if the type is `::instance_ref`. Otherwise, it will throw. */
    instance_id_t get_instance_ref() const;
    /** Checks if the `instance_ref` is unset. If the type is not `::instance_ref`, it will throw. */
    bool is_unset_instance_ref() const;
    /** Returns the value if the type is `::instance_identity_ref`. Otherwise, it will throw. */
    std::string get_instance_identity_ref() const;
    /** Checks if the `instance_identity_ref` is unset. If the type is not `::instance_identity_ref`, it will throw. */
    bool is_unset_instance_identity_ref() const;
    /** Returns the value if the type is `::enum_value`. Otherwise, it will throw. */
    uint16_t get_enum_value() const;
    /** Returns the value if the type is `::file_id`. Otherwise, it will throw. */
    file_id get_file_id() const;
    /** Returns the value if the type is `::instantiations`. Otherwise, it will throw. */
    std::vector<class_instantiation> get_instantiations() const;
    
    /** Returns the list of items if the rank is `::array`. Otherwise, it will throw. */
    std::vector<parameter_value> get_items() const;
    
    /** Returns a JSON representation of the value */
    json_string_t get_json() const;

    /** Returns a description of the value */
    std::string to_string() const;
    
    bool is_numeric_value_allowed(allowed_values_spec const &allowed_values);
    
    /** NOTE: Only for internal use in the parameter service.
     * Used for `parameter_values` created by `create_with_unknown_type`, to qualify their type and rank.
     */
    void set_type_internal(parameter_value_types const &type_, parameter_value_rank const &rank_);

private:

    parameter_value_types m_type = parameter_value_types::unknown;
    parameter_value_rank m_rank = parameter_value_rank::scalar;

    json_string_t m_json_value{};

    json_string_t create_json_result() const;
    void check_type(parameter_value_types type_, parameter_value_rank rank_) const;

    // Variant to hold different value types
    std::variant<
        std::monostate,                    // for unknown/uninitialized state
        std::string,                       // for string, file_id, ipv4address, instance_identity_ref
        bool,                              // for boolean
        double,                            // for float32, float64
        uint64_t,                          // for all integer types, instance_ref, enum_member
        bytes_t,                           // for bytes
        std::vector<parameter_value>       // for arrays
    > m_value;
};

bool operator==(parameter_value const &lhs, parameter_value const &rhs) noexcept;
bool operator!=(parameter_value const &lhs, parameter_value const &rhs) noexcept;

}
}
#endif // INC_WAGO_WDX_PARAMETER_VALUE_HPP_
