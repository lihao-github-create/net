#ifndef __TCP_CONNECTION_H__
#define __TCP_CONNECTION_H__

#include "base/noncopyable.h"
#include "net/callbacks.h"
#include "string"
#include <memory>

namespace net {

class InetAddress;
class EventLoop;
/**
 * @brief 表示一个已经建立的 tcp 连接。
 */
class TcpConnection {
public:
  TcpConnection(std::weak_ptr<EventLoop> loop, const std::string &name,
                int sockfd, const InetAddress &localAddr,
                const InetAddress &peerAddr);
  ~TcpConnection();

  void send(const void *message, int len);
  // reading or not
  void startRead();
  void stopRead();
  // shutdown connection
  void shutdown();

  void setConnectionCallback(const ConnectionCallback &cb);

  void setMessageCallback(const MessageCallback &cb);

  void setWriteCompleteCallback(const WriteCompleteCallback &cb);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace net

#endif