//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXCORE_UTILS_JOB_QUEUE_HPP_
#define SRC_LIBWDXCORE_UTILS_JOB_QUEUE_HPP_

#include <queue>
#include <mutex>
#include <memory>
#include <atomic>
#include <functional>

#include <wc/structuring.h>

namespace wago {
namespace wdx {

using shared_marker = std::shared_ptr<std::atomic<bool>>;
using shared_mutex  = std::shared_ptr<std::mutex>;

class job_i {
    WC_INTERFACE_CLASS(job_i)

public:
    using completion_handler = std::function<void()>;
    virtual void start(completion_handler on_complete) noexcept = 0;
    virtual void cancel() noexcept = 0;
};

/// A queue for serial execution of asyncronous tasks. Tasks managed by the
/// queue will be processed sequentially one after another.
class job_queue final {
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(job_queue)

private:
    std::mutex                         jobs_mutex_m;
    std::queue<std::shared_ptr<job_i>> jobs_m;

    shared_marker exit_marker_m = std::make_shared<std::atomic<bool>>(false);
    shared_mutex  exit_mutex_m  = std::make_shared<std::mutex>();
public:

    /// New, empty queue
    job_queue() = default;

    /// Destroys the queue. Upon destruction, pending and the potential running
    /// job will be canceled by calling their `cancel` method.
    ~job_queue() noexcept;

    /// \brief Adds a new job to the queue. The job will be started as soon as 
    /// possible.
    /// 
    /// Either it starts just right away, or it gets enqueued to start
    /// after the preceeding job has been completed (called its completion
    /// handler). In the latter case, a job will not start, when the queue's 
    /// `cancel_jobs` method has been called or the queue itself got destructed.
    ///
    /// \param job The job to be added to the queue.
    void enqueue_job(std::shared_ptr<job_i> job);

    /// \brief Cancels all pending and a potentially started job by clearing the
    /// queue and by calling the `cancel` method of all jobs.
    void cancel_jobs();

private:
    void start_jobs(shared_marker exit_marker, shared_mutex exit_mutex);
};

}
}

#endif // SRC_LIBWDXCORE_UTILS_JOB_QUEUE_HPP_
