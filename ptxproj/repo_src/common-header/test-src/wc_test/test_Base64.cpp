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
///  \brief    Test for trace functions.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wc/base64.h"
#include "wc/std_includes.h"
#include "c-modules/test_Base64Mod.h"

#include <gtest/gtest.h>
#include <tuple>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------

struct test_data {
    std::vector<uint8_t> bytes;
    std::string          base64;
    bool                 url_safe;
};
using Base64Test = ::testing::TestWithParam<test_data>;

TEST_P(Base64Test, Encode) {
    auto data = GetParam();

    char * result_buffer = new char[data.base64.size()];
    result_buffer[data.base64.size()] = '\0';
    size_t result_length = 0;

    {
        auto result = wc_base64_encode_selected(data.bytes.data(), data.bytes.size(), 
                                                data.url_safe,
                                                result_buffer, &result_length);

        EXPECT_TRUE(result);
        EXPECT_EQ(result_length, data.base64.size());
        EXPECT_STREQ(result_buffer, data.base64.c_str());
    }

    delete [] result_buffer;
}

TEST_P(Base64Test, Decode) {
    auto data = GetParam();

    uint8_t * result_buffer = new uint8_t[data.bytes.size()];
    size_t result_length = 0;

    {
        auto result = wc_base64_decode(data.base64.data(), data.base64.size(), 
                                       result_buffer, &result_length);

        EXPECT_TRUE(result);
        EXPECT_EQ(result_length, data.bytes.size());
        EXPECT_EQ(std::vector<uint8_t>(result_buffer, result_buffer + result_length), data.bytes);
    }

    delete [] result_buffer;
}

constexpr static char const * const base64_alphabet =
    R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/)";

constexpr static char const * const base64_alphabet_url =
    R"(ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_)";

constexpr static uint8_t const base64_alphabet_decoded[] =
    { 0x00, 0x10, 0x83, 0x10, 0x51, 0x87, 0x20, 0x92, 0x8b, 0x30, 0xd3, 0x8f,
      0x41, 0x14, 0x93, 0x51, 0x55, 0x97, 0x61, 0x96, 0x9b, 0x71, 0xd7, 0x9f,
      0x82, 0x18, 0xa3, 0x92, 0x59, 0xa7, 0xa2, 0x9a, 0xab, 0xb2, 0xdb, 0xaf,
      0xc3, 0x1c, 0xb3, 0xd3, 0x5d, 0xb7, 0xe3, 0x9e, 0xbb, 0xf3, 0xdf, 0xbf };

constexpr static size_t const base64_alphabet_decoded_size = sizeof(base64_alphabet_decoded);

INSTANTIATE_TEST_CASE_P(
    Base64Tests,
    Base64Test,
    ::testing::Values(
        test_data { std::vector<uint8_t>(0), "", false },
        test_data { { 'A' }, "QQ==", false },
        test_data { { 'A', 'B', 'C' }, "QUJD", false },
        test_data { { 'L', 'o', 'r', 'e', 'm', ' ', 'i', 'p', 's', 'u', 'm' }, "TG9yZW0gaXBzdW0=", false },
        test_data { { 0x6b, 0xef, 0xfd }, "a+/9", false },
        test_data { {base64_alphabet_decoded, base64_alphabet_decoded + base64_alphabet_decoded_size}, base64_alphabet, false },
        test_data { std::vector<uint8_t>(0), "", true },
        test_data { { 'A' }, "QQ", true },
        test_data { { 'A', 'B', 'C' }, "QUJD", true },
        test_data { { 'L', 'o', 'r', 'e', 'm', ' ', 'i', 'p', 's', 'u', 'm' }, "TG9yZW0gaXBzdW0", true },
        test_data { { 0x6b, 0xef, 0xfd }, "a-_9", true },
        test_data { {base64_alphabet_decoded, base64_alphabet_decoded + base64_alphabet_decoded_size}, base64_alphabet_url, true }
    )
);

TEST(Base64Test, CharCalculationForEncoded)
{
    // Check small numbers around rounding of 4
    EXPECT_EQ(          0 + 0, wc_base64_calc_encoded_length(            0));
    EXPECT_EQ(          1 + 3, wc_base64_calc_encoded_length(            1));
    EXPECT_EQ(          1 + 3, wc_base64_calc_encoded_length(            2));
    EXPECT_EQ(          4 + 0, wc_base64_calc_encoded_length(            3));
    EXPECT_EQ(          5 + 3, wc_base64_calc_encoded_length(            4));
    EXPECT_EQ(          6 + 2, wc_base64_calc_encoded_length(            5));

    // Check a number in the middle of common range
    EXPECT_EQ(       1646 + 2, wc_base64_calc_encoded_length(         1235));

    // Check border case around singed size (32 bit)
    size_t const enc_32bit_ssize = 2863311529UL + 3;
    EXPECT_EQ(enc_32bit_ssize, wc_base64_calc_encoded_length(      INT_MAX)); // SSIZE_MAX 32 bit

    // Check border case around signed size (64 bit) only if SSIZE_MAX 64 bit is available
# if __WORDSIZE == 64 || __WORDSIZE32_SIZE_ULONG
    WC_STATIC_ASSERT(SSIZE_MAX  > UINT32_MAX);
    WC_STATIC_ASSERT(SSIZE_MAX  >    INT_MAX);
    WC_STATIC_ASSERT(SSIZE_MAX ==   LONG_MAX);

    size_t const enc_64bit_ssize = 12297829382473034409ULL + 3;
    EXPECT_EQ(enc_64bit_ssize, wc_base64_calc_encoded_length(    SSIZE_MAX)); // SSIZE_MAX 64 bit
# endif

    // Check maximum numbers
    size_t const max_unencoded = wc_get_max_base64_unencoded_length();
    size_t const max_encoded   = wc_get_max_base64_encoded_length();
    EXPECT_EQ(   max_encoded, wc_base64_calc_encoded_length(max_unencoded));
    EXPECT_EQ(             0, wc_base64_calc_encoded_length(   SIZE_MAX  ));
}

TEST(Base64Test, CharCalculationForDecoded)
{
    // Check small numbers around rounding of 3
    EXPECT_EQ(            0 + 0, wc_base64_calc_decoded_length(            0));
    EXPECT_EQ(            1 + 2, wc_base64_calc_decoded_length(            1));
    EXPECT_EQ(            2 + 1, wc_base64_calc_decoded_length(            2));
    EXPECT_EQ(            3 + 0, wc_base64_calc_decoded_length(            3));
    EXPECT_EQ(            3 + 0, wc_base64_calc_decoded_length(            4));
    EXPECT_EQ(            4 + 2, wc_base64_calc_decoded_length(            5));

    // Check a number in the middle of common range
    EXPECT_EQ(         1235 + 1, wc_base64_calc_decoded_length(         1646));

    // Check maximum numbers
    size_t const max_unencoded = wc_get_max_base64_unencoded_length();
    size_t const max_encoded   = wc_get_max_base64_encoded_length();
    EXPECT_EQ(   max_unencoded,  wc_base64_calc_decoded_length( max_encoded ));
}

TEST(Base64Test, EncodingDecodingCFunctions)
{
    uint8_t const data_bytes[] = "Data to encode";
    size_t  const data_length  = sizeof(data_bytes) - 1;

    uint8_t data_decoded[sizeof(data_bytes)] = "";
    size_t  decoded_length                   = 0;

    EXPECT_TRUE(test_base64_endecode_c(data_bytes, data_length, data_decoded, &decoded_length));
    EXPECT_EQ(data_length, decoded_length);
    data_decoded[data_length] = '\0';
    EXPECT_STREQ(reinterpret_cast<char const *>(data_bytes), reinterpret_cast<char const *>(data_decoded));
}

TEST(Base64Test, EncodingDecodingForURLCFunctions)
{
    uint8_t const data_bytes[] = "Data to encode";
    size_t  const data_length  = sizeof(data_bytes) - 1;

    uint8_t data_decoded[sizeof(data_bytes)] = "";
    size_t  decoded_length                   = 0;

    EXPECT_TRUE(test_base64_endecode_url_c(data_bytes, data_length, data_decoded, &decoded_length));
    EXPECT_EQ(data_length, decoded_length);
    data_decoded[data_length] = '\0';
    EXPECT_STREQ(reinterpret_cast<char const *>(data_bytes), reinterpret_cast<char const *>(data_decoded));
}

TEST(Base64Test, EncodingDecodingCPPFunctions)
{
    uint8_t const data_bytes[] = "Data to encode";
    size_t  const data_length  = sizeof(data_bytes) - 1;

    std::string          const base64_result = wc_base64_encode({data_bytes, data_bytes + data_length});
    std::vector<uint8_t> const raw_result    = wc_base64_decode(base64_result);

    EXPECT_EQ(data_length, raw_result.size());
    EXPECT_STREQ(reinterpret_cast<char const *>(data_bytes), reinterpret_cast<char const *>(raw_result.data()));
}

TEST(Base64Test, EncodingDecodingForURLCPPFunctions)
{
    uint8_t const data_bytes[] = "Data to encode";
    size_t  const data_length  = sizeof(data_bytes) - 1;

    std::string          const base64_result = wc_base64_encode({data_bytes, data_bytes + data_length}, true);
    std::vector<uint8_t> const raw_result    = wc_base64_decode(base64_result);

    EXPECT_EQ(data_length, raw_result.size());
    EXPECT_STREQ(reinterpret_cast<char const *>(data_bytes), reinterpret_cast<char const *>(raw_result.data()));
}

TEST(Base64Test, DecodingInvalidCPPFunctions)
{
    std::string const invalid_base64 = "i n v a l i d";
    EXPECT_THROW(auto const raw_result = wc_base64_decode(invalid_base64), std::invalid_argument);
}


//---- End of source file ------------------------------------------------------
