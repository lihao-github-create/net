#ifndef __EPOLL_DEMULTIPLEXER_H__
#define __EPOLL_DEMULTIPLEXER_H__

#include "net/poller.h"
#include <memory>

namespace net {
class EPollPoller : public Poller {
public:
  EPollPoller(EventLoop *loop);
  ~EPollPoller() override;

  void poll(int timeoutMs, ChannelList *activeChannels) override;
  void updateChannel(Channel *channel) override;
  void removeChannel(Channel *channel) override;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace net
#endif