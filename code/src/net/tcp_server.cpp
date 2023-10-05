#include "net/tcp_server.h"
#include "net/acceptor.h"
#include "net/event_loop.h"

#include <atomic>
#include <string>

using namespace base;

namespace net {

class TcpServer::Impl {
public:
  Impl(const std::string &name, const InetAddress &listenAddr,
       int numsThreads = 1);
  ~Impl();

private:
  const std::string name_;
  const InetAddress listenAddr_;
  std::shared_ptr<EventLoop> acceptorLoop_;
  std::unique_ptr<Acceptor> acceptor_;
  int numThreads_;
  std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;
  std::atomic<bool> runing_;
};

TcpServer::Impl::Impl(const std::string &name, const InetAddress &listenAddr,
                      int numsThreads)
    : name_(name), listenAddr_(listenAddr),
      acceptorLoop_(std::make_shared<EventLoop>()), numThreads_(numsThreads),
      acceptor_(std::make_unique<Acceptor>(acceptorLoop_, listenAddr_)),
      eventLoopThreadPool_(
          std::make_unique<EventLoopThreadPool>(std::string(), numsThreads)),
      runing_(false) {}

} // namespace net