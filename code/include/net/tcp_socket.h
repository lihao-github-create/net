#ifndef __SOCKET_H__
#define __SOCKET_H__
#include "net/inet_address.h"
#include <stddef.h>
#include <sys/socket.h>

namespace net {
/**
 * @brief
 */
class TCPSocket {
public:
  TCPSocket(int fd);
  ~TCPSocket();

  static int generateSocket(void);
  bool bind(const InetAddress &);
  bool listen(int qs);
  bool connect(const InetAddress &);
  int accept(InetAddress &);
  int read(char *buf, size_t len);
  int readn(char *vptr, unsigned int n);
  int write(const char *buf, size_t len);
  int writev(const struct iovec *, size_t);
  int getFd(void) const;
  bool close(void);
  bool shutdownWrite(void);

  bool setNonblock(void);
  bool enableReuseaddr(void);
  bool setKeepAlive(void);

private:
  int sockFd_;
};

} // namespace net

#endif
