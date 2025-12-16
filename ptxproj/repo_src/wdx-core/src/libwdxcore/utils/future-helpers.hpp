//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------
#include <functional>
#include <exception>
#include <wago/future.hpp>

namespace wago {
namespace wdx {

template <typename T1, typename T2>
wago::future<T2> mapped_future(wago::future<T1>&& f, std::function<T2(T1&&)> transformer) {
    auto p = std::make_shared<wago::promise<T2>>();

    try {
        f.set_exception_notifier([p](std::exception_ptr eptr) {
            p->set_exception(eptr);
        });
        f.set_notifier([p, transformer](T1&& r) {
            p->set_value(transformer(std::move(r)));
        });
    }
    catch(const std::exception& ex) {
        p->set_exception(std::make_exception_ptr(ex));
    }
    return p->get_future();
}

template <typename T>
wago::future<void> void_future(wago::future<T>&& f) {
    auto p = std::make_shared<wago::promise<void>>();
    try {
        f.set_exception_notifier([p](std::exception_ptr eptr) {
            p->set_exception(eptr);
        });
        f.set_notifier([p](T&& r) {
            (void)(r);
            p->set_value();
        });
    }
    catch(const std::exception& ex) {
        p->set_exception(std::make_exception_ptr(ex));
    }
    return p->get_future();
}

}}