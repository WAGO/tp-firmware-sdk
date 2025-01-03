#pragma once

#include <string>
#include <vector>


namespace wago::snmp_config_lib {

enum class StatusCode : ::std::uint32_t {
  OK = 0,
  WRONG_PARAMETER_PATTERN = 1,
};

class Status {
 public:
  Status() = default;
  explicit Status(StatusCode code);
  Status(StatusCode code, ::std::string &&additional_information_);
  virtual ~Status() = default;
  Status(const Status &other) = default;
  Status(Status &&other) = default;
  Status& operator=(const Status &other) = default;
  Status& operator=(Status &&other) = default;

  ::std::string ToString() const;
  bool IsOk() const;

  bool operator == (const Status &other) const {
    return status_code_ == other.status_code_ && additional_information_ == other.additional_information_;
  }

 private:
  StatusCode status_code_ = StatusCode::OK;
  ::std::string additional_information_;
};

class Statuses {
 public:
  Statuses() = default;
  virtual ~Statuses() = default;
  Statuses(const Statuses &other) = default;
  Statuses(Statuses &&other) = default;
  Statuses& operator=(const Statuses &other) = delete;
  Statuses& operator=(Statuses &&other) = delete;

  void AddErrorStatus(Status&& status);
  void AddErrorStatuses(Statuses&& statuses);

  const ::std::vector<Status>& GetStatuses() const;

  bool IsOk() const;

 private:
  ::std::vector<Status> statuses_;

};

}
