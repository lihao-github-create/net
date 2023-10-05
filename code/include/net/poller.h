#ifndef __DEMULTIPLEXER_H__
#define __DEMULTIPLEXER_H__

#include "net/channel.h"
#include "net/event_loop.h"
#include <unordered_map>
#include <vector>

namespace net {
class Poller {
public:
  using ChannelList = std::vector<Channel *>;

  Poller() = default;
  virtual ~Poller() = default;

  /// Polls the I/O events.
  /// Must be called in the loop thread.
  virtual void poll(int timeoutMs, ChannelList *activeChannels) = 0;

  /// Add the interested I/O events.
  /// Must be called in the loop thread.
  virtual void addChannel(Channel *channel) = 0;

  /// Changes the interested I/O events.
  /// Must be called in the loop thread.
  virtual void updateChannel(Channel *channel) = 0;

  /// Remove the channel, when it destructs.
  /// Must be called in the loop thread.
  virtual void removeChannel(Channel *channel) = 0;
};

} // namespace net
#endif