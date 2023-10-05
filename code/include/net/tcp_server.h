#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "base/noncopyable.h"
#include "net/callbacks.h"
#include "net/inet_address.h"
#include <string>

using namespace base;
namespace net {
class TcpServer {
  NOCOPYABLE_DECLARE(TcpServer)
public:
  TcpServer(const InetAddress &listenAddr, const std::string &name,
            int numsThreads = 1);
  ~TcpServer();

  /**
   * @brief Set the number of threads for handling input.
   * @param numThreads
   *  - 0 means all I/O in loop's thread, no thread will created.
   *     this is the default value.
   *  - 1 means all I/O in another thread.
   *  - N means a thread pool with N threads, new connections
   *     are assigned on a round-robin basis.
   */
  void setThreadNum(int numThreads);
  // start tcp server, listening
  void start();

  // set callback
  void setConnectionCallback(const ConnectionCallback &cb);
  void setMessageCallback(const MessageCallback &cb);
  void setWriteCompleteCallback(const WriteCompleteCallback &cb);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace net

#endif