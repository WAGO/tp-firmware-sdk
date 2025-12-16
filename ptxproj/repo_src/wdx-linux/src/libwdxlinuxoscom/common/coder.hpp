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
///  \brief    Central IPC coding functions.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_CODER_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_CODER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "data_stream.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"
#include "common/definitions.hpp"

#include "wda_ipc/ipc.hpp"
#include <wago/wdx/parameter_provider_i.hpp>
#include <wc/assertion.h>
#include <wc/structuring.h>
#include <wc/log.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class coder_exception : public wdx::linuxos::com::exception
{
public:
    using wdx::linuxos::com::exception::exception;
};

/// Holds static functions to encode and decode types for IPC 
struct coder final
{
public:
    // turn off any instantiations
    coder() = delete;

    /// Encode values.
    /// \param os     The output stream to put the encoded bytes into
    /// \param values Values to encode
    template <class ... Ts>
    static void encode(data_ostream &os, Ts const & ... values)
    {
        try
        {
            encode_internal(os, values...);
        }
        catch (coder_exception const&)
        {
            throw;
        }
        catch(...)
        {
            std::string error_message = "Unexpected error while encoding values";
            WC_FAIL(error_message.c_str());
            throw wdx::linuxos::com::coder_exception(error_message);
        }
    }

    /// Decode values.
    /// \param is     The input stream to get the decodable bytes from
    /// \param values Values to decode
    template <class ... Ts>
    static void decode(data_istream &is, Ts & ... values)
    {
        try
        {
            decode_internal(is, values...);
        }
        catch (coder_exception const&)
        {
            throw;
        }
        catch(...)
        {
            std::string error_message = "Unexpected error while decoding values";
            WC_FAIL(error_message.c_str());
            throw wdx::linuxos::com::coder_exception(error_message);
        }
    }
    
private:
    // Use own serialization for scalar and string types
    // Performance improvement: Use own serialization for uint8 vectors used for file content
    template<class T>
    using use_wda_serialization = std::enable_if_t<
            (!std::is_scalar<T>::value) &&
            (!std::is_same<T, std::string>::value) &&
            (!std::is_same<T, std::vector<uint8_t>>::value) &&
            (!std::is_same<T, std::vector<std::string>>::value)
        , int>;

    inline static void encode_internal(data_ostream &) { /* noop */ }
    inline static void decode_internal(data_istream &) { /* noop */ }

    /// Encode two or more values at once.
    /// \param os     The output stream to put the encoded bytes into
    /// \param first  A value to encode
    /// \param second Another value to encode
    /// \param rest   Even more values to encode
    template <class T, class Tn, class ... Ts>
    static void encode_internal(data_ostream &os, T const &first, Tn const &second, Ts const & ... rest)
    {
        encode_internal(os, first);
        encode_internal(os, second, rest...);
    }

    /// Decode two or more values at once.
    /// \param is     The input stream to get the decodable bytes from
    /// \param first  A value to decode
    /// \param second Another value to decode
    /// \param rest   Even more values to decode
    template <class T, class Tn, class ... Ts>
    static void decode_internal(data_istream &is, T &first, Tn &second, Ts & ... rest)
    {
        decode_internal(is, first);
        decode_internal(is, second, rest...);
    }

    /// Encode an empty value.
    /// \param os    The output stream to put the encoded bytes into
    /// \param value A value to encode
    static void encode_internal(data_ostream &os, no_return const &)
    {
        encode_internal(os);
    }

    /// Decode an empty value.
    /// \param is    The input stream to get the decodable bytes from
    /// \param value A value to decode
    static void decode_internal(data_istream &is, no_return &)
    {
        decode_internal(is);
    }

    /// Encode a string value.
    /// \param os    The output stream to put the encoded bytes into
    /// \param value A value to encode
    static void encode_internal(data_ostream &os, std::string const &value)
    {
        size_t const string_length = value.length();
        encode_internal<uint32_t>(os, string_length);
        WC_ASSERT(string_length <= static_cast<uint64_t>(std::numeric_limits<std::streamsize>::max()));
        os.write(reinterpret_cast<uint8_t const *>(value.data()), static_cast<std::streamsize>(string_length)); // parasoft-suppress CERT_C-EXP39-b-3 "Coder works with uint8_t while string works on char."
    }

    /// Decode a string value.
    /// \param is    The input stream to get the decodable bytes from
    /// \param value A value to decode
    static void decode_internal(data_istream &is, std::string &value)
    {
        uint32_t string_length;
        decode_internal(is, string_length);
        uint32_t chars_to_read = string_length;
        value = "";
        uint8_t buffer[4096];
        uint32_t buffer_size = sizeof(buffer);
        uint32_t chunk_size  = 0;
        do
        {
            chunk_size = std::min(buffer_size, chars_to_read);
            WC_ASSERT(chunk_size <= static_cast<uint64_t>(std::numeric_limits<std::streamsize>::max()));
            if(is.read(WC_ARRAY_TO_PTR(buffer), static_cast<std::streamsize>(chunk_size)))
            {
                value.append(reinterpret_cast<char *>(buffer), chunk_size);
            }
            chars_to_read -= chunk_size;
        }
        while(chars_to_read > 0);
        WC_ASSERT(value.c_str()[value.length()] == '\0'); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        WC_ASSERT(string_length == value.length());
    }

    /// Encode a scalar value.
    /// \param os    The output stream to put the encoded bytes into
    /// \param value A value to encode
    template<typename T,
             std::enable_if_t<std::is_scalar<T>::value, int> = 0>
    static void encode_internal(data_ostream &os, T const &value)
    {
        os.write(reinterpret_cast<uint8_t const *>(&value), sizeof(value));
    }

    /// Decode a scalar value.
    /// \param is    The input stream to get the decodable bytes from
    /// \param value A value to decode
    template<typename T,
             std::enable_if_t<std::is_scalar<T>::value, int> = 0>
    static void decode_internal(data_istream &is, T &value)
    {
        if(!is.read(reinterpret_cast<uint8_t *>(&value), sizeof(value)))
        {
            std::string error_message = "Unexpected error while decoding values";
            WC_FAIL(error_message.c_str());
            throw coder_exception(error_message);
        }
    }

    /// Encode a vector of values
    /// \param os    The output stream to put the encoded bytes into
    /// \param value A value to encode
    template<typename T,
             std::enable_if_t<!std::is_scalar<T>::value, int> = 0>
    static void encode_internal(data_ostream &os, std::vector<T> const &value)
    {
        WC_STATIC_ASSERT(sizeof(value.size()) <= sizeof(uint64_t));
        uint64_t const size = value.size();
        encode_internal(os, size);
        for (T const & element : value)
        {
            encode_internal(os, element);
        }
    }

    /// Decode a vector of values.
    /// \param is    The input stream to get the decodable bytes from
    /// \param value A value to decode
    template<typename T,
             std::enable_if_t<!std::is_scalar<T>::value, int> = 0>
    static void decode_internal(data_istream &is, std::vector<T> &value)
    {
        uint64_t size;
        decode_internal(is, size);
        value = std::vector<T>(static_cast<size_t>(size));
        for (T & element : value)
        {
            decode_internal(is, element);
        }
    }

    /// Encode a vector of scalar values
    /// \param os    The output stream to put the encoded bytes into
    /// \param value A value to encode
    template<typename T,
             std::enable_if_t<std::is_scalar<T>::value, int> = 0>
    static void encode_internal(data_ostream &os, std::vector<T> const &value)
    {
        WC_STATIC_ASSERT(sizeof(value.size()) <= sizeof(uint64_t));
        uint64_t const size = value.size();
        encode_internal(os, size);
        auto const data_size = sizeof(T) * value.size();
        WC_ASSERT(data_size <= static_cast<uint64_t>(std::numeric_limits<std::streamsize>::max()));
        // Performance improvement: Use direct data access for values to boost up file transfer case
        os.write(reinterpret_cast<uint8_t const *>(value.data()), static_cast<std::streamsize>(data_size));
    }

    /// Decode a vector of scalar values.
    /// \param is    The input stream to get the decodable bytes from
    /// \param value A value to decode
    template<typename T,
             std::enable_if_t<std::is_scalar<T>::value, int> = 0>
    static void decode_internal(data_istream &is, std::vector<T> &value)
    {
        uint64_t size;
        decode_internal(is, size);
        value = std::vector<T>(static_cast<size_t>(size));
        auto const data_size = sizeof(T) * size;
        WC_ASSERT(data_size <= static_cast<uint64_t>(std::numeric_limits<std::streamsize>::max()));
        // Performance improvement: Use direct data access for values to boost up file transfer case
        is.read(reinterpret_cast<uint8_t *>(value.data()), static_cast<std::streamsize>(data_size));
    }

    /// Encode a map.
    /// \param os    The output stream to put the encoded bytes into
    /// \param value A value to encode
    template<typename K,
             typename V>
    static void encode_internal(data_ostream &os, std::map<K,V> const &value)
    {
        WC_ASSERT(value.size() <= UINT32_MAX);
        encode_internal(os, static_cast<uint32_t>(value.size()));
        for(auto const &element : value)
        {
            encode_internal(os, element.first);
            encode_internal(os, element.second);
        }
    }

    /// Decode a map.
    /// \param is    The input stream to get the decodable bytes from
    /// \param value A value to decode
    template<typename K,
             typename V>
    static void decode_internal(data_istream &is, std::map<K,V> &value)
    {
        uint32_t size;
        decode_internal(is, size);
        value = std::map<K,V>();
        for(size_t i = 0; i < size; ++i)
        {
            K element_key;
            V element_value;
            decode_internal(is, element_key);
            decode_internal(is, element_value);
            value[element_key] = element_value;
        }
    }

    /// Encode a value hold by a unique pointer.
    /// \param os    The output stream to put the encoded bytes into
    /// \param value A value to encode
    template<typename T>
    static void encode_internal(data_ostream &os, std::unique_ptr<T> const &value)
    {
        encode_internal(os, *value.get());
    }

    /// Decode a value hold by a unique pointer.
    /// \param is    The input stream to get the decodable bytes from
    /// \param value A value to decode
    template<typename T>
    static void decode_internal(data_istream &is, std::unique_ptr<T> &value)
    {
        value = std::make_unique<T>();
        decode_internal(is, *value.get());
    }

    /// Encode a file response value.
    /// \note        Performance improvement for file transfer case
    /// \param os    The output stream to put the encoded bytes into
    /// \param value A file response value to encode
    static void encode_internal(data_ostream &os, wdx::file_read_response const &value)
    {
        encode_internal(os, value.status);
        encode_internal(os, value.data);
    }

    /// Decode a file response value.
    /// \note        Performance improvement for file transfer case
    /// \param is    The input stream to get the decodable bytes from
    /// \param value A file response value to decode
    static void decode_internal(data_istream &is, wdx::file_read_response &value)
    {
        decode_internal(is, value.status);
        decode_internal(is, value.data);
    }

    /// Encode a value by using WDA specific IPC conversion functions.
    /// \param os    The output stream to put the encoded bytes into
    /// \param value A value to encode
    template<typename T, 
             use_wda_serialization<T> = 0>
    static void encode_internal(data_ostream &os, T const &value)
    {
        wdx::bytes_t bytes;
        try
        {
            bytes = wda_ipc::to_bytes(value);
        }
        catch(...)
        {
            std::string error_message = std::string("Unexpected error while encoding core type ") + typeid(T).name();
            WC_FAIL(error_message.c_str());
            throw wdx::linuxos::com::coder_exception(error_message);
        }
        encode_internal(os, bytes);
    }

    /// Decode a value by using WDA specific IPC conversion functions.
    /// \param is    The input stream to get the decodable bytes from
    /// \param value A value to decode
    template<typename T,
             use_wda_serialization<T> = 0>
    static void decode_internal(data_istream &is, T &value)
    {
        std::vector<uint8_t> bytes_value;
        decode_internal(is, bytes_value);
        try
        {
            value = wda_ipc::from_bytes<T>(bytes_value);
        }
        catch(std::exception const &e)
        {
            std::string error_message = std::string("Unexpected error while decoding core type ") + typeid(T).name() + ": " + e.what();
            WC_FAIL(error_message.c_str());
            WC_DEBUG_LOG(("bytes_value=" + std::string(reinterpret_cast<char*>(bytes_value.data()))).c_str());
            throw wdx::linuxos::com::coder_exception(error_message);
        }
    }
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_CODER_HPP_
//---- End of source file ------------------------------------------------------
