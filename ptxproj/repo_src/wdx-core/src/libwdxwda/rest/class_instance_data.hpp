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
///  \brief    Container for class instance data from the core API.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_CLASS_INSTANCE_DATA_HPP_
#define SRC_LIBWDXWDA_REST_CLASS_INSTANCE_DATA_HPP_

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

class class_instance_data {
private:
    wdx::parameter_instance_path               const class_param_id_m = {};
    wdx::instance_id_t                         const instance_no_m    = 0;

public:
    /// Construct a new class instance data object which represents a dangling 
    /// reference.
    class_instance_data() = default;
    explicit class_instance_data(wdx::parameter_instance_path const &instance_param_id);
    class_instance_data(wdx::parameter_instance_path const &class_param_id, 
                        wdx::instance_id_t                  instance_no);

    wdx::parameter_instance_path get_class_parameter() const;
    std::string                  get_path() const;
    wdx::instance_id_t           get_instance_no() const;
    bool                         is_dangling() const;
};

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_CLASS_INSTANCE_DATA_HPP_
//---- End of source file ------------------------------------------------------
