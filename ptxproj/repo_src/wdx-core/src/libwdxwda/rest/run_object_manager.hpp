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
///
///  \brief    Manager to handle run objects.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_RUN_OBJECT_MANAGER_HPP_
#define SRC_LIBWDXWDA_REST_RUN_OBJECT_MANAGER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/responses.hpp"
#include <wago/future.hpp>
#include <wc/std_includes.h>

#include <string>
#include <map>
#include <memory>
#include <mutex>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {


struct method_run_object {

    method_run_object(std::string                  const &run_id,
                      wdx::parameter_instance_path        instance,
                      uint16_t                            result_timeout_span)
    : id(run_id)
    , method(instance)
    , timeout_span(result_timeout_span)
    { }

    method_run_object(std::string                           const &run_id,
                      wdx::parameter_instance_path                 instance,
                      uint16_t                                     result_timeout_span,
                      uint64_t                                     result_timeout_time,
                      wdx::method_invocation_named_response      &&response)
    : id(run_id)
    , method(instance)
    , timeout_span(result_timeout_span)
    , timeout_time(result_timeout_time)
    , invocation_response(std::make_shared<wdx::method_invocation_named_response>(std::move(response)))
    {}

    std::string id;
    wdx::parameter_instance_path                           method;
    uint16_t                                               timeout_span;              // timespan  in seconds
    uint64_t                                               timeout_time = UINT64_MAX; // timestamp in seconds
    std::shared_ptr<wdx::method_invocation_named_response> invocation_response;

    uint32_t get_timeout_left() const;
    std::string get_id() const;
};

class run_object_manager_i
{
public:
    using ready_handler = std::function<void(std::string run_id)>;
    run_object_manager_i() = default;
    virtual ~run_object_manager_i() noexcept = default;

    virtual std::string add_run(wdx::parameter_instance_path     const &method_instance_path,
                                uint16_t                                timeout_span,
                                wdx::method_invocation_named_response &&response) = 0;
    virtual std::string add_run(wdx::parameter_instance_path            const &method_instance_path,
                                uint16_t                                       timeout_span,
                                future<wdx::method_invocation_named_response> &&invoke_future,
                                ready_handler                                   handler) = 0;
    virtual std::shared_ptr<method_run_object> get_run(wdx::parameter_instance_path const &method_instance_path,
                                                       std::string                         run_id) = 0;
    virtual std::vector<std::shared_ptr<method_run_object>> get_runs(wdx::parameter_instance_path const &method_instance_path) = 0;
    virtual void remove_run(wdx::parameter_instance_path const &method_instance_path,
                            std::string                         run_id) = 0;
    virtual void clean_runs() = 0;
    virtual bool max_runs_reached() = 0;
};

class run_object_manager: public run_object_manager_i
{
public:
    static constexpr uint16_t const max_runs = 1000;

    using ready_handler = std::function<void(std::string run_id)>;
    run_object_manager()                    = default;
    ~run_object_manager() noexcept override = default;

    std::string add_run(wdx::parameter_instance_path     const &method_instance_path,
                        uint16_t                                timeout_span,
                        wdx::method_invocation_named_response &&response) override;
    std::string add_run(wdx::parameter_instance_path             const &method_instance_path,
                        uint16_t                                        timeout_span,
                        future<wdx::method_invocation_named_response> &&invoke_future,
                        ready_handler                                   handler) override;
    std::shared_ptr<method_run_object> get_run(wdx::parameter_instance_path const &method_instance_path,
                                               std::string                         run_id) override;
    std::vector<std::shared_ptr<method_run_object>> get_runs(wdx::parameter_instance_path const &method_instance_path) override;
    void remove_run(wdx::parameter_instance_path const &method_instance_path,
                    std::string                         run_id) override;
    void clean_runs() override;
    bool max_runs_reached() override;

private:
    std::mutex                                                change_mutex_m;
    std::map<std::string, std::shared_ptr<method_run_object>> runs_map_m;
};


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_RUN_OBJECT_MANAGER_HPP_
//---- End of source file ------------------------------------------------------
