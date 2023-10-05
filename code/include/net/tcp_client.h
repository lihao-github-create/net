#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include "base/noncopyable.h"
#include "net/callbacks.h"
#include <memory>
#include <string>

namespace net {
class InetAddress;
class TcpClient {
  NOCOPYABLE_DECLARE(TcpClient)
public:
  TcpClient(const InetAddress &serverAddr, const std::string name);
  ~TcpClient();

  // connect, blocking
  void connecting();
  void disconnect();
  void stop();

  // retry
  bool retry() const;
  void enableRetry();

  // callback
  void setConnectionCallback(ConnectionCallback cb);
  void setMessageCallback(MessageCallback cb);
  void setWriteCompleteCallback(WriteCompleteCallback cb);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace net

#endif