//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2022-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    File parameter handler for file parameters.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_FILE_FILE_PARAMETER_HANDLER_HPP_
#define INC_WAGO_WDX_LINUXOS_FILE_FILE_PARAMETER_HANDLER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/file/file_provider_extended_i.hpp"

#include <wago/future.hpp>
#include <wago/wdx/parameter_value.hpp>
#include <wago/wdx/wdmm/base_types.hpp>
#include <wc/structuring.h>

#include <memory>
#include <mutex>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {

namespace wdx {
class parameter_service_backend_i;
}

namespace wdx {
namespace linuxos {
namespace file {

/// Factory function to create a custom file provider type used in \ref file_parameter_handler
using file_provider_factory = std::unique_ptr<file_provider_extended_i>(bool readonly);

/// File ID value as shared pointer of parameter value
using file_id_value = std::shared_ptr<wdx::parameter_value>;


//------------------------------------------------------------------------------
/// \brief File parameter handler class to handle parameters of type File-ID.
///
/// \remark This class is thread-safe.
///
/// This handler automatically creates, registers, unregisters and destroys file providers
/// used to implement the file parameter (\see file_provider_factory).
//------------------------------------------------------------------------------
class file_parameter_handler final
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(file_parameter_handler)

private:
    using read_file_id_notifier = std::function<void(file_id_value)>;

    struct state
    {
        wdx::parameter_id_t                       const  file_parameter_id_m;
        wdx::parameter_service_backend_i                &backend_m;
        std::function<file_provider_factory>             provider_factory_m;

        file_id_value                                    read_file_id_m;
        std::unique_ptr<file_provider_extended_i>        read_provider_m;
        std::mutex                                       read_provider_mutex_m;
        std::vector<read_file_id_notifier>               read_file_id_notifier_m;
        file_id_value                                    write_file_id_m;
        std::unique_ptr<file_provider_extended_i>        write_provider_m;
        std::mutex                                       write_provider_mutex_m;

        state(wdx::parameter_id_t                  const  file_parameter_id,
              wdx::parameter_service_backend_i           &backend,
              std::function<file_provider_factory>        provider_factory);
    };
    std::shared_ptr<state> state_m;

public:
    //------------------------------------------------------------------------------
    /// Constructs a file parameter handler for automatic handling of File-ID parameters.
    ///
    /// \remark
    ///   Does not catch any exception thrown by backend
    ///   (e.g. wdx::linuxos::com::exception if used remote via IPC).
    ///
    /// \param file_parameter_id
    ///   File parameter ID this handler is responsible for.
    /// \param backend
    ///   Service backend to be used for register and unregister calls of file providers.
    /// \param provider_factory
    ///   Factory function to create custom file provider type.
    //------------------------------------------------------------------------------
    WDXLINUXOSFILE_API file_parameter_handler(wdx::parameter_id_t                  const  file_parameter_id,
                                       wdx::parameter_service_backend_i           &backend,
                                       std::function<file_provider_factory>        provider_factory);

    //------------------------------------------------------------------------------
    /// Destructs file parameter handler and removes any file provider for associated file parameter.
    //------------------------------------------------------------------------------
    WDXLINUXOSFILE_API ~file_parameter_handler() noexcept;

    //------------------------------------------------------------------------------
    /// Get the file ID for read purposes.
    ///
    /// \return
    ///   Future of shared pointer to parameter value with file ID for read purposes.
    //------------------------------------------------------------------------------
    WDXLINUXOSFILE_API future<file_id_value> get_file_id();

    //------------------------------------------------------------------------------
    /// Create file ID for write purposes.
    ///
    /// \remark
    ///   Does not catch any exception thrown by backend
    ///   (e.g. wdx::linuxos::com::exception if used remote via IPC).
    ///
    /// \throws
    /// Throws wdx::linuxos::file::exception if an error occurs.
    ///
    /// \param context
    ///   Parameter ID for context information.
    ///
    /// \return
    ///   Future of shared pointer to parameter value with file ID for write purposes.
    //------------------------------------------------------------------------------
    WDXLINUXOSFILE_API future<file_id_value> create_file_id_for_write(wdx::parameter_id_t context);

    //------------------------------------------------------------------------------
    /// Removes the current file ID for write purposes.
    ///
    /// \remark
    ///   Does not catch any exception thrown by backend
    ///   (e.g. wdx::linuxos::com::exception if used remote via IPC).
    ///
    /// \throws
    /// Throws wdx::linuxos::file::exception if an error occurs.
    ///
    /// \param context
    ///   File ID expected to be removed.
    //------------------------------------------------------------------------------
    WDXLINUXOSFILE_API void                  remove_file_id_for_write(wdx::file_id id);

    //------------------------------------------------------------------------------
    /// Set file ID value active for parameter.
    ///
    /// \remark
    /// This operation also sets this file ID to readonly.
    /// No further write requests will be processed successfully.
    ///
    /// \remark
    ///   Does not catch any exception thrown by backend
    ///   (e.g. wdx::linuxos::com::exception if used remote via IPC).
    ///
    /// \throws
    /// Throws wdx::linuxos::file::exception if an error occurs.
    /// For example this is also the case if the file
    /// - was written incomplete or
    /// - has not been validated successfully or
    /// - have not been written successfully to the final destination.
    ///
    /// \param value
    ///   Shared pointer to parameter value with file ID to write.
    /// \param validator
    ///   Function to validate file content.
    ///
    /// \return
    ///   Future to track the end of this operation.
    //------------------------------------------------------------------------------
    WDXLINUXOSFILE_API future<void>          set_file_id(file_id_value  value,
                                                  file_validator validator);
};


} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_FILE_FILE_PARAMETER_HANDLER_HPP_

//---- End of source file ------------------------------------------------------
