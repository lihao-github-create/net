#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "net/event_loop.h"
#include <functional>
#include <memory>

namespace net {
/**
 * @brief 负责单一fd上的事件分发
 */
class Channel {
public:
  using EventCallback = std::function<void()>;
  Channel(EventLoop *loop, int fd);
  ~Channel();

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

  bool isWriting() const;
  bool isReading() const;
  bool isNoneEvent() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace net

#endif