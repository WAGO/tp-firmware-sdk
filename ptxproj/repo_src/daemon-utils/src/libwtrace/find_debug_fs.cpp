//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2021 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Kernel Implementation of helper function to find Debug FS.
///
///  \author   MaHe:  WAGO GmbH & Co. KG
///  \author   PEn:   WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "find_debug_fs.hpp"

#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wtrace {

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
std::string find_debug_fs()
{
    std::string result = "";
    // Open mount points file descriptor for search
    char const mount_fs[] = "/proc/mounts";
    FILE * mounts = fopen(mount_fs, "r");
    if(mounts == NULL)
    {
        throw std::runtime_error(std::string("Kernel Tracer: Unable to read mounts via ") + mount_fs);
    }

    // Search debug file system type in mount points
    char type[32];
    char filesystem_path[256];
    while(fscanf(mounts, "%*s %255s %31s %*s %*d %*d\n", filesystem_path, type) == 2)
    {
        // Check type
        if(strcmp(type, "debugfs") == 0)
        {
            result = filesystem_path;
            break;
        }
    }

    // Close mount points file descriptor
    fclose(mounts);

    if(result.empty())
    {
        throw std::runtime_error(std::string("Kernel Tracer: Unable to find debug file system, not listed in ") + mount_fs);
    }

    return result;
}

} // Namespace wtrace
} // Namespace wago


//---- End of source file ------------------------------------------------------
