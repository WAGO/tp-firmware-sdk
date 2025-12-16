//------------------------------------------------------------------------------
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
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
///  \brief    Test file parameter handler.
///
///  \author   Röh: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include "wago/wdx/linuxos/file/file_parameter_handler.hpp"
#include "wago/wdx/linuxos/file/exception.hpp"
#include "mock_file_provider_extended.hpp"

#include <wago/future.hpp>
#include <wago/wdx/wdmm/base_types.hpp>
#include <wago/wdx/test/mock_backend.hpp>
#include <wago/wdx/test/wda_check.hpp>

#include <gtest/gtest.h>
//#include "file_provider_extended_override.hpp"

using wago::wdx::test::mock_backend;
using wago::wdx::parameter_id_t;
using wago::wdx::linuxos::file::file_parameter_handler;
using wago::wdx::linuxos::file::file_provider_extended_i;
using wago::wdx::linuxos::file::mock_file_provider_extended;
using mock_file_provider_factory = std::unique_ptr<mock_file_provider_extended>(bool readonly);

using ::testing::Exactly;
using ::testing::AtLeast;
using ::testing::AtMost;
using ::testing::AnyNumber;
using ::testing::WithArgs;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ByMove;


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

class file_parameter_handler_fixture : public ::testing::Test
{
public:
    mock_backend                               backend_mock;
    std::string                                file_id_read  = "ABC";
    std::string                                file_id_write = "WAGO";
    parameter_id_t                             param_id = 42;
    std::function<mock_file_provider_factory>  file_provider_factory_mock;

    struct Deleter
    {
        //Called by unique_ptr to destroy/free the Resource
        void operator()(mock_file_provider_extended*)
        {
        }
    };

    void SetUp() override
    {
        backend_mock.set_default_expectations();
    }

    std::vector<wago::wdx::response> resp_vect(bool success)
    {
        std::vector<wago::wdx::response> responses;
        wago::wdx::response my_response;
        my_response.status = success ? wago::wdx::status_codes::success : wago::wdx::status_codes::internal_error;
        responses.push_back(my_response);
        return responses;
    }

    std::vector<wago::wdx::file_id_response> resp_vect_file_id(bool success)
    {
        return resp_vect_file_id(success, file_id_read);
    }

    std::vector<wago::wdx::file_id_response> resp_vect_file_id(bool success, std::string file_id)
    {
        std::vector<wago::wdx::file_id_response> responses;
        wago::wdx::file_id_response my_response;
        my_response.status = success ? wago::wdx::status_codes::success : wago::wdx::status_codes::internal_error;
        my_response.registered_file_id = file_id;
        responses.push_back(my_response);
        return responses;
    }
};

TEST_F(file_parameter_handler_fixture, constructor_fail_bad_ptr)
{
    EXPECT_THROW(file_parameter_handler my_handler(param_id, backend_mock, file_provider_factory_mock), std::bad_function_call);
}

// FIXME: Correct and re-enable unit tests

TEST_F(file_parameter_handler_fixture, constructor)
{
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true)))));
    EXPECT_CALL(backend_mock, unregister_file_providers(::testing::_))
        .Times(Exactly(1));
    file_parameter_handler my_handler(param_id, backend_mock, [](bool){
            return std::make_unique<mock_file_provider_extended>();
    });
}

TEST_F(file_parameter_handler_fixture, constructor_bad_register)
{
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(false)))));
    file_parameter_handler my_handler(param_id, backend_mock, [](bool){
        return std::make_unique<mock_file_provider_extended>();
    });

    auto file_id_gotten_future = my_handler.get_file_id();
    ASSERT_FUTURE_EXCEPTION(file_id_gotten_future);
    EXPECT_THROW(file_id_gotten_future.get(), wago::wdx::linuxos::file::exception);
}

TEST_F(file_parameter_handler_fixture, get_file_id)
{
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true)))));
    EXPECT_CALL(backend_mock, unregister_file_providers(::testing::_))
        .Times(Exactly(1));
    file_parameter_handler my_handler(param_id, backend_mock, [](bool){
        return std::make_unique<mock_file_provider_extended>();
    });

    auto file_id_gotten_future = my_handler.get_file_id();
    ASSERT_FUTURE_VALUE(file_id_gotten_future);
    auto file_id_gotten = file_id_gotten_future.get();
    EXPECT_EQ(file_id_read, file_id_gotten->get_file_id());
}

TEST_F(file_parameter_handler_fixture, create_file_id_for_write)
{
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, ::testing::_))
        .Times(Exactly(2))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true)))))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true, file_id_write)))));
    EXPECT_CALL(backend_mock, unregister_file_providers(::testing::_))
        .Times(Exactly(2));
    file_parameter_handler my_handler(param_id, backend_mock, [](bool){
        return std::make_unique<mock_file_provider_extended>();
    });

    auto file_id_gotten_future = my_handler.create_file_id_for_write(param_id);
    ASSERT_FUTURE_VALUE(file_id_gotten_future);
    auto file_id_gotten = file_id_gotten_future.get();
    EXPECT_EQ(file_id_write, file_id_gotten->get_file_id());
}

TEST_F(file_parameter_handler_fixture, remove_file_id_for_write)
{
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, ::testing::_))
        .Times(Exactly(2))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true)))))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true, file_id_write)))));
    EXPECT_CALL(backend_mock, unregister_file_providers(::testing::_))
        .Times(Exactly(2));

    file_parameter_handler my_handler(param_id, backend_mock, [](bool){
        return std::make_unique<mock_file_provider_extended>();
    });
    my_handler.create_file_id_for_write(param_id);
    my_handler.remove_file_id_for_write(file_id_write);
}

TEST_F(file_parameter_handler_fixture, remove_file_id_for_write_no_create_called)
{
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true)))));
    EXPECT_CALL(backend_mock, unregister_file_providers(::testing::_))
        .Times(Exactly(1));

    file_parameter_handler my_handler(param_id, backend_mock, [](bool){
        return std::make_unique<mock_file_provider_extended>();
    });
    EXPECT_THROW(my_handler.remove_file_id_for_write(file_id_read), wago::wdx::linuxos::file::exception);
}

TEST_F(file_parameter_handler_fixture, remove_file_id_for_write_wrong_file_id)
{
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, ::testing::_))
        .Times(Exactly(2))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true)))))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true)))));
    EXPECT_CALL(backend_mock, unregister_file_providers(::testing::_))
        .Times(Exactly(2));

    file_parameter_handler my_handler(param_id, backend_mock, [](bool){
        return std::make_unique<mock_file_provider_extended>();
    });
    my_handler.create_file_id_for_write(param_id);
    EXPECT_THROW(my_handler.remove_file_id_for_write("FILEID"), wago::wdx::linuxos::file::exception);
}

TEST_F(file_parameter_handler_fixture, set_file_id_for_write)
{
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, ::testing::_))
        .Times(Exactly(2))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true)))))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true, file_id_write)))));
    EXPECT_CALL(backend_mock, unregister_file_providers(::testing::_))
        .Times(Exactly(2));
    auto my_complete_result = true;

    auto my_read_mock = std::make_unique<mock_file_provider_extended>();
    my_read_mock->set_default_expectations();
    auto my_write_mock = std::make_unique<mock_file_provider_extended>();
    my_write_mock->set_default_expectations();
    EXPECT_CALL(*my_write_mock, is_complete())
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(my_complete_result)))));
    EXPECT_CALL(*my_write_mock, finish())
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future())));
    EXPECT_CALL(*my_write_mock, validate(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future())));

    file_parameter_handler my_handler(param_id, backend_mock,
                                      ////NOLINTNEXTLINE (use after move, but move is used in ecpect_call)
                                      [&my_complete_result, &my_read_mock, &my_write_mock](bool readonly)mutable{
                                            if(readonly) return std::move(my_read_mock);
                                            else         return std::move(my_write_mock);
                                      });
    auto file_id_gotten_future = my_handler.create_file_id_for_write(param_id);
    ASSERT_FUTURE_VALUE(file_id_gotten_future);
    auto file_id_gotten = file_id_gotten_future.get();
    EXPECT_EQ(file_id_write, file_id_gotten->get_file_id());
    wago::future<void> set_future = my_handler.set_file_id(file_id_gotten, [](std::basic_istream<uint8_t>& ){return true;});
    EXPECT_NO_THROW(set_future.get());
    file_id_gotten_future = my_handler.get_file_id();
    ASSERT_FUTURE_VALUE(file_id_gotten_future);
    file_id_gotten = file_id_gotten_future.get();
    EXPECT_EQ(file_id_write, file_id_gotten->get_file_id());
}

TEST_F(file_parameter_handler_fixture, set_file_id_for_write_no_create_called)
{
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true)))));
    EXPECT_CALL(backend_mock, unregister_file_providers(::testing::_))
        .Times(Exactly(1));

    file_parameter_handler my_handler(param_id, backend_mock, [](bool){
        return std::make_unique<mock_file_provider_extended>();
    });
    std::shared_ptr<wago::wdx::parameter_value> other_file_id;
    wago::future<void> set_future = my_handler.set_file_id(other_file_id, [](std::basic_istream<uint8_t>& ){return true;});
    ASSERT_FUTURE_EXCEPTION(set_future);
    EXPECT_THROW(set_future.get(), wago::wdx::linuxos::file::exception);
}

TEST_F(file_parameter_handler_fixture, set_file_id_for_write_wrong_file_id)
{
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, ::testing::_))
        .Times(Exactly(2))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true)))))
        .WillOnce(Return(ByMove(wago::resolved_future(resp_vect_file_id(true)))));
    EXPECT_CALL(backend_mock, unregister_file_providers(::testing::_))
        .Times(Exactly(2));

    file_parameter_handler my_handler(param_id, backend_mock, [](bool){
        auto my_mock = std::make_unique<mock_file_provider_extended>();
        my_mock->set_default_expectations();
        return my_mock;
    });
    my_handler.create_file_id_for_write(param_id);
    auto other_file_id = wago::wdx::parameter_value::create_file_id("XYZ");
    wago::future<void> set_future = my_handler.set_file_id(other_file_id, [](std::basic_istream<uint8_t>& ){return true;});
    ASSERT_FUTURE_EXCEPTION(set_future);
    EXPECT_THROW(set_future.get(), wago::wdx::linuxos::file::exception);
}


//---- End of source file ------------------------------------------------------
