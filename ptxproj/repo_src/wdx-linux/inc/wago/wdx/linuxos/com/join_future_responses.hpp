//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Helper for joining individual future responses.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_COM_JOIN_FUTURE_RESPONSES_HPP_
#define INC_WAGO_WDX_LINUXOS_COM_JOIN_FUTURE_RESPONSES_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/status_codes.hpp>
#include <wago/future.hpp>
#include <wc/std_includes.h>
#include <wc/assertion.h>

#include <vector>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <exception>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

/// Produces a future for a vector of individual results coming from individual futures
/// in the first place. The future returned never has an exception but is always resolved
/// with a value instead
///
/// \tparam ResponseType
///   The type of individual responses inherting from \ref wdx::response
///
/// \param futures
///   List of single futures which are not notified, yet
///
/// \return
///   A single future
template<typename ResponseType>
future<std::vector<ResponseType>> join_future_responses(std::vector<future<ResponseType>> &&futures)
{
    auto joined_promise = std::make_shared<promise<std::vector<ResponseType>>>();
    auto values         = std::make_shared<std::vector<ResponseType>>(futures.size());
    auto done_count     = std::make_shared<size_t>(0);
    auto mutex          = std::make_shared<std::mutex>();
    auto increment_check_resolve = [joined_promise, values, done_count]() {
        *done_count += 1;
        if (*done_count == values->size())
        {
            joined_promise->set_value(std::move(*values));
        }
    };
    for (size_t i = 0; i < values->size(); ++i) {
        futures.at(i).set_notifier([i, values, mutex, increment_check_resolve](ResponseType &&value) {
            std::lock_guard<std::mutex> lock(*mutex);
            values->at(i) = std::move(value);
            increment_check_resolve();
        });
        futures.at(i).set_exception_notifier([i, values, mutex, increment_check_resolve](std::exception_ptr e_ptr) {
            WC_FAIL("unexected exception on wda response future");
            std::lock_guard<std::mutex> lock(*mutex);
            try
            {
                if(e_ptr)
                {
                    std::rethrow_exception(e_ptr);
                }
            }
            catch(std::exception const &e)
            {
                values->at(i) = ResponseType(wdx::status_codes::internal_error, e.what());
            }
            catch(...)
            { 
                values->at(i) = ResponseType(wdx::status_codes::internal_error);
            }
            increment_check_resolve();
        });
    }
    return joined_promise->get_future();
}

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_COM_JOIN_FUTURE_RESPONSES_HPP_
//---- End of source file ------------------------------------------------------
