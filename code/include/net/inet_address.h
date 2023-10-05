#ifndef __INET_ADDRESS_H__
#define __INET_ADDRESS_H__

namespace net {
class InetAddress {
public:
  InetAddress();
  ~InetAddress();

  InetAddress(const InetAddress &other);

  uint16_t getPort();
  std::string getIp();

private:
  uint16_t port_;
  std::string ip_;
};
} // namespace net

#endif