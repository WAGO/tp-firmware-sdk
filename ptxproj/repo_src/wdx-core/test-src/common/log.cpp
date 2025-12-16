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
#include <gtest/gtest.h>
#include <wc/log.h>

#include "common/log.hpp"

std::string to_string(log_level_t const log_level) {
    switch(log_level) {
        case log_level::fatal: return "FATAL";
        case log_level::error: return "ERROR";
        case log_level::warning: return "WARN";
        case log_level::notice: return "NOTE";
        case log_level::info: return "INFO";
        case log_level::debug: return "DEBUG";
        default: return "";
    }
}

log_mode current_log_mode = lax;

void wc_log_output(log_level_t  const log_level,
                   char const * const message) noexcept
{
    if(log_level == log_level_t::debug) {return;}
    printf("%-5s: %s\n", to_string(log_level).c_str(), message);
    if(current_log_mode == strict)
    {
        ASSERT_NE(log_level, log_level_t::error) << "No error should happen during test.";
        ASSERT_NE(log_level, log_level_t::warning) << "No warning should happen during test.";
    }
};

log_level_t wc_get_log_level() noexcept
{
    return log_level_t::debug;
}
