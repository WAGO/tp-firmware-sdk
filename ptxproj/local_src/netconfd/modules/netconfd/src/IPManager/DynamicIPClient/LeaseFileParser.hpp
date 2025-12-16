// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <map>

#include "BaseTypes.hpp"


namespace netconf {

class LeaseFileParser {
 public:
  LeaseFileParser() = default;
  virtual ~LeaseFileParser() = default;

  LeaseFileParser(const LeaseFileParser&) = default;
  LeaseFileParser& operator=(const LeaseFileParser&) = default;

  LeaseFileParser(LeaseFileParser&&) = default;
  LeaseFileParser& operator=(LeaseFileParser&&) = default;

  void Parse(const ::std::string &content);

  Address GetAddress();
  Netmask GetNetmask();
  ::std::string GetDHCPHostname();
  ::std::string GetDHCPDomain();

 private:
  ::std::map<::std::string, ::std::string> values_;
};

} /* namespace netconf */

