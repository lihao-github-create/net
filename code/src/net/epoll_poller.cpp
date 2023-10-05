#include "net/epoll_poller.h"
#include <assert.h>
#include <errno.h>
#include <glog/logging.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

namespace net {
static const int kInitEventListSize = 16;
class EPollPoller::Impl {
public:
  Impl(EventLoop *loop);
  ~Impl();

  void poll(int timeoutMs, ChannelList *activeChannels);
  void addChannel(Channel *channel);
  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);

private:
  void fillActiveChannels(int numEvents, ChannelList *activeChannels);
  bool epollCtl(int operation, Channel *channel);

  const char *operationToString(int op);

private:
  using EventList = std::vector<struct epoll_event>;
  // fd -> channel
  using ChannelMap = std::unordered_map<int, Channel *>;
  int epollFd_;
  EventList events_;
  ChannelMap channels_;
  EventLoop *ownerLoop_;
};

EPollPoller::Impl::Impl(EventLoop *loop)
    : epollFd_(epoll_create1(EPOLL_CLOEXEC)), ownerLoop_(loop),
      events_(kInitEventListSize) {}
EPollPoller::Impl::~Impl() { close(epollFd_); }

void EPollPoller::Impl::poll(int timeoutMs, ChannelList *activeChannels) {
  LOG(INFO) << "fd total count " << channels_.size();
  // 直至一个事件发生或timeoutMs到期，才返回
  int numEvents =
      epoll_wait(epollFd_, events_.data(), events_.size(), timeoutMs);
  int savedErrno = errno;
  // 处理已发生的事件
  if (numEvents > 0) {
    fillActiveChannels(numEvents, activeChannels);
  } else if (numEvents == 0) {
    LOG(INFO) << "nothing happened";
  } else {
    // error happens, log uncommon ones
    LOG(ERROR) << "EPollPoller::poll() error : " << strerror(savedErrno);
  }
}

void EPollPoller::Impl::addChannel(Channel *channel) {
  LOG(INFO) << "fd = " << channel->fd() << " events = " << channel->events();
  bool flag = epollCtl(EPOLL_CTL_ADD, channel);
  if (flag) {
    channels_[channel->fd()] = channel;
  }
}

void EPollPoller::Impl::updateChannel(Channel *channel) {
  LOG(INFO) << "fd = " << channel->fd() << " events = " << channel->events();
  epollCtl(EPOLL_CTL_MOD, channel);
}

void EPollPoller::Impl::removeChannel(Channel *channel) {
  LOG(INFO) << "fd = " << channel->fd() << " events = " << channel->events();
  epollCtl(EPOLL_CTL_DEL, channel);
  channels_.erase(channel->fd());
}

void EPollPoller::Impl::fillActiveChannels(int numEvents,
                                           ChannelList *activeChannels) {
  activeChannels->clear();
  for (int i = 0; i < numEvents; i++) {
    Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
    channel->set_revents(events_[i].events);
    activeChannels->push_back(channel);
  }
}

bool EPollPoller::Impl::epollCtl(int operation, Channel *channel) {
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  LOG(INFO) << "epoll_ctl op = " << operationToString(operation)
            << " fd = " << fd << " event = { " << channel->eventsToString()
            << " }";
  if (::epoll_ctl(epollFd_, operation, fd, &event) < 0) {
    LOG(ERROR) << "epoll_ctl op =" << operationToString(operation)
               << " fd =" << fd;
    return false;
  }
  return true;
}

const char *EPollPoller::Impl::operationToString(int op) {
  switch (op) {
  case EPOLL_CTL_ADD:
    return "ADD";
  case EPOLL_CTL_DEL:
    return "DEL";
  case EPOLL_CTL_MOD:
    return "MOD";
  default:
    return "Unknown Operation";
  }
}

/*******************************EPollPoller****************************************/
EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(), impl_(std::make_unique<EPollPoller::Impl>(loop)) {}

EPollPoller::~EPollPoller() {}

void EPollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
  impl_->poll(timeoutMs, activeChannels);
}
void EPollPoller::updateChannel(Channel *channel) {
  impl_->updateChannel(channel);
}
void EPollPoller::removeChannel(Channel *channel) {
  impl_->removeChannel(channel);
}
} // namespace net