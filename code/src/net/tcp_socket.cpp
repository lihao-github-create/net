#include "net/tcp_socket.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <glog/logging.h>

namespace net {

TCPSocket::TCPSocket(int fd) : sockFd_(fd) {}
TCPSocket::~TCPSocket() {}

// fail return -1
int TCPSocket::generateSocket(void) {
  // create socket
  return ::socket(PF_INET, SOCK_STREAM, 0);
}

// bind
bool TCPSocket::bind(const InetAddress &servaddr) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = PF_INET;

  if (servaddr.ifAnyAddr()) {
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    if ((inet_aton(servaddr.getIp().c_str(),
                   (in_addr *)&addr.sin_addr.s_addr)) == 0) {
      LOG(ERROR) << "addr invalid";
      return false;
    }
  }

  if (servaddr.getPort() == 0) {
    LOG(ERROR) << "port invalid";
    return false;
  }
  addr.sin_port = htons(servaddr.getPort());
  if (::bind(sockFd_, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
    return false;
  }
  return true;
}

// listen
bool TCPSocket::listen(int qs) {
  if (::listen(sockFd_, qs) < 0) {
    return false;
  }
  return true;
}

bool TCPSocket::connect(const InetAddress &ipaddr) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = PF_INET;
  if (ipaddr.ifAnyAddr()) {
    LOG(ERROR) << "TCPSocket::connect:ipaddr.ip NULL";
    return false;
  } else {
    if ((addr.sin_addr.s_addr = inet_addr(ipaddr.getIp().c_str())) ==
        INADDR_NONE) {
      LOG(ERROR) << "IP Address Invalid";
      return false;
    }
  }
  if (ipaddr.getPort() == 0) {
    LOG(ERROR) << "Port Invalid";
    return false;
  }
  addr.sin_port = htons(ipaddr.getPort());

  int ret = ::connect(sockFd_, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    return false;
  }
  return true;
}

// accept a new socket fd, fail return -1
int TCPSocket::accept(InetAddress &addr) {
  struct sockaddr_in cliAddr;
  unsigned int cliAddrLen = sizeof(cliAddr);
  memset(&cliAddr, 0, cliAddrLen);
  int fd = ::accept(sockFd_, (struct sockaddr *)&cliAddr, &cliAddrLen);
  addr.setAddress(inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));
  return fd;
}

// read
int TCPSocket::read(char *buf, size_t len) {
  int readNum = ::read(sockFd_, buf, len);
  return readNum;
}
int TCPSocket::readn(char *vptr, unsigned int n) {
  assert(vptr != nullptr);
  LOG(ERROR) << "not implement";
  assert(false);
  return -1;
}

// write
int TCPSocket::write(const char *buf, size_t len) {
  int writeNum = ::write(sockFd_, buf, len);
  return writeNum;
}
int TCPSocket::writev(const struct iovec *v, size_t c) {
  int writeNum = ::writev(sockFd_, v, c);
  return writeNum;
}

int TCPSocket::getFd(void) const { return sockFd_; }

// close
bool TCPSocket::close(void) {
  if (sockFd_ == -1) {
    return true;
  }
  if (::close(sockFd_) < 0) {
    return false;
  }
  sockFd_ = -1;
  return true;
}

bool TCPSocket::shutdownWrite(void) {
  if (::shutdown(sockFd_, SHUT_WR) < 0) {
    return false;
  }
  return true;
}

bool TCPSocket::setNonblock(void) {
  int val;

  if ((val = fcntl(sockFd_, F_GETFL, 0)) < 0) {
    return false;
  }
  val |= O_NONBLOCK;
  if (fcntl(sockFd_, F_SETFL, val) < 0) {
    return false;
  }

  return true;
}

bool TCPSocket::enableReuseaddr(void) {
  int val = 1;
  if (setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR, (const void *)&val,
                 sizeof(val)) < 0) {
    return false;
  }

  return true;
}

bool TCPSocket::setKeepAlive(void) {
  int val = 1;
  if (setsockopt(sockFd_, SOL_SOCKET, SO_KEEPALIVE, (const void *)&val,
                 sizeof(val)) < 0) {
    return false;
  }
  return true;
}

} // namespace net