#ifndef __EVENT_LOOP_THREAD_H__
#define __EVENT_LOOP_THREAD_H__
#include <memory>
#include <string>
#include <vector>

#include "base/noncopyable.h"
#include "net/channel.h"

namespace net {
/**
 * @brief 相当于 Reactor 中的 dispatcher，
 * 主要负责事件的注册、更新、取消和分发
 */
class EventLoop {
  NOCOPYABLE_DECLARE(EventLoop)
public:
  EventLoop();
  ~EventLoop();

  void loop();

  void stop();

  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);

  static EventLoop *getEventLoopOfCurrentThread();

  void assertInLoopThread();

  bool isInLoopThread();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

class EventLoopThread {
public:
  EventLoopThread(const std::string &name = std::string());
  ~EventLoopThread();

  void loop();
  void stop();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

class EventLoopThreadPool {
public:
  EventLoopThreadPool(const std::string &name = std::string(),
                      int numThreads = 1);
  ~EventLoopThreadPool();

  void setThreadNum(int numThreads);

  void start();

  void stop();

  std::vector<std::shared_ptr<EventLoopThread>> getAllEventLoop();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace net

#endif