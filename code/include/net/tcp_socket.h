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
  int bind(const InetAddress &);
  int listen(int qs);
  int connect(const InetAddress &);
  int accept(InetAddress &);
  int read(char *buf, size_t len);
  int readn(char *vptr, unsigned int n);
  int write(const char *buf, size_t len);
  int writev(const struct iovec *, size_t);
  int getFd(void) const;
  int close(void);
  int shutdownWrite(void);

  int setNonblock(void);
  int enableReuseaddr(void);
  int disableLinger(void);
  int disableNagle(void);
  int setKeepAlive(void);

private:
  int m_sockFd;
};

} // namespace net

#endif
