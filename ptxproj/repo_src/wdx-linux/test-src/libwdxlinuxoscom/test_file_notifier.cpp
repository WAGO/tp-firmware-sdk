//------------------------------------------------------------------------------
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
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
///  \brief    Test inotify implementation for socket.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "asio/file_notifier.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"

#include "mocks/mock_inotify.hpp"
#include "mocks/mock_filesystem.hpp"

#include <boost/asio.hpp>
#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::file_notifier;
using boost::asio::io_context;
using testing::AtLeast;
using testing::AnyNumber;
using testing::Return;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class file_notifier_fixture : public testing::Test
{
public:
    mock_inotify          inotify_mock;
    mock_filesystem       filesystem_mock;

    std::string     const parent_dir;
    std::string     const requested_file;
    std::string     const full_path;
    io_context            context;

    bool                  handler_called_for_success;
    bool                  handler_called_for_error;

    file_notifier::available_handler default_handler = [this](bool success){
        if(success)
        {
            handler_called_for_success = true;
        }
        else
        {
            handler_called_for_error = true;
        }
    };

    file_notifier_fixture()
    : parent_dir("/some/test/directory")
    , requested_file("test_file")
    , full_path(parent_dir + "/" + requested_file)
    {}

    ~file_notifier_fixture() override = default;

    void SetUp() override
    {
        inotify_mock.set_default_expectations();
        filesystem_mock.set_default_expectations();
        handler_called_for_success = false;
        handler_called_for_error   = false;
    }

    void run_context()
    {
        do
        {
            if(context.stopped())
            {
                context.restart();
            }
        }
        while(context.poll_one());
    }
};

TEST_F(file_notifier_fixture, construct_delete)
{
    file_notifier notifier(context, file_notifier::type::directory, full_path);
}

TEST_F(file_notifier_fixture, directory_present)
{
    file_notifier notifier(context, file_notifier::type::directory, full_path);

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));

    notifier.async_wait_for_file(default_handler);
    run_context();

    EXPECT_TRUE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, socket_present)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));

    notifier.async_wait_for_file(default_handler);
    run_context();

    EXPECT_TRUE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_parent_not_present_no_event)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const next_parent_dir = parent_dir.substr(0, parent_dir.find_last_of('/'));

    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(next_parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(next_parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));

    notifier.async_wait_for_file(default_handler);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_no_event)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_parent_initially_not_present_matching_create_event_for_socket)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename  = requested_file;
    std::string const next_parent_dir = parent_dir.substr(0, parent_dir.find_last_of('/'));
    std::string const parent_dir_name = parent_dir.substr(parent_dir.find_last_of('/') + 1);

    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(next_parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(next_parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));

    notifier.async_wait_for_file(default_handler);
    run_context();
    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
    testing::Mock::VerifyAndClearExpectations(&filesystem_mock);

    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));
    inotify_mock.write_event(parent_dir_name, IN_CREATE | IN_ISDIR);
    run_context();
    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
    testing::Mock::VerifyAndClearExpectations(&filesystem_mock);

    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    inotify_mock.write_event(event_filename, IN_CREATE);
    run_context();
    EXPECT_TRUE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_matching_create_event_for_directory)
{
    file_notifier notifier(context, file_notifier::type::directory, full_path);
    std::string const event_filename = requested_file;

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    notifier.async_wait_for_file(default_handler);
    run_context();
    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);

    inotify_mock.write_event(event_filename, IN_CREATE | IN_ISDIR);
    run_context();
    EXPECT_TRUE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_matching_create_event_for_socket)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = requested_file;

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    notifier.async_wait_for_file(default_handler);
    run_context();
    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);

    inotify_mock.write_event(event_filename, IN_CREATE);
    run_context();
    EXPECT_TRUE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_matching_move_event_for_directory)
{
    file_notifier notifier(context, file_notifier::type::directory, full_path);
    std::string const event_filename = requested_file;

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    notifier.async_wait_for_file(default_handler);
    run_context();
    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);

    inotify_mock.write_event(event_filename, IN_MOVED_TO | IN_ISDIR);
    run_context();
    EXPECT_TRUE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_matching_move_event_for_socket)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = requested_file;

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    notifier.async_wait_for_file(default_handler);
    run_context();
    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);

    inotify_mock.write_event(event_filename, IN_MOVED_TO);
    run_context();
    EXPECT_TRUE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_multiple_events_one_matching_move_event)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename_1 = "some other file 1";
    std::string const event_filename_2 = "some other file 2";
    std::string const event_filename_3 = "some other file 3";
    std::string const event_filename_4 = requested_file;
    std::string const event_filename_5 = "some other file x";

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));

    notifier.async_wait_for_file(default_handler);
    run_context();
    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);

    inotify_mock.write_event(event_filename_1, IN_DELETE);
    inotify_mock.write_event(event_filename_2, IN_CREATE);
    inotify_mock.write_event(event_filename_3, IN_MOVED_TO);
    inotify_mock.write_event(event_filename_4, IN_MOVED_TO);
    inotify_mock.write_event(event_filename_5, IN_CREATE);
    run_context();
    EXPECT_TRUE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_matching_filename_but_directory_event_for_socket)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = requested_file;

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);

    inotify_mock.write_event(event_filename, IN_MOVED_TO | IN_ISDIR);
    run_context();
    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_matching_filename_but_nondirectory_event)
{
    file_notifier notifier(context, file_notifier::type::directory, full_path);
    std::string const event_filename = requested_file;

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);

    inotify_mock.write_event(event_filename, IN_MOVED_TO);
    run_context();
    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_matching_filename_but_nonsocket_event)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = requested_file;

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);

    inotify_mock.write_event(event_filename, IN_MOVED_TO);
    run_context();
    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_event_with_wrong_filename)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = "some-other filename.trash";

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    inotify_mock.write_event(event_filename, IN_CREATE);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_event_for_directory_with_filename_prefixed)
{
    file_notifier notifier(context, file_notifier::type::directory, full_path);
    std::string const event_filename = "x" + requested_file;

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    inotify_mock.write_event(event_filename, IN_CREATE | IN_ISDIR);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_event_for_directory_with_filename_postfixed)
{
    file_notifier notifier(context, file_notifier::type::directory, full_path);
    std::string const event_filename = requested_file + "y";

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    inotify_mock.write_event(event_filename, IN_CREATE | IN_ISDIR);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_event_with_filename_prefixed)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = "x" + requested_file;

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    inotify_mock.write_event(event_filename, IN_CREATE);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_event_with_filename_postfixed)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = requested_file + "y";

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    inotify_mock.write_event(event_filename, IN_CREATE);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_event_for_directory_without_filename)
{
    file_notifier notifier(context, file_notifier::type::directory, full_path);
    std::string const event_filename = "";

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    inotify_mock.write_event(event_filename, IN_CREATE | IN_ISDIR);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_event_without_filename)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = "";

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    inotify_mock.write_event(event_filename, IN_CREATE);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_event_for_directory_with_wrong_event_type)
{
    file_notifier notifier(context, file_notifier::type::directory, full_path);
    std::string const event_filename = requested_file;

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    inotify_mock.write_event(event_filename, IN_MOVED_FROM | IN_ISDIR);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_event_with_wrong_event_type)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = requested_file;

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    inotify_mock.write_event(event_filename, IN_MOVED_FROM);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_watch_destroied_by_delete)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = "";

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    inotify_mock.write_event(event_filename, IN_DELETE_SELF);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_TRUE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_watch_destroied_by_move)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = "";

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    inotify_mock.write_event(event_filename, IN_MOVE_SELF);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_TRUE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_watch_obsolete)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = "";

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    notifier.async_wait_for_file(default_handler);
    inotify_mock.write_event(event_filename, IN_IGNORED);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_TRUE(handler_called_for_error);
}

TEST_F(file_notifier_fixture, not_present_watch_init_failed)
{
    inotify_mock.disallow_init();
    EXPECT_THROW(file_notifier notifier(context, file_notifier::type::socket, full_path), wago::wdx::linuxos::com::exception);
}

TEST_F(file_notifier_fixture, not_present_watch_failed)
{
    file_notifier notifier(context, file_notifier::type::socket, full_path);
    std::string const event_filename = requested_file;

    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_dir), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_dir)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(full_path)))
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    inotify_mock.disable_watches();
    EXPECT_THROW(notifier.async_wait_for_file(default_handler), wago::wdx::linuxos::com::exception);
    run_context();

    EXPECT_FALSE(handler_called_for_success);
    EXPECT_FALSE(handler_called_for_error);
}


//---- End of source file ------------------------------------------------------
