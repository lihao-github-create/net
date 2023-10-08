#ifndef __INET_ADDRESS_H__
#define __INET_ADDRESS_H__
#include <stdint.h>
#include <string>
namespace net {
class InetAddress {
public:
  InetAddress();
  InetAddress(const char *ip, uint16_t port);
  ~InetAddress();

  InetAddress(const InetAddress &other) = default;

  uint16_t getPort() const;
  std::string getIp() const;

  void setAddress(const char *ip, uint16_t port);

  bool ifAnyAddr(void) const;

private:
  std::string ip_;
  uint16_t port_;
};
} // namespace net

#endif