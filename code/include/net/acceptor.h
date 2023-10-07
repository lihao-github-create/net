#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include "base/noncopyable.h"
#include "net/event_loop.h"
#include "net/inet_address.h"

#include <functional>
#include <memory>

namespace net {
class Acceptor {
  NOCOPYABLE_DECLARE(Acceptor)
public:
  using NewConnectionCallback =
      std::function<void(int sockfd, const InetAddress &)>;
  Acceptor(EventLoop *loop, const InetAddress &listenAddr);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback &cb);
  // start listening
  void listen();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace net

#endif
