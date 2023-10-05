#ifndef __CONNECTION_MANAGER_H__
#define __CONNECTION_MANAGER_H__

#include "base/singleton.h"

#include <memory>
#include <string>

namespace net {

class TcpConnection;
class InetAddress;
class TcpConnectionManager {
  SINGLETON_PATTERN_DECLARE(TcpConnectionManager)
public:
  TcpConnectionManager();
  ~TcpConnectionManager();

  std::shared_ptr<TcpConnection> createConnection(const std::string &name,
                                                  int sockfd,
                                                  const InetAddress &localAddr,
                                                  const InetAddress &peerAddr);
  std::shared_ptr<TcpConnection> getConnection(int id);
  void destroyById(int id);
  void destroyAll();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace net

#endif