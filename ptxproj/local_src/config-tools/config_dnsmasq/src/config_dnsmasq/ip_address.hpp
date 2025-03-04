//------------------------------------------------------------------------------
/// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
///
/// PROPRIETARY RIGHTS of WAGO GmbH & Co. KG are involved in
/// the subject matter of this material. All manufacturing, reproduction,
/// use, and sales rights pertaining to this subject matter are governed
/// by the license agreement. The recipient of this software implicitly
/// accepts the terms of the license.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///
///  \brief    Class representation of a port configuration
///
///  \author   MSc : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

#ifndef SRC_CONFIG_DNSMASQ_IPADDRESS_HPP_
#define SRC_CONFIG_DNSMASQ_IPADDRESS_HPP_

#include <string>

namespace configdnsmasq {

class ip_address {
 public:
  explicit ip_address(const std::string &address);
  explicit ip_address(uint32_t address);
  ip_address(const ip_address& other) = default;
  ip_address& operator=(const ip_address& other) = default;
  ip_address(ip_address&& address) = default;
  ip_address& operator=(ip_address&& other) = default;
  virtual ~ip_address() = default;

  std::string asString() const;
  uint32_t asBinary() const;
  bool isZero() const;

 private:
  std::string string_;
  uint32_t binary_;
};

} // namespace configdnsmasq

#endif /* SRC_CONFIG_DNSMASQ_IPADDRESS_HPP_ */
