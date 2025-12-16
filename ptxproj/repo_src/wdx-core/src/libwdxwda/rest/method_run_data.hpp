//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Wrapper for method invocation responses from the core API
///            and other method run data.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_METHOD_RUN_DATA_HPP_
#define SRC_LIBWDXWDA_REST_METHOD_RUN_DATA_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/responses.hpp"
#include <wc/std_includes.h>

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

class method_run_data {
private:
    std::string                                            const method_id_m;
    uint32_t                                               const timeout_m;
    std::shared_ptr<wdx::method_invocation_named_response> const response_m;

public:
    method_run_data(std::string                                            const &method_id,
                    uint32_t                                               const  timeout,
                    std::shared_ptr<wdx::method_invocation_named_response> const  response);

    std::string                                            get_associated_method() const;
    uint32_t                                               get_timeout() const;
    std::shared_ptr<wdx::method_invocation_named_response> get_response() const;
};


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_METHOD_RUN_DATA_HPP_
//---- End of source file ------------------------------------------------------
