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
#ifndef SRC_LIBWDXCORE_UTILS_PROVIDER_JOB_HPP_
#define SRC_LIBWDXCORE_UTILS_PROVIDER_JOB_HPP_

#include "job_queue.hpp"
#include "wago/future.hpp"
#include "wago/wdx/parameter_exception.hpp"

#include <wc/structuring.h>
#include <functional>

namespace wago {
namespace wdx {

template <class ReturnType>
class provider_job final : public job_i {
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(provider_job)
public:
    provider_job(provider_job &&) noexcept = default;
    provider_job &operator=(provider_job &&) noexcept = default;

    provider_job(std::shared_ptr<wago::promise<ReturnType>> promise_, std::function<wago::future<ReturnType>()> provider_call_)
    : provider_call(std::move(provider_call_))
    , promise(std::move(promise_))
    {}

    ~provider_job() noexcept override
    {
        cancel();
    }

    void start(std::function<void()> on_complete) noexcept override
    {
        try
        {
            pending_future = std::make_unique<wago::future<ReturnType>>(provider_call());
            pending_future->set_notifier([p = promise, on_complete](auto &&result){
                p->set_value(std::move(result));
                on_complete();
            });
            pending_future->set_exception_notifier([p = promise, on_complete](auto &&ex) {
                p->set_exception(std::move(ex));
                on_complete();
            });
        }
        catch(...)
        {

            // Neither set_notifier nor set_exception_notifier should throw
            WC_FAIL("Unexpected exception caught in provider_job::start");
            try
            {
                promise->set_exception(std::current_exception());
            }
            catch(...) {} // parasoft-suppress CERT_CPP-ERR56-b "There actually is nothing to-do when the promise already is satisfied. But the catch is required for the destructor to be noexcept."
            on_complete();
        }
    }

    void cancel() noexcept override
    {
        try
        {
            // TODO: if(promise.execute() and promise ready) promise->set_exception() -> give better information to the caller, instead of 'broken promise'.
            // could future.hpp be changed so that promise.execute() is also false when notifier has been called? -> check with Mariusz
            if(pending_future && !pending_future->ready())
            {
                pending_future->set_notifier([](auto&&){});
                pending_future->set_exception_notifier([](auto&&){});
                pending_future->dismiss();
                if(promise->execute())
                {
                    throw parameter_exception("Serial wrapper has been cleaned up before the response has been received.");
                }
            }
        }
        catch(...)
        {
            try
            {
                promise->set_exception(std::current_exception());
            }
            catch(...) {} // parasoft-suppress CERT_CPP-ERR56-b "There actually is nothing to-do when the promise already is satisfied. But the catch is required for the destructor to be noexcept."
        }
    }

private:
    std::function<wago::future<ReturnType>()>  provider_call;
    std::shared_ptr<wago::promise<ReturnType>> promise;
    std::unique_ptr<wago::future<ReturnType>>  pending_future;
};

}
}

#endif // SRC_LIBWDXCORE_UTILS_PROVIDER_JOB_HPP_
