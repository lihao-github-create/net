#ifndef __SOCKET_H__
#define __SOCKET_H__
#include "net/inet_address.h"

namespace net {
/**
 * @brief
 */
class Socket {
  explicit Socket(int sockFd) : sockFd_(sockFd) {}
  int fd() const { return sockFd_; }

  /// abort if address in use
  void bindAddress(const InetAddress &localaddr);
  /// abort if address in use
  void listen();

  /// On success, returns a non-negative integer that is
  /// a descriptor for the accepted socket, which has been
  /// set to non-blocking and close-on-exec. *peeraddr is assigned.
  /// On error, -1 is returned, and *peeraddr is untouched.
  int accept(InetAddress *peeraddr);

  void shutdownWrite();

private:
  const int sockFd_;
};

} // namespace net

#endif