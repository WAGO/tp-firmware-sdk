//------------------------------------------------------------------------------
// Copyright (c) 2025 WAGO GmbH & Co. KG
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
///  \brief    Test job queue.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "utils/job_queue.hpp"

#include <memory>
#include <atomic>
#include <thread>
#include <chrono>

#include <gtest/gtest.h>


class test_job : public wago::wdx::job_i
{
public:
    test_job(std::atomic_bool &start_marker,
             std::atomic_bool &cancel_marker,
             std::shared_ptr<std::atomic_bool> finish_start  = nullptr,
             std::shared_ptr<std::atomic_bool> finish_cancel = nullptr)
    : start_marker_m(start_marker)
    , cancel_marker_m(cancel_marker)
    , finish_start_m(finish_start)
    , finish_cancel_m(finish_cancel)
    { }

    void start(completion_handler on_complete) noexcept override
    {
        start_marker_m = true;
        while((finish_start_m.get() != nullptr) && (*finish_start_m == false))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if(cancel_marker_m)
        {
            return;
        }
        on_complete();
    }

    void cancel() noexcept override
    {
        cancel_marker_m = true;
        while((finish_cancel_m.get() != nullptr) && (*finish_cancel_m == false))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

private:
    std::atomic_bool &start_marker_m;
    std::atomic_bool &cancel_marker_m;
    std::shared_ptr<std::atomic_bool> finish_start_m;
    std::shared_ptr<std::atomic_bool> finish_cancel_m;
};

struct job_queue_test_fixture : public ::testing::Test
{
    std::unique_ptr<wago::wdx::job_queue> test_queue;

    void SetUp() override 
    {
        test_queue = std::make_unique<wago::wdx::job_queue>();
    }

    void TearDown() override 
    {
        
    }
};

TEST_F(job_queue_test_fixture, create_and_destroy)
{
    test_queue.reset();
    EXPECT_EQ(nullptr, test_queue.get());
}

TEST_F(job_queue_test_fixture, enqueue_and_start)
{
    std::atomic_bool start_marker( false);
    std::atomic_bool cancel_marker(false);
    test_queue->enqueue_job(std::make_shared<test_job>(start_marker, cancel_marker));

    EXPECT_TRUE( start_marker);
    EXPECT_FALSE(cancel_marker);
}

TEST_F(job_queue_test_fixture, enqueue_and_cancel)
{
    std::atomic_bool start_marker( false);
    std::atomic_bool cancel_marker(false);
    auto             allow_start  = std::make_shared<std::atomic_bool>(false);
    auto             allow_cancel = std::make_shared<std::atomic_bool>(true);

    std::thread worker([this, &start_marker=start_marker, &cancel_marker=cancel_marker, allow_start, allow_cancel](){
        test_queue->enqueue_job(std::make_shared<test_job>(start_marker, cancel_marker, allow_start, allow_cancel));
    });

    while(!start_marker)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    test_queue->cancel_jobs();
    *allow_start = true;
    worker.join();

    EXPECT_TRUE(start_marker);
    EXPECT_TRUE(cancel_marker);
}

TEST_F(job_queue_test_fixture, enqueue_and_destroy)
{
    std::atomic_bool start_marker( false);
    std::atomic_bool cancel_marker(false);
    auto             allow_start  = std::make_shared<std::atomic_bool>(false);
    auto             allow_cancel = std::make_shared<std::atomic_bool>(true);

    std::thread worker([this, &start_marker=start_marker, &cancel_marker=cancel_marker, allow_start, allow_cancel](){
        test_queue->enqueue_job(std::make_shared<test_job>(start_marker, cancel_marker, allow_start, allow_cancel));
    });

    while(!start_marker)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    test_queue.reset();
    EXPECT_EQ(nullptr, test_queue.get());
    *allow_start = true;
    worker.join();

    EXPECT_TRUE(start_marker);
    EXPECT_TRUE(cancel_marker);
}

TEST_F(job_queue_test_fixture, cancel_after_start_done)
{
    std::atomic_bool start_marker( false);
    std::atomic_bool cancel_marker(false);
    auto             allow_start  = std::make_shared<std::atomic_bool>(false);
    auto             allow_cancel = std::make_shared<std::atomic_bool>(true);

    std::thread worker([this, &start_marker=start_marker, &cancel_marker=cancel_marker, allow_start, allow_cancel](){
        test_queue->enqueue_job(std::make_shared<test_job>(start_marker, cancel_marker, allow_start, allow_cancel));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    *allow_start = true;
    worker.join();
    test_queue->cancel_jobs();

    EXPECT_TRUE( start_marker);
    EXPECT_FALSE(cancel_marker);
}

TEST_F(job_queue_test_fixture, long_running_cancel)
{
    std::atomic_bool start_marker( false);
    std::atomic_bool cancel_marker(false);
    auto             allow_start  = std::make_shared<std::atomic_bool>(false);
    auto             allow_cancel = std::make_shared<std::atomic_bool>(false);

    std::thread starter([this, &start_marker=start_marker, &cancel_marker=cancel_marker, allow_start, allow_cancel](){
        test_queue->enqueue_job(std::make_shared<test_job>(start_marker, cancel_marker, allow_start, allow_cancel));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    std::thread cancelor([this](){
        test_queue->cancel_jobs();
    });
    *allow_start = true;
    starter.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    *allow_cancel = true;
    cancelor.join();

    EXPECT_TRUE(start_marker);
    EXPECT_TRUE(cancel_marker);
}

TEST_F(job_queue_test_fixture, enqueue_multiple_jobs)
{
    std::atomic_bool start_marker_1( false);
    std::atomic_bool cancel_marker_1(false);
    std::atomic_bool start_marker_2( false);
    std::atomic_bool cancel_marker_2(false);
    std::atomic_bool start_marker_3( false);
    std::atomic_bool cancel_marker_3(false);

    test_queue->enqueue_job(std::make_shared<test_job>(start_marker_1, cancel_marker_1));
    test_queue->enqueue_job(std::make_shared<test_job>(start_marker_2, cancel_marker_2));
    test_queue->enqueue_job(std::make_shared<test_job>(start_marker_3, cancel_marker_3));

    EXPECT_TRUE( start_marker_1);
    EXPECT_FALSE(cancel_marker_1);
    EXPECT_TRUE( start_marker_2);
    EXPECT_FALSE(cancel_marker_2);
    EXPECT_TRUE( start_marker_3);
    EXPECT_FALSE(cancel_marker_3);
}

TEST_F(job_queue_test_fixture, enqueue_multiple_jobs_from_different_threads)
{
    std::atomic_bool start_marker_1( false);
    std::atomic_bool cancel_marker_1(false);
    std::atomic_bool start_marker_2( false);
    std::atomic_bool cancel_marker_2(false);
    std::atomic_bool start_marker_3( false);
    std::atomic_bool cancel_marker_3(false);
    auto             allow_start    = std::make_shared<std::atomic_bool>(false);

    std::thread worker([this, &start_marker=start_marker_1, &cancel_marker=cancel_marker_1, allow_start](){
        test_queue->enqueue_job(std::make_shared<test_job>(start_marker, cancel_marker, allow_start));
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    std::thread starter_1([this, &start_marker=start_marker_2, &cancel_marker=cancel_marker_2, allow_start](){
        test_queue->enqueue_job(std::make_shared<test_job>(start_marker, cancel_marker, allow_start));
    });
    std::thread starter_2([this, &start_marker=start_marker_3, &cancel_marker=cancel_marker_3, allow_start](){
        test_queue->enqueue_job(std::make_shared<test_job>(start_marker, cancel_marker, allow_start));
    });

    // Jobs started after an active worker should only enqueue and return
    starter_1.join();
    starter_2.join();

    // Allow first job to finish, first job thread is the worker for all
    *allow_start = true;
    worker.join();

    EXPECT_TRUE( start_marker_1);
    EXPECT_FALSE(cancel_marker_1);
    EXPECT_TRUE( start_marker_2);
    EXPECT_FALSE(cancel_marker_2);
    EXPECT_TRUE( start_marker_3);
    EXPECT_FALSE(cancel_marker_3);
}

TEST_F(job_queue_test_fixture, enqueue_job_from_different_thread_after_last_job_is_done)
{
    std::atomic_bool start_marker_1( false);
    std::atomic_bool cancel_marker_1(false);
    std::atomic_bool start_marker_2( false);
    std::atomic_bool cancel_marker_2(false);

    std::atomic_bool enqueue_marker_1(false);
    std::thread worker([this, &start_marker=start_marker_1, &cancel_marker=cancel_marker_1, &enqueue_marker_1](){
        test_queue->enqueue_job(std::make_shared<test_job>(start_marker, cancel_marker));
        enqueue_marker_1 = true;
    });

    while(!enqueue_marker_1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    ASSERT_TRUE( start_marker_1);
    EXPECT_FALSE(cancel_marker_1);

    worker.join(); // Job is done when worker finished
    test_queue->enqueue_job(std::make_shared<test_job>(start_marker_2, cancel_marker_2));
    EXPECT_TRUE( start_marker_2);
    EXPECT_FALSE(cancel_marker_2);
}
