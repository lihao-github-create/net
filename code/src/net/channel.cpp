#include "net/channel.h"
#include <glog/logging.h>
#include <poll.h>
#include <sstream>
#include <sys/epoll.h>

namespace net {
static const int kNoneEvent = 0;
static const int kReadEvent = POLLIN | POLLPRI;
static const int kWriteEvent = POLLOUT;

class Channel::Impl {
public:
  Impl(Channel *channel, EventLoop *loop, int fd);
  ~Impl();
  void hanleEvent();

  // set callback
  void setReadCallback(EventCallback cb);
  void setWriteCallback(EventCallback cb);
  void setCloseCallback(EventCallback cb);
  void setErrorCallback(EventCallback cb);

  // used by pollers
  int fd() const;
  int events() const;
  void set_revents(int revt);
  std::string eventsToString();

  // events register and unregister
  void enableReading();
  void disableReading();
  void enableWriting();
  void disableWriting();
  void disableAll();

private:
  Channel *ownerChannel_;
  EventLoop *ownerLoop_;
  int fd_;
  int events_;
  int revents_; // it's the received event types of epoll or poll
  EventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

Channel::Impl::Impl(Channel *channel, EventLoop *loop, int fd)
    : ownerChannel_(channel), ownerLoop_(loop), fd_(fd) {}

Channel::Impl::~Impl() {}

void Channel::Impl::hanleEvent() {
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    LOG(WARNING) << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
    if (closeCallback_)
      closeCallback_();
  }
  if (revents_ & POLLNVAL) {
    LOG(WARNING) << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
  }
  if (revents_ & (POLLERR | POLLNVAL)) {
    if (errorCallback_)
      errorCallback_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) { // readable
    if (readCallback_)
      readCallback_();
  }
  if (revents_ & POLLOUT) { // writable
    if (writeCallback_)
      writeCallback_();
  }
}

// set callback
void Channel::Impl::setReadCallback(EventCallback cb) {
  readCallback_ = std::move(cb);
}
void Channel::Impl::setWriteCallback(EventCallback cb) {
  writeCallback_ = std::move(cb);
}
void Channel::Impl::setCloseCallback(EventCallback cb) {
  closeCallback_ = std::move(cb);
}
void Channel::Impl::setErrorCallback(EventCallback cb) {
  errorCallback_ = std::move(cb);
}

// used by pollers
int Channel::Impl::fd() const { return fd_; }
int Channel::Impl::events() const { return events_; }
void Channel::Impl::set_revents(int revt) { revents_ = revt; }
std::string Channel::Impl::eventsToString() {
  std::ostringstream oss;
  oss << fd_ << ": ";
  if (events_ & POLLIN)
    oss << "IN ";
  if (events_ & POLLPRI)
    oss << "PRI ";
  if (events_ & POLLOUT)
    oss << "OUT ";
  if (events_ & POLLHUP)
    oss << "HUP ";
  if (events_ & POLLRDHUP)
    oss << "RDHUP ";
  if (events_ & POLLERR)
    oss << "ERR ";
  if (events_ & POLLNVAL)
    oss << "NVAL ";

  return oss.str();
}

// events register and unregister
void Channel::Impl::enableReading() {
  events_ |= kReadEvent;
  ownerLoop_->updateChannel(ownerChannel_);
}

void Channel::Impl::disableReading() {
  events_ &= ~kReadEvent;
  ownerLoop_->updateChannel(ownerChannel_);
}

void Channel::Impl::enableWriting() {
  events_ |= kWriteEvent;
  ownerLoop_->updateChannel(ownerChannel_);
}

void Channel::Impl::disableWriting() {
  events_ &= ~kWriteEvent;
  ownerLoop_->updateChannel(ownerChannel_);
}

void Channel::Impl::disableAll() {
  events_ = kNoneEvent;
  ownerLoop_->updateChannel(ownerChannel_);
}

/************************************Channel**********************************/
Channel::Channel(EventLoop *loop, int fd)
    : impl_(std::make_unique<Impl>(loop, fd)) {}
Channel::~Channel() {}

void Channel::hanleEvent() { impl_->hanleEvent(); }

// set callback
void Channel::setReadCallback(EventCallback cb) { impl_->setReadCallback(cb); }
void Channel::setWriteCallback(EventCallback cb) {
  impl_->setWriteCallback(cb);
}
void Channel::setCloseCallback(EventCallback cb) {
  impl_->setCloseCallback(cb);
}
void Channel::setErrorCallback(EventCallback cb) {
  impl_->setErrorCallback(cb);
}

// used by pollers
int Channel::fd() const { return impl_->fd(); }
int Channel::events() const { return impl_->events(); }
void Channel::set_revents(int revt) { impl_->set_revents(revt); }
std::string Channel::eventsToString() { return impl_->eventsToString(); }

// events register and unregister
void Channel::enableReading() { impl_->enableReading(); }
void Channel::disableReading() { impl_->disableReading(); }
void Channel::enableWriting() { impl_->enableWriting(); }
void Channel::disableWriting() { impl_->disableWriting(); }
void Channel::disableAll() { impl_->disableAll(); }

} // namespace net