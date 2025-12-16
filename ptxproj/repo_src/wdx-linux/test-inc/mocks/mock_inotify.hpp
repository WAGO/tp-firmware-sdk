//------------------------------------------------------------------------------
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
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
///  \brief    Mock for inotify notification interface (system abstraction).
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_INOTIFY_HPP_
#define TEST_INC_MOCKS_MOCK_INOTIFY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction_com.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>

using wago::wdx::linuxos::com::sal::inotify;
using wago::wdx::linuxos::com::sal::watch_handle;
using wago::wdx::linuxos::com::sal::file_descriptor;

using testing::Exactly;
using testing::Invoke;
using testing::WithArgs;
using testing::AnyNumber;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_inotify : public inotify
{
private:
    using watches = std::map<watch_handle, std::string>;
    using fd_map  = std::map<file_descriptor, file_descriptor>;

    inotify         *saved_instance;
    fd_map           active_fds;
    watch_handle     handle_count;
    watches          active_watches;
    bool             watches_possible;
    bool             watches_enabled;

public:
    static void set_instance(inotify *new_instance)
    {
        instance = new_instance;
    }

    mock_inotify()
    {
        saved_instance = &get_instance();
        handle_count   = 0;
        set_instance(this);
        allow_init();
        enable_watches();
    }
    ~mock_inotify()
    {
        set_instance(saved_instance);
        EXPECT_TRUE(active_fds.empty());
    }

    void allow_init()
    {
        watches_possible = true;
    }

    void disallow_init()
    {
        watches_possible = false;
    }

    void enable_watches()
    {
        watches_enabled = true;
    }

    void disable_watches()
    {
        watches_enabled = false;
    }

    void write_event(std::string const &event_filename,
                     uint32_t    const  event_mask)
    {
        size_t const filename_size = event_filename.empty() ? 0 : event_filename.length() + 1;
        std::vector<uint8_t> event_container;
        event_container.resize(offsetof(inotify_event, name) + filename_size);
        GNUC_DIAGNOSTIC_PUSH
        GNUC_DIAGNOSTIC_IGNORE("-Wcast-align")
        inotify_event *event = reinterpret_cast<inotify_event *>(event_container.data());
        GNUC_DIAGNOSTIC_POP
        event->mask = event_mask;
        event->len  = filename_size;
        if(filename_size > 0)
        {
            strcpy(event->name, event_filename.c_str());
        }
        for(auto fd_pair : active_fds)
        {
            write(fd_pair.second, event_container.data(), event_container.size());
        }
    }

    MOCK_METHOD(file_descriptor, init1, (int flags), (noexcept));
    file_descriptor init1_followers(int flags)
    {
        // Prepare pipe file descriptor usable for Boost
        EXPECT_EQ(IN_NONBLOCK, (flags & IN_NONBLOCK));
        file_descriptor  pipe_fds[2]   = { -1, -1 };
        file_descriptor &read_pipe_fd  = pipe_fds[0];
        file_descriptor &write_pipe_fd = pipe_fds[1];
        if(watches_possible)
        {
            EXPECT_EQ(0, pipe2(pipe_fds, O_NONBLOCK));
            EXPECT_LE(0, read_pipe_fd);
            EXPECT_LE(0, write_pipe_fd);
            active_fds.insert(std::pair<file_descriptor, file_descriptor>(read_pipe_fd, write_pipe_fd));
        }
        if(read_pipe_fd != -1)
        {
            EXPECT_CALL(*this, close(::testing::_))
                .Times(Exactly(1))
                .WillRepeatedly(WithArgs<0>(Invoke(this, &mock_inotify::close_followers)))
                .RetiresOnSaturation();
        }
        return read_pipe_fd;
    }

    MOCK_METHOD(int, close, (file_descriptor fd));
    int close_followers(file_descriptor fd)
    {
        EXPECT_TRUE(active_fds.count(fd) > 0);
        ::close(fd);
        ::close(active_fds.at(fd));
        active_fds.erase(fd);
        EXPECT_CALL(*this, add_watch(fd, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, rm_watch(fd, ::testing::_))
            .Times(0);
        return 0;
    }

    MOCK_METHOD(watch_handle, add_watch, (file_descriptor        fd,
                                          char            const *name,
                                          uint32_t               mask), (noexcept));
    watch_handle add_watch_followers(file_descriptor        fd,
                                     char            const *name,
                                     uint32_t               mask)
    {
        EXPECT_EQ(IN_DELETE_SELF, mask & IN_DELETE_SELF); // Be aware of directory deletion
        EXPECT_EQ(IN_MOVE_SELF,   mask & IN_MOVE_SELF);   // Be aware of directory movement
        EXPECT_TRUE(active_fds.count(fd) > 0);
        EXPECT_GT(INT_MAX, handle_count);
        watch_handle current_handle = watches_enabled ? ++handle_count : -1;
        if(current_handle != -1)
        {
            active_watches.emplace(current_handle, name);
            EXPECT_CALL(*this, rm_watch(fd, current_handle))
                .Times(Exactly(1))
                .RetiresOnSaturation();
        }
        return current_handle;
    }

    MOCK_METHOD(int, rm_watch, (file_descriptor fd,
                                watch_handle    wd), (noexcept));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, init1(::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(WithArgs<0>(Invoke(this, &mock_inotify::init1_followers)));
        EXPECT_CALL(*this, close(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, add_watch(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, rm_watch(::testing::_, ::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_INOTIFY_HPP_
//---- End of source file ------------------------------------------------------
