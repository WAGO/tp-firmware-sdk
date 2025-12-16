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
#include "job_queue.hpp"

#include <wc/assertion.h>

namespace wago {
namespace wdx {

namespace {
void do_if_not_destroyed(shared_marker exit_marker, shared_mutex exit_mutex, std::function<void()> task);
}

job_queue::~job_queue() noexcept
{
    *exit_marker_m = true;
    try 
    {
        std::lock_guard<std::mutex> exit_lock_guard(*exit_mutex_m);
        cancel_jobs();
    }
    catch(...)
    {
        // Catch all exceptions to avoid throwing from destructor.
        // - std::lock_guard throws when recursivly locking the same mutex, which
        //   is most certainly not the case.
        // - cancel_jobs is also only potentially throwing because it also uses
        //   a lock_guard.
        WC_FAIL("Unexpected exception caught in job_queue destructor");
    }
}

void job_queue::enqueue_job(std::shared_ptr<job_i> job)
{
    bool needs_start = false;
    {
        std::lock_guard<std::mutex> jobs_lock_guard(jobs_mutex_m);
        needs_start = jobs_m.empty();
        jobs_m.push(std::move(job));
    }
    if (needs_start)
    {
        start_jobs(exit_marker_m, exit_mutex_m);
    }
}

void job_queue::cancel_jobs()
{
    std::lock_guard<std::mutex> jobs_lock_guard(jobs_mutex_m);
    while(!jobs_m.empty())
    {
        jobs_m.front()->cancel();
        jobs_m.pop();
    }
}


void job_queue::start_jobs(shared_marker exit_marker, shared_mutex exit_mutex)
{
    std::shared_ptr<job_i> next_job;
    do_if_not_destroyed(
        exit_marker, 
        exit_mutex, 
        [this, &next_job]() 
        {
            std::lock_guard<std::mutex> jobs_lock_guard(jobs_mutex_m);
            if(!jobs_m.empty())
            {
                next_job = jobs_m.front();
            }
        }
    );
    if (next_job != nullptr)
    {
        next_job->start([this, exit_marker, exit_mutex, current_job_ptr=next_job.get()]() {

            bool next_job_available = false;
            do_if_not_destroyed(
                exit_marker, 
                exit_mutex, 
                [this, current_job_ptr, &next_job_available]() // parasoft-suppress CERT_CPP-EXP61-b-2 "Inner lambda is executed synchronously, lifetime will not be longer than variabes from outer lambda"
                {
                    // remove the job that just finished
                    std::lock_guard<std::mutex> jobs_lock_guard(jobs_mutex_m);
                    if(!jobs_m.empty() && jobs_m.front().get() == current_job_ptr)
                    {
                        jobs_m.pop();
                    }                
                    next_job_available = !jobs_m.empty();
                }
            );

            // start next job if avaialble
            if (next_job_available)
            {
                start_jobs(exit_marker, exit_mutex);
            }
        });
    }
}

namespace
{
    void do_if_not_destroyed(shared_marker exit_marker, shared_mutex exit_mutex, std::function<void()> task)
    {
        // check if we need to exit before aquiiring the lock to avoid waiting
        // for destructor to finish
        if (*exit_marker)
        {
            return;
        }

        // prevent this queue to be destructed in parallel by aquiring the lock
        std::lock_guard<std::mutex> exit_lock_guard(*exit_mutex);

        // check again if we need to exit, if a destruction took place before we
        // aquired the lock
        if (*exit_marker)
        {
            return;
        }

        task();   
    }
}

}
}
