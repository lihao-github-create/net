#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__
#include <functional>
#include <memory>

namespace net {
// All client visible callbacks go here.

class TcpConnection;
class NetReq;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;
typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr &, size_t)>
    HighWaterMarkCallback;

// the data has been read to (buf, len)
typedef std::function<void(const TcpConnectionPtr &, const NetReq &)>
    MessageCallback;

} // namespace net
#endif