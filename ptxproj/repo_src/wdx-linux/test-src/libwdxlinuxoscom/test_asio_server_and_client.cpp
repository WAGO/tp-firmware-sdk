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
///  \brief    Test Boost ASIO server and client implementations.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "asio/asio_server.hpp"
#include "asio/asio_client.hpp"

#include "mocks/mock_receiver.hpp"

#include <gtest/gtest.h>

#include <cstdlib>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::asio_client;
using wago::wdx::linuxos::com::asio_server;
using wago::wdx::linuxos::com::asio_adapter;
using wago::wdx::linuxos::com::adapter_i;
using wago::wdx::linuxos::com::connect_notifier;

using testing::Invoke;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
using after_connection = std::function<void(mock_receiver &, std::unique_ptr<adapter_i>)>;
static std::string create_socket_path();
static void test_server_start(asio_server      &server,
                              bool             &connection_handler_called,
                              after_connection  on_connection = nullptr);
static mock_receiver & test_client_connect(asio_client      &client, 
                                           bool             &connection_handler_called,
                                           after_connection  on_connection = nullptr,
                                           connect_notifier  on_connect_notifier = nullptr);

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(asio_server, construct_delete)
{
    auto socket_path = create_socket_path();
    asio_server server("test connection", socket_path);
}

TEST(asio_client, construct_delete)
{
    auto socket_path = create_socket_path();
    asio_client client("test connection", socket_path);
}

TEST(asio_server, start)
{
    auto socket_path = create_socket_path();
    asio_server server("test connection", socket_path);

    bool connection_handler_called = false;
    test_server_start(server, connection_handler_called, [](auto &, auto) {
        FAIL(); // no connection expected at all.
    });
    EXPECT_FALSE(connection_handler_called);
}

TEST(asio_client, connect)
{
    auto socket_path = create_socket_path();
    asio_client client("test connection", socket_path);
    
    bool connection_handler_called = false;
    test_client_connect(client, connection_handler_called);

    EXPECT_TRUE(connection_handler_called);
}

TEST(asio_server_and_client, connection_establishment)
{
    auto socket_path = create_socket_path();
    asio_server server("test server connection", socket_path);
    asio_client client("test client connection", socket_path);

    bool server_connection_handler_called = false;
    test_server_start(server, server_connection_handler_called);

    bool client_connection_handler_called = false;
    test_client_connect(client, client_connection_handler_called);

    // client connection handler has been called already
    EXPECT_TRUE(client_connection_handler_called);
    EXPECT_FALSE(server_connection_handler_called);
    EXPECT_FALSE(client.is_connected());

    // server connection handler is called as well after the server did run
    server.run_once();
    EXPECT_TRUE(server_connection_handler_called);
    EXPECT_FALSE(client.is_connected());

    // client is actually connected as soon as the client did receive its run
    client.run_once();
    EXPECT_TRUE(client.is_connected());
}


TEST(asio_server_and_client, connection_establishment_multiple_clients)
{
    auto socket_path = create_socket_path();
    asio_server server("test server connection", socket_path);
    std::vector<std::pair<std::shared_ptr<asio_client>, bool>> clients;

    clients.push_back(std::move(std::make_pair(std::make_shared<asio_client>("test client connection 1", socket_path), false)));
    clients.push_back(std::move(std::make_pair(std::make_shared<asio_client>("test client connection 2", socket_path), false)));

    bool   server_connection_handler_called = false;
    size_t server_connection_handler_count  = 0;
    test_server_start(server, server_connection_handler_called, [&server_connection_handler_count](auto&, auto) {
        server_connection_handler_count++;
    });

    for (auto &client : clients)
    {
        auto &client_connection_handler_called = client.second;
        test_client_connect(*client.first, client.second);
        // client connection handler has been called already
        EXPECT_TRUE(client_connection_handler_called);
        EXPECT_FALSE(client.first->is_connected());
    }
    EXPECT_FALSE(server_connection_handler_called);

    // server connection handler is called as well after the server did run
    while(server.run_once());
    EXPECT_TRUE(server_connection_handler_called);
    EXPECT_EQ(clients.size(), server_connection_handler_count);

    for (auto &client : clients)
    {
        EXPECT_FALSE(client.first->is_connected());

        // client is actually connected as soon as the client did receive its run
        client.first->run_once();
        EXPECT_TRUE(client.first->is_connected());
    }
}


TEST(asio_server_and_client, send_and_receive_echos)
{
    auto socket_path = create_socket_path();
    asio_server server("test server connection", socket_path);
    asio_client client("test client connection", socket_path);

    bool server_connection_handler_called = false;
    std::unique_ptr<adapter_i>  server_adapter  = nullptr;
    mock_receiver              *server_receiver = nullptr;
    test_server_start(server, server_connection_handler_called, [&server_receiver, &server_adapter](auto &receiver, auto adapter) {
        server_receiver = &receiver;
        server_adapter  = std::move(adapter);
    });

    bool client_connection_handler_called = false;
    bool client_connection_established    = false;
    std::unique_ptr<adapter_i>  client_adapter  = nullptr;
    mock_receiver              *client_receiver = nullptr;
    test_client_connect(client, client_connection_handler_called, [&client_receiver, &client_adapter](auto &receiver, auto adapter) {
        client_receiver = &receiver;
        client_adapter  = std::move(adapter);
    }, [&client_connection_established]() {
        client_connection_established = true;
    });

    // establish the connection
    server.run_once();
    client.run_once();
    EXPECT_TRUE(client.is_connected());
    EXPECT_TRUE(client_connection_established);

    // adapters and receivers must be existing now
    ASSERT_NE(nullptr, server_adapter);
    ASSERT_NE(nullptr, server_receiver);
    ASSERT_NE(nullptr, client_adapter);
    ASSERT_NE(nullptr, client_receiver);

    message_data test_message = {0x42};

    bool echo_received = false;
    auto message_from_client = test_message;
    client_adapter->send(std::move(message_from_client), [](auto error) {
        if (!error.empty())
        {
            // there should be no error
            FAIL();
        }
    });

    EXPECT_CALL(*server_receiver, receive(testing::ContainerEq(test_message)))
        .Times(1)
        .WillRepeatedly(Invoke([&server_adapter](auto incoming_message){
            server_adapter->send(std::move(incoming_message), [](auto error) {
                if (!error.empty())
                {
                    // there should be no error
                    FAIL();
                }
            });
        }));

    EXPECT_CALL(*client_receiver, receive(testing::ContainerEq(test_message)))
        .Times(1)
        .WillRepeatedly(Invoke([&echo_received](auto){
            echo_received = true;
        }));

    // run both as often as needed
    while(server.run_once() || client.run_once());

    EXPECT_TRUE(echo_received);
}

//------------------------------------------------------------------------------
// function implementations
//------------------------------------------------------------------------------
static std::string create_socket_path()
{
    char dir_template[] = "/tmp/test_asio_XXXXXX";
    char * dir = ::mkdtemp(dir_template);
    if (dir == nullptr)
    {
        std::cout << "error=" << errno << std::endl;
    }
    std::string socket_path = std::string(dir) + "/my.socket";
    return socket_path;
}

static void test_server_start(asio_server      &server,
                              bool             &connection_handler_called,
                              after_connection  on_connection)
{
    server.start([&connection_handler_called, on_connection](std::unique_ptr<adapter_i> adapter) -> std::unique_ptr<receiver_i>{
        connection_handler_called = true;
        std::unique_ptr<mock_receiver> receiver_mock = std::make_unique<mock_receiver>();
        receiver_mock->set_default_expectations();
        if (on_connection)
        {
            on_connection(*receiver_mock, std::move(adapter));
        }
        return receiver_mock;
    });
}

static mock_receiver & test_client_connect(asio_client      &client,
                                           bool             &connection_handler_called,
                                           after_connection  on_connection,
                                           connect_notifier  on_connect_notifier)
{
    return client.connect<mock_receiver>([&connection_handler_called, on_connection](std::unique_ptr<adapter_i> adaper) -> std::unique_ptr<mock_receiver> {
        connection_handler_called = true;
        std::unique_ptr<mock_receiver> receiver_mock = std::make_unique<mock_receiver>();
        receiver_mock->set_default_expectations();
        if (on_connection)
        {
            on_connection(*receiver_mock, std::move(adaper));
        }
        return receiver_mock;
    }, on_connect_notifier);
}

//---- End of source file ------------------------------------------------------
