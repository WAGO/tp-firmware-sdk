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
///  \brief    System abstraction implementation for file management.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction_file.hpp"
#include "errno_utils.hpp"

#include <wc/assertion.h>
#include <wc/structuring.h>

#include <limits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <regex>

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {
namespace sal {

class filemanagement_impl : public filemanagement
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(filemanagement_impl)

public:
    filemanagement_impl() noexcept = default;
    ~filemanagement_impl() noexcept override = default;

    file_descriptor open_readonly(std::string const &file_path) noexcept override;
    file_descriptor create_and_open_temp_file(std::string const &file_path_prefix,
                                              std::string const &file_suffix,
                                              std::string       &result_file_path) override;
    void delete_old_temp_files(std::string const &file_path_prefix,
                               std::string const &file_suffix) override;
    int fchmod(file_descriptor fd,
               mode_t          mode) noexcept override;
    int fchown(file_descriptor fd,
               uid_t           owner,
               gid_t           group) noexcept override;
    off64_t seek_to_offset(file_descriptor fd,
                           size_t          offset) noexcept override;
    ssize_t read(file_descriptor       fd,
                 std::vector<uint8_t> &buffer) noexcept override;
    ssize_t write(file_descriptor             fd,
                  std::vector<uint8_t> const &data) noexcept override;
    int close(file_descriptor fd) noexcept override;
    int fsync(file_descriptor fd) noexcept override;
    int get_file_size(std::string const &file_path,
                      uint64_t          &file_size) noexcept override;
    int get_file_permissions(std::string const &file_path,
                             mode_t            &mode,
                             uid_t             &uid,
                             gid_t             &gid) noexcept override;
    int get_file_system_space(std::string const &directory_path,
                              uint64_t          &free_size) noexcept override;
    int rename(std::string const &old_path,
               std::string const &new_path) noexcept override;
    int unlink(std::string const &file_path) noexcept override;
    void get_user_info(std::string const &name,
                       uid_t             &uid,
                       gid_t             &gid) override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static filemanagement_impl default_filemanagement;

filemanagement *filemanagement::instance = &default_filemanagement;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
file_descriptor filemanagement_impl::open_readonly(std::string const &file_path) noexcept
{
    return ::open(file_path.c_str(), O_RDONLY);
}

file_descriptor filemanagement_impl::create_and_open_temp_file(std::string const &file_path_prefix,
                                                               std::string const &file_suffix,
                                                               std::string       &result_file_path)
{
    std::string template_file_name = "-XXXXXX";
    std::string full_file_path_template = file_path_prefix + template_file_name + file_suffix;
    // mkstemps may modify its input argument, therefore a modifiable c-string is needed
    char * template_path = ::strdup(full_file_path_template.c_str());
    if(template_path != nullptr)
    {
        auto return_value = ::mkstemps(template_path, static_cast<int>(file_suffix.length()));
        result_file_path = template_path;
        ::free(template_path); // NOLINT(cppcoreguidelines-owning-memory, cppcoreguidelines-no-malloc)
        return return_value;
    }
    return -1;
}

void filemanagement_impl::delete_old_temp_files(std::string const &file_path_prefix,
                                                std::string const &file_suffix)
{
    std::string file_path = file_path_prefix.substr(0, file_path_prefix.find_last_of('/'));
    std::string file_name = file_path_prefix.substr(file_path_prefix.find_last_of('/') + 1, file_path_prefix.size());
    DIR *directory;
    dirent dir_content;
    dirent *dir_content_result = nullptr;
    if((directory = ::opendir(file_path.c_str())) != nullptr) // parasoft-suppress CERT_C-EXP45-d "Compliant solution. Won't fix."
    {
        while(   ((::readdir_r(directory, &dir_content, &dir_content_result)) == 0) 
              && (dir_content_result != nullptr)) // parasoft-suppress CERT_C-EXP45-d "Compliant solution. Won't fix."
        {
            if(std::regex_match(dir_content.d_name, std::regex(file_name + "-.{6}" + file_suffix)))
            {
                ::unlink((file_path + "/"+ dir_content.d_name).c_str());
            }
        }
        ::closedir(directory);
    }
}

int filemanagement_impl::fchmod(file_descriptor fd,
                                mode_t          mode) noexcept
{
    return ::fchmod(fd, mode);
}

int filemanagement_impl::fchown(file_descriptor fd,
                                uid_t           owner,
                                gid_t           group) noexcept
{
    return ::fchown(fd, owner, group);
}

off64_t filemanagement_impl::seek_to_offset(file_descriptor fd,
                                            size_t          offset) noexcept
{
    return ::lseek64(fd, static_cast<off64_t>(offset), SEEK_SET);
}

ssize_t filemanagement_impl::read(file_descriptor       fd,
                                  std::vector<uint8_t> &buffer) noexcept
{
    return ::read(fd, buffer.data(), buffer.size());
}

ssize_t filemanagement_impl::write(file_descriptor             fd,
                                   std::vector<uint8_t> const &data) noexcept
{
    WC_ASSERT(data.size() <= SSIZE_MAX);

    ssize_t data_pos     = 0;
    ssize_t written_data = 0;
    WC_STATIC_ASSERT(SSIZE_MAX <= SIZE_MAX);
    while((written_data = ::write(fd, data.data() + data_pos, data.size() - static_cast<size_t>(data_pos))) >= 0) // parasoft-suppress CERT_C-EXP45-d "Compliant solution. Won't fix."
    {
        data_pos += written_data;
        if(data_pos >= static_cast<ssize_t>(data.size()))
        {
            break;
        }
    }
    if (written_data < 0)
    {
        return written_data;
    }
    return data_pos;
}

int filemanagement_impl::close(file_descriptor fd) noexcept
{
    return ::close(fd);
}

int filemanagement_impl::fsync(file_descriptor fd) noexcept
{
    return ::fsync(fd);
}

int filemanagement_impl::get_file_size(std::string const &file_path,
                                       uint64_t          &file_size) noexcept
{
    struct stat64 stat_buffer;
    auto return_value = ::stat64(file_path.c_str(), &stat_buffer);
    if(return_value == 0)
    {
        file_size = static_cast<uint64_t>(stat_buffer.st_size);
    }
    return return_value;
}

int filemanagement_impl::get_file_permissions(std::string const &file_path,
                                              mode_t            &mode,
                                              uid_t             &uid,
                                              gid_t             &gid) noexcept
{
    struct stat64 stat_buffer;
    auto return_value = ::stat64(file_path.c_str(), &stat_buffer);
    if(return_value == 0)
    {
        mode = stat_buffer.st_mode;
        uid  = stat_buffer.st_uid;
        gid  = stat_buffer.st_gid;
    }
    return return_value;
}

int filemanagement_impl::get_file_system_space(std::string const &directory_path,
                                               uint64_t          &free_size) noexcept
{
    struct statvfs64 stat_buffer;
    auto return_value = ::statvfs64(directory_path.c_str(), &stat_buffer);
    if(return_value == 0)
    {
        free_size = stat_buffer.f_bsize * stat_buffer.f_bfree;
    }
    return return_value;
}

int filemanagement_impl::rename(std::string const &old_path,
                                std::string const &new_path) noexcept
{
    return ::rename(old_path.c_str(), new_path.c_str());
}

int filemanagement_impl::unlink(std::string const &file_path) noexcept
{
    return ::unlink(file_path.c_str());
}

void filemanagement_impl::get_user_info(std::string const &name,
                                        uid_t             &uid,
                                        gid_t             &gid)
{
    // 16k as also used in linux manpage example
    constexpr size_t   pw_buffer_size = 16384u;
    char               pw_buffer[pw_buffer_size];
    passwd             pw_data;
    passwd           * pw_result = nullptr;
    auto               pw_status = getpwnam_r(name.c_str(), &pw_data, pw_buffer, pw_buffer_size, &pw_result);

    if(pw_result == nullptr)
    {
        std::string error_message = std::string("Unable to determine user ID for user \"") + name + "\": ";
        if(pw_status != 0)
        {
            error_message += errno_to_string(pw_status);
        }
        else
        {
            error_message += "No user exists with that name.";
        }
        throw std::runtime_error(error_message);
    }

    uid = pw_data.pw_uid;
    gid = pw_data.pw_gid;
}


} // Namespace sal
} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


//---- End of source file ------------------------------------------------------
