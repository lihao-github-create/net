#include "net/event_loop.h"
#include "net/channel.h"
#include "net/epoll_poller.h"
#include "net/poller.h"

#include <atomic>
#include <glog/logging.h>
#include <mutex>
#include <thread>

namespace net {

const int kPollTimeMs = 10;
thread_local EventLoop *t_loopInThisThread = nullptr;

class EventLoop::Impl {
public:
  using ChannelList = std::vector<Channel *>;
  Impl();
  ~Impl();

  void loop();

  void stop();
  // must be in loop thread
  void addChannel(Channel *channel);
  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);

  void assertInLoopThread();

  bool isInLoopThread();

private:
  std::atomic<bool> looping_;
  pid_t threadId_;
  std::unique_ptr<Poller> poller_;
  std::mutex mutex_;
};

EventLoop::Impl::Impl()
    : looping_(true), threadId_(pthread_self()),
      poller_(std::make_unique<EPollPoller>(this)) {
  // 后续改为 poller_根据配置文件设置
}

void EventLoop::Impl::loop() {
  assertInLoopThread();
  LOG(INFO) << "EventLoop " << this << " start looping";
  ChannelList activeChannell;
  while (looping_) {
    poller_->poll(kPollTimeMs, &activeChannell);
    for (auto &channel : activeChannell) {
      channel->hanleEvent();
    }
  }
  LOG(INFO) << "EventLoop " << this << " stop looping";
}

void EventLoop::Impl::stop() {
  // 终止 looping
  looping_ = false;
}

void EventLoop::Impl::updateChannel(Channel *channel) {
  assertInLoopThread();
  poller_->updateChannel(channel);
}
void EventLoop::Impl::removeChannel(Channel *channel) {
  assertInLoopThread();
  poller_->removeChannel(channel);
}

void EventLoop::Impl::assertInLoopThread() {
  if (isInLoopThread()) {
    LOG(FATAL) << "EventLoop::abortNotInLoopThread - EventLoop " << this
               << " was created in threadId_ = " << threadId_
               << ", current thread id = " << pthread_self();
  }
}

bool EventLoop::Impl::isInLoopThread() { return threadId_ == pthread_self(); }

EventLoop::EventLoop() : impl_(std::make_unique<EventLoop::Impl>()) {
  if (t_loopInThisThread) {
    LOG(FATAL) << "Another EventLoop " << t_loopInThisThread
               << " exists in this thread " << pthread_self();
  } else {
    t_loopInThisThread = this;
  }
}

EventLoop::~EventLoop() {}

void EventLoop::loop() { impl_->loop(); }

void EventLoop::stop() { impl_->stop(); }

void EventLoop::updateChannel(Channel *channel) {
  impl_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel *channel) {
  impl_->removeChannel(channel);
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}

void EventLoop::assertInLoopThread() { impl_->assertInLoopThread(); }

bool EventLoop::isInLoopThread() { impl_->isInLoopThread(); }
} // namespace net