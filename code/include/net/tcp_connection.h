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
  NOCOPYABLE_DECLARE(TcpConnection)
public:
  TcpConnection(EventLoop *loop, const std::string &name, int sockfd,
                const InetAddress &localAddr, const InetAddress &peerAddr);
  ~TcpConnection();
  // send, maybe cross-thread call, but thread safe
  void send(const char *message, int len);

  // reading or not, maybe cross-thread call, but thread safe
  void startRead();
  void stopRead();

  // shutdown connection, maybe cross-thread call, but thread safe
  void shutdownWrite();

  // callback
  void setConnectionCallback(const ConnectionCallback &cb);
  void setMessageCallback(const MessageCallback &cb);
  void setWriteCompleteCallback(const WriteCompleteCallback &cb);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace net

#endif