#include "net/acceptor.h"
#include "net/event_loop.h"
#include "net/tcp_socket.h"
#include <glog/logging.h>
#include <memory>
namespace net {
class Acceptor::Impl {
public:
  Impl(EventLoop *loop, const InetAddress &listenAddr);
  ~Impl();

  void setNewConnectionCallback(const NewConnectionCallback &cb);

  // start listening
  void listen();

private:
  // 当有新连接时，将回调它
  void handleRead();

private:
  EventLoop *ownerLoop_;
  TCPSocket acceptSocket_;
  Channel acceptChannel_;

  NewConnectionCallback newConnectionCallback_;
};

Acceptor::Impl::Impl(EventLoop *loop, const InetAddress &listenAddr)
    : ownerLoop_(loop), acceptSocket_(TCPSocket::generateSocket()),
      acceptChannel_(ownerLoop_, acceptSocket_.getFd()) {
  acceptSocket_.enableReuseaddr();
  acceptSocket_.bind(listenAddr);
  acceptChannel_.setReadCallback([&]() { handleRead(); });
}

Acceptor::Impl::~Impl() { acceptSocket_.close(); }

void Acceptor::Impl::setNewConnectionCallback(const NewConnectionCallback &cb) {
  newConnectionCallback_ = cb;
}

// start listening
void Acceptor::Impl::listen() {
  acceptSocket_.listen(SOMAXCONN);
  acceptChannel_.enableReading();
}

void Acceptor::Impl::handleRead() {
  ownerLoop_->assertInLoopThread();
  InetAddress peerAddr;
  int connfd = acceptSocket_.accept(peerAddr);
  if (connfd >= 0) {
    if (newConnectionCallback_) {
      newConnectionCallback_(connfd, peerAddr);
    }
  } else {
    LOG(ERROR) << "accept() error";
  }
}
/**************************************Acceptor************************************/
Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr) {}
Acceptor::~Acceptor() {}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback &cb) {}
// start listening
void Acceptor::listen() {}

} // namespace net