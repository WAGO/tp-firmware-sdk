// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <map>
#include <string>

namespace netconf {

class HostConfFileParser {
 public:
  HostConfFileParser() = default;
  virtual ~HostConfFileParser() = default;

  HostConfFileParser(const HostConfFileParser&) = default;
  HostConfFileParser& operator=(const HostConfFileParser&) = default;
  HostConfFileParser(HostConfFileParser&&) = default;
  HostConfFileParser& operator=(HostConfFileParser&&) = default;

  void Parse(const ::std::string &content);

  ::std::string GetHostname();
  ::std::string GetDomain();

 private:
  ::std::map<::std::string, ::std::string> values_;
};

} /* namespace netconf */

