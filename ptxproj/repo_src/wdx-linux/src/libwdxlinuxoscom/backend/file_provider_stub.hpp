//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    File provider stub for IPC.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_BACKEND_FILE_PROVIDER_STUB_HPP_
#define SRC_LIBWDXLINUXOSCOM_BACKEND_FILE_PROVIDER_STUB_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/abstract_stub.hpp"
#include "backend_methods.hpp"

#include <wago/wdx/file_transfer/file_provider_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class file_provider_stub final : public abstract_stub<wdx::file_provider_i>
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(file_provider_stub)

private:
    wdx::parameter_id_t target_context_m = 0;
    wdx::file_id        file_id_m;

public:
    bool used_serial_mode_on_register = false;
    using abstract_stub::abstract_stub;
    ~file_provider_stub() noexcept override = default;

    void set_context(wdx::parameter_id_t context)
    { 
        target_context_m = context;
    }
    wdx::parameter_id_t get_context() const
    {
        return target_context_m;
    }

    void set_file_id(wdx::file_id const &file_id)
    { 
        file_id_m = file_id;
    }
    wdx::file_id get_file_id() const
    {
        return file_id_m;
    }

private:
    void handle_call(method_id_type method_id,
                     data_istream  &method_args) override
    {
        switch (method_id)
        {
        case file_provider_method_id::read:
            call(&wdx::file_provider_i::read, method_id, method_args);
            break;
        case file_provider_method_id::write:
            call(&wdx::file_provider_i::write, method_id, method_args);
            break;
        case file_provider_method_id::get_file_info:
            call(&wdx::file_provider_i::get_file_info, method_id, method_args);
            break;
        case file_provider_method_id::create:
            call(&wdx::file_provider_i::create, method_id, method_args);
            break;
        default:
            break;
        }
    }
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_BACKEND_FILE_PROVIDER_STUB_HPP_
//---- End of source file ------------------------------------------------------
