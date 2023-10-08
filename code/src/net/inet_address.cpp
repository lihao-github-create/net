#include "net/inet_address.h"
#include <stdint.h>
#include <string>

namespace net {
const char *IPV4_ANYADDR = "0.0.0.0";
InetAddress::InetAddress() : ip_(IPV4_ANYADDR), port_(0) {}
InetAddress::InetAddress(const char *ip, uint16_t port)
    : ip_(ip), port_(port) {}
InetAddress::~InetAddress() {}

uint16_t InetAddress::getPort() const { return port_; }
std::string InetAddress::getIp() const { return ip_; }

void InetAddress::setAddress(const char *ip, uint16_t port) {
  ip_ = std::string(ip);
  port_ = port;
}

bool InetAddress::ifAnyAddr(void) const {
  std::string anyaddr(IPV4_ANYADDR);
  return anyaddr == ip_;
}
} // namespace net