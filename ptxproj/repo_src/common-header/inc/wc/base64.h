//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project common-header.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Base64 encoding and decoding functions
///
///  \author   Mahe : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WC_BASE64_H_
#define INC_WC_BASE64_H_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/std_includes.h>
#include <wc/assertion.h>
#include <wc/compiler.h>

#ifdef __cplusplus
#include <string>
#include <vector>
#include <stdexcept>
#endif // __cplusplus

#include <ctype.h>
#include <string.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
/// \def WC_BASE64_UNENCODED_MAX
/// Macro defining maximum length of unencoded strings.
///
/// \note Do not use directly but function
///       \link wc_get_max_base64_unencoded_length \endlink instead.
///
/// \see wc_get_max_base64_unencoded_length
#define WC_BASE64_UNENCODED_MAX                             (SIZE_MAX / 4u * 3u)

/// \def WC_BASE64_ENCODED_MAX
/// Macro defining maximum length of encoded strings.
///
/// \note Do not use directly but function
///       \link wc_get_max_base64_encoded_length \endlink instead.
///
/// \see wc_get_max_base64_encoded_length
#define WC_BASE64_ENCODED_MAX                                    (SIZE_MAX - 3u)

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    /// Encode the given data to a Base64 string into the provided buffer. 
    /// The buffer must be large enough to hold encoded data.
    ///
    /// \note `4*(n/3)` chars are required to represent n bytes.
    ///       If not URL safe encoded also rounded up to a multiple of 4.
    ///       See: \link wc_base64_calc_encoded_length \endlink
    ///       See: \link wc_get_max_base64_unencoded_length \endlink
    ///
    /// \note The resulting string DOES NOT include the null terminator. 
    ///       You may set it on your own by setting the last byte to '\0':
    ///       \code decoded[decoded_length] = '\0'; \endcode 
    ///
    /// \param bytes
    ///   The data to be encoded.
    /// \param bytes_length 
    ///   The length of the data to be encoded.
    /// \param url_safe
    ///   Use Base64URL encoding with URL-safe charset and without padding.
    /// \param encoded
    ///   The buffer to hold the Base64-encoded representation of the data.
    /// \param encoded_length
    ///   The length of the buffer to hold the Base64-encoded representation of
    ///   the data (without terminating null).
    ///
    /// \return 
    ///   True on success. False otherwise.
    ///
    /// \see wc_base64_encode
    /// \see wc_base64_encode_for_url
    /// \see wc_base64_calc_encoded_length
    /// \see wc_get_max_base64_unencoded_length
    //------------------------------------------------------------------------------
    static inline bool wc_base64_encode_selected(uint8_t const *bytes,
                                                 size_t         bytes_length,
                                                 bool           url_safe,
                                                 char          *encoded,
                                                 size_t        *encoded_length);

    /// Encode the given data to a Base64 string into the provided buffer. 
    /// The buffer must be large enough to hold encoded data.
    ///
    /// \note `4*(n/3)` chars are required to represent n bytes, rounded up to a
    ///       multiple of 4.
    ///       See: \link wc_base64_calc_encoded_length \endlink
    ///       See: \link wc_get_max_base64_unencoded_length \endlink
    ///
    /// \note The resulting string DOES NOT include the null terminator. 
    ///       You may set it on your own by setting the last byte to '\0':
    ///       \code decoded[decoded_length] = '\0'; \endcode 
    ///
    /// \param bytes
    ///   The data to be encoded.
    /// \param bytes_length 
    ///   The length of the data to be encoded.
    /// \param encoded
    ///   The buffer to hold the Base64-encoded representation of the data.
    /// \param encoded_length
    ///   The length of the buffer to hold the Base64-encoded representation of
    ///   the data (without terminating null).
    ///
    /// \return 
    ///   True on success. False otherwise.
    ///
    /// \see wc_base64_encode_for_url
    /// \see wc_base64_calc_encoded_length
    /// \see wc_get_max_base64_unencoded_length
    //------------------------------------------------------------------------------
    static inline bool wc_base64_encode(uint8_t const *bytes,
                                        size_t         bytes_length,
                                        char          *encoded,
                                        size_t        *encoded_length)
    {
        return wc_base64_encode_selected(bytes, bytes_length, false, encoded, encoded_length);
    }

    /// Encode the given data a Base64URL string into the provided buffer. 
    /// The buffer must be large enough to hold encoded data.
    ///
    /// \note `4*(n/3)` chars are required to represent n bytes.
    ///       See: \link wc_base64_calc_encoded_length \endlink
    ///       See: \link wc_get_max_base64_unencoded_length \endlink
    ///
    /// \note The resulting string DOES NOT include the null terminator. 
    ///       You may set it on your own by setting the last byte to '\0':
    ///       \code decoded[decoded_length] = '\0'; \endcode 
    ///
    /// \param bytes
    ///   The data to be encoded.
    /// \param bytes_length 
    ///   The length of the data to be encoded.
    /// \param encoded
    ///   The buffer to hold the Base64-encoded representation of the data.
    /// \param encoded_length
    ///   The length of the buffer to hold the Base64-encoded representation of
    ///   the data (without terminating null).
    ///
    /// \return 
    ///   True on success. False otherwise.
    ///
    /// \see wc_base64_encode
    /// \see wc_base64_calc_encoded_length
    /// \see wc_get_max_base64_unencoded_length
    //------------------------------------------------------------------------------
    static inline bool wc_base64_encode_for_url(uint8_t const *bytes,
                                                size_t         bytes_length,
                                                char          *encoded,
                                                size_t        *encoded_length)
    {
        return wc_base64_encode_selected(bytes, bytes_length, true, encoded, encoded_length);
    }

    /// Decode the given Base64-encoded string into the provided buffer. 
    /// The buffer must be large enough to hold encoded data.
    ///
    /// \note At most `3*(n/4)` bytes are to be decoded from a base64 string
    ///       with a length of n.
    ///       See: \link wc_base64_calc_decoded_length \endlink
    ///       See: \link wc_get_max_base64_encoded_length \endlink
    /// 
    /// \note The given string lenth must not include the null terminator. Set
    ///       the string length to the length of the string without.
    ///
    /// \param string
    ///   The Base64 string to be decoded.
    ///   Also Base64URL encoded data is supported.
    ///   Mixed contend may be tolerated.
    /// \param string_length 
    ///   The length of the Base64 string to be decoded (without terminating 
    ///   null).
    /// \param decoded
    ///   The buffer to hold the decoded bytes.
    /// \param decoded_length
    ///   The length of the buffer to hold the decoded bytes.
    ///
    /// \return 
    ///   True on success. False otherwise.
    ///
    /// \see wc_base64_calc_decoded_length
    /// \see wc_get_max_base64_encoded_length
    //------------------------------------------------------------------------------
    static inline bool wc_base64_decode(char const *string,
                                        size_t      string_length,
                                        uint8_t    *decoded,
                                        size_t     *decoded_length);

    /// Function to calculate the Base64-encoded length of an unencoded string.
    ///
    /// \note
    ///   The calculation does not honor a terminating null character.
    ///
    /// \param unencoded_length
    ///   The length of an unencoded string to be encoded as Base64.
    ///
    /// \return 
    ///   The needed length as Base64-encoded string
    ///   or 0 if result does not fit into size_t.
    //------------------------------------------------------------------------------
    static inline size_t wc_base64_calc_encoded_length(size_t unencoded_length);

    /// Function to calculate the decoded length of an Base64-encoded string.
    ///
    /// \note
    ///   The calculation does not honor a terminating null character.
    ///
    /// \note
    ///   For simlicity this function may only return values
    ///   which are a multiple of 3.
    ///
    /// \param encoded_length
    ///   The length of an Base64-encoded string to be decoded.
    ///
    /// \return 
    ///   The needed length as unencoded string.
    //------------------------------------------------------------------------------
    static inline size_t wc_base64_calc_decoded_length(size_t encoded_length);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------
static inline bool wc_base64_encode_selected(uint8_t const *bytes,
                                             size_t         bytes_length, 
                                             bool           url_safe,
                                             char          *encoded,
                                             size_t        *encoded_length)
{
    WC_ASSERT_RETURN(bytes          != NULL || bytes_length == 0u, false);
    WC_ASSERT_RETURN(encoded        != NULL,                       false);
    WC_ASSERT_RETURN(encoded_length != NULL,                       false);
    *encoded_length = 0;

    char const  wc_base64_charset_default[]  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char const  wc_base64_charset_url_safe[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    char const *wc_base64_charset            = url_safe ? wc_base64_charset_url_safe : wc_base64_charset_default;
    int i = 0;
    int j = 0;
    uint8_t byte_array_3[3];
    uint8_t byte_array_4[4];

    while(bytes_length > 0u)
    {
        bytes_length--;
        byte_array_3[i++] = *(bytes++);
        if(i == 3)
        {
            byte_array_4[0] = (byte_array_3[0] & 0xfcu) >> 2u;
            byte_array_4[1] = ((byte_array_3[0] & 0x03u) << 4u) + ((byte_array_3[1] & 0xf0u) >> 4u);
            byte_array_4[2] = ((byte_array_3[1] & 0x0fu) << 2u) + ((byte_array_3[2] & 0xc0u) >> 6u);
            byte_array_4[3] = byte_array_3[2] & 0x3fu;

            for(i = 0; i < 4; i++)
            {
                encoded[(*encoded_length)++] = wc_base64_charset[byte_array_4[i]];
            }
            i = 0;
        }
    }

    if(i)
    {
        for(j = i; j < 3; j++)
        {
            byte_array_3[j] = (uint8_t)'\0';
        }

        byte_array_4[0] = (byte_array_3[0] & 0xfcu) >> 2u;
        byte_array_4[1] = ((byte_array_3[0] & 0x03u) << 4u) + ((byte_array_3[1] & 0xf0u) >> 4u);
        byte_array_4[2] = ((byte_array_3[1] & 0x0fu) << 2u) + ((byte_array_3[2] & 0xc0u) >> 6u);
        byte_array_4[3] = byte_array_3[2] & 0x3fu;

        for(j = 0; (j < i + 1); j++)
        {
            encoded[(*encoded_length)++] = wc_base64_charset[byte_array_4[j]];
        }

        if(!url_safe)
        {
            while(i++ < 3)
            {
                encoded[(*encoded_length)++] = '=';
            }
        }
    }
    return true;
}

static inline bool wc_base64_decode(char   const *string,
                                    size_t        string_length,
                                    uint8_t      *decoded,
                                    size_t       *decoded_length)
{
    WC_ASSERT_RETURN(string         != NULL, false);
    WC_ASSERT_RETURN(decoded        != NULL, false);
    WC_ASSERT_RETURN(decoded_length != NULL, false);
    *decoded_length = 0;

    char const  wc_base64_charset_default[]  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char const  wc_base64_charset_url_safe[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    char const *wc_base64_charset            = wc_base64_charset_default;
    int i = 0;
    int j = 0;
    int in = 0;
    uint8_t byte_array_4[4], byte_array_3[3];

    while(( string_length > 0u ) && ( string[in] != '='))
    {
        string_length--;
        char c = string[in];
        bool is_base64_character = false;
        switch(c) // We are not strictly enforcing a specific encoding
        {
            case '+':
            case '/':
                is_base64_character = true;
                wc_base64_charset   = wc_base64_charset_default;
                break;

            case '-':
            case '_':
                is_base64_character = true;
                wc_base64_charset   = wc_base64_charset_url_safe;
                break;

            default:
                is_base64_character = isalnum((unsigned char)c);
        }
        if(!is_base64_character)
        {
            return false; // Non-base64 character found
        }
        byte_array_4[i++] = (uint8_t)c;
        in++;
        if(i == 4)
        {
            for(i = 0; i < 4; i++)
            {
                char const * result = strchr(wc_base64_charset, (signed char)byte_array_4[i]);
                WC_ASSERT_RETURN(result != NULL, false);
                byte_array_4[i] = (uint8_t)(result - wc_base64_charset);
            }
            byte_array_3[0] = (byte_array_4[0] << 2u) + ((byte_array_4[1] & 0x30u) >> 4u);
            byte_array_3[1] = ((byte_array_4[1] & 0xfu) << 4u) + ((byte_array_4[2] & 0x3cu) >> 2u);
            byte_array_3[2] = ((byte_array_4[2] & 0x3u) << 6u) + byte_array_4[3];

            for(i = 0; i < 3; i++)
            {
                decoded[(*decoded_length)++] = byte_array_3[i];
            }
            i = 0;
        }
    }

    if(i)
    {
        for(j = i; j < 4; j++)
        {
            byte_array_4[j] = 0;
        }
        for(j = 0; j < 4; j++)
        {
            char const * result = strchr(wc_base64_charset, (signed char)byte_array_4[j]);
            WC_ASSERT_RETURN(result != NULL, false);
            byte_array_4[j] = (uint8_t)(result - wc_base64_charset);
        }

        byte_array_3[0] = (byte_array_4[0] << 2u) + ((byte_array_4[1] & 0x30u) >> 4u);
        byte_array_3[1] = ((byte_array_4[1] & 0xfu) << 4u) + ((byte_array_4[2] & 0x3cu) >> 2u);
        byte_array_3[2] = ((byte_array_4[2] & 0x3u) << 6u) + byte_array_4[3];

        for(j = 0; (j < i - 1); j++)
        {
            decoded[(*decoded_length)++] = byte_array_3[j];
        }
    }
    return true;
}

static inline size_t wc_base64_calc_encoded_length(size_t unencoded_length)
{
    if(unencoded_length > WC_BASE64_UNENCODED_MAX) return 0;

    // Encoded length is always a multiple of 4
    // Use integer arithmetics for round up (add divider - 1 before division)
    size_t const rounded_to_next_of_four = (unencoded_length + 2u) / 3u * 4u;

    return rounded_to_next_of_four;
}

static inline size_t wc_base64_calc_decoded_length(size_t encoded_length)
{
    // Use integer arithmetics for round up (add divider - 1 before division)
    return (encoded_length + 3u) / 4u * 3u;
}

#ifndef __cplusplus

    /// Function (C variant) to determine maximum length
    /// of unencoded strings to be encoded as Base64.
    ///
    /// \return
    ///   Maximum length of unencoded strings in bytes.
    //------------------------------------------------------------------------------
    static size_t wc_get_max_base64_unencoded_length()
    {
        return WC_BASE64_UNENCODED_MAX;
    }

    /// Function (C variant) to determine maximum length
    /// of Base64 encoded strings.
    ///
    /// \return
    ///   Maximum length of encoded strings in bytes.
    //------------------------------------------------------------------------------
    static size_t wc_get_max_base64_encoded_length()
    {
        return WC_BASE64_ENCODED_MAX;
    }

#else

    /// Function (C++ variant: constexpr) to determine maximum length
    /// of unencoded strings to be encoded as Base64.
    ///
    /// \return
    ///   Maximum length of unencoded strings in bytes.
    //------------------------------------------------------------------------------
    static inline constexpr size_t wc_get_max_base64_unencoded_length()
    {
        return WC_BASE64_UNENCODED_MAX;
    }

    /// Function (C++ variant: constexpr) to determine maximum length
    /// of Base64 encoded strings.
    ///
    /// \return
    ///   Maximum length of encoded strings in bytes.
    //------------------------------------------------------------------------------
    static inline constexpr size_t wc_get_max_base64_encoded_length()
    {
        return WC_BASE64_ENCODED_MAX;
    }

    /// Encode (C++ variant) the given data to a Base64 string.
    ///
    /// \param data
    ///   The data vector to be encoded.
    /// \param url_safe
    ///   Use Base64URL encoding with URL-safe charset and without padding.
    ///
    /// \throw std::runtime_error
    ///   Runtime exceptions are thrown if data can not be encoded
    ///   (e.g. data size too large).
    ///
    /// \return 
    ///   Base64-encoded string representation of the data.
    ///
    /// \see wc_base64_encode_selected
    //------------------------------------------------------------------------------
    static inline std::string wc_base64_encode(std::vector<uint8_t> const &data,
                                               bool                        url_safe = false)
    {
        size_t encoded_length = wc_base64_calc_encoded_length(data.size());
        if(encoded_length == 0)
        {
            throw std::runtime_error("Data too large for Base64 encoding (exceeding range of size_t)");
        }

        std::string result(encoded_length, '\0');
        size_t actual_length = encoded_length;

        if(!wc_base64_encode_selected(data.data(), data.size(), url_safe, &result[0], &actual_length))
        {
            throw std::runtime_error("Failed to encode data to base64");
        }

        result.resize(actual_length);
        return result;
    }

    /// Decode (C++ variant) the given Base64 string to a raw data vector.
    ///
    /// \param encoded_base64
    ///   Base64 string to be decoded.
    ///   Also Base64URL encoded data is supported.
    ///   Mixed contend may be tolerated.
    ///
    /// \throw std::invalid_argument
    ///   Invalid argument exceptions are thrown if data can not be decoded
    ///   (e.g. string is not Base64 encoded).
    ///
    /// \return 
    ///   Base64-encoded string representation of the data.
    //------------------------------------------------------------------------------
    static inline std::vector<uint8_t> wc_base64_decode(std::string const &encoded_base64)
    {
        size_t decoded_length = wc_base64_calc_decoded_length(encoded_base64.size());
        WC_ASSERT(decoded_length != 0);

        std::vector<uint8_t> result(decoded_length);
        size_t actual_length = decoded_length;

        if(!wc_base64_decode(encoded_base64.c_str(), encoded_base64.size(), result.data(), &actual_length))
        {
            throw std::invalid_argument("Failed to decode Base64 data");
        }

        result.resize(actual_length);
        return result;
    }

#endif // __cplusplus


#endif // INC_WC_BASE64_H_
//---- End of source file ------------------------------------------------------
