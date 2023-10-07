#include "net/tcp_connection.h"
#include "net/buffer.h"
#include "net/channel.h"
#include "net/net_req.h"
#include "net/tcp_socket.h"
#include <glog/logging.h>

#include <limits.h>
#include <list>
#include <sys/uio.h>

using namespace base;

namespace net {

enum class StateE {
  kDisconnected,
  kConnected,
  kDisconnecting // 过渡状态，正在关闭或已经关闭写端，不允许写
};

class TcpConnection::Impl {
public:
  Impl(TcpConnectionPtr con, EventLoop *loop, const std::string &name,
       int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr);
  ~Impl();
  // send
  void send(const char *message, int len);

  // reading or not
  void startRead();
  void stopRead();

  // shutdown connection
  void shutdownWrite();

  // callback
  void setConnectionCallback(const ConnectionCallback &cb);
  void setMessageCallback(const MessageCallback &cb);
  void setWriteCompleteCallback(const WriteCompleteCallback &cb);

private:
  void sendInLoop(Buffer buffer);
  void shutdownWriteInLoop();
  void handleRead();
  int readHeader();
  int readContent();
  void handleWrite();
  int doWrite();
  void handleClose();
  void handleError();

private:
  TcpConnectionPtr ownCon_;
  EventLoop *ownerLoop_; // useless?
  const std::string name_;
  TCPSocket socket_; // useless ?
  Channel channel_;
  const InetAddress localAddr_;
  const InetAddress peerAddr_;
  StateE state_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  NetReq netReq_; // 用作接收 buffer
  uint32_t readOffset_;
  uint32_t headerSize_;
  uint32_t contentLength_;
  bool readHeader_;
  bool newPackage_;                 // 是否将接收新的数据包
  std::list<Buffer> outputBuffers_; // 用作发送 buffer list
};

TcpConnection::Impl::Impl(TcpConnectionPtr con, EventLoop *loop,
                          const std::string &name, int sockfd,
                          const InetAddress &localAddr,
                          const InetAddress &peerAddr)
    : ownCon_(con), ownerLoop_(loop), name_(name), socket_(sockfd),
      channel_(ownerLoop_, sockfd), localAddr_(localAddr), peerAddr_(peerAddr),
      state_(StateE::kConnected), readOffset_(0), contentLength_(0),
      newPackage_(true), readHeader_(true) {
  // 设置相应的callback
  channel_.setReadCallback([&]() { handleRead(); });
  channel_.setWriteCallback([&]() { handleWrite(); });
  channel_.setCloseCallback([&]() { handleClose(); });
  channel_.setErrorCallback([&]() { handleError(); });
  // 维持一个已建立的TCP连接处于活动状态
  socket_.setKeepAlive();
  // 设置可读，并回调用户注册的connectionCallback
  channel_.enableReading();
  connectionCallback_(ownCon_);
}

TcpConnection::Impl::~Impl() {
  // 关闭连接
  state_ = StateE::kDisconnected;
  socket_.close();
  // channel 析构时会自己取消对应的事件
}

// send, maybe async
void TcpConnection::Impl::send(const char *message, int len) {
  Buffer buffer(message, len);
  // 采用移动语义的捕获方式，减少一次拷贝
  ownerLoop_->runInLoop(
      [&, buff = std::move(buffer)]() { sendInLoop(std::move(buff)); });
}

void TcpConnection::Impl::sendInLoop(Buffer buffer) {
  ownerLoop_->assertInLoopThread();
  // add output buffer
  outputBuffers_.emplace_back(buffer);
  // 注册可写事件
  channel_.enableWriting();
}

// reading or not
void TcpConnection::Impl::startRead() { channel_.enableReading(); }

void TcpConnection::Impl::stopRead() { channel_.disableReading(); }

// shutdown connection
void TcpConnection::Impl::shutdownWrite() {
  state_ = StateE::kDisconnecting;
  ownerLoop_->runInLoop([&]() { shutdownWriteInLoop(); });
}

void TcpConnection::Impl::shutdownWriteInLoop() {
  ownerLoop_->assertInLoopThread();
  socket_.shutdownWrite();
}

// callback
void TcpConnection::Impl::setConnectionCallback(const ConnectionCallback &cb) {
  connectionCallback_ = cb;
}
void TcpConnection::Impl::setMessageCallback(const MessageCallback &cb) {
  messageCallback_ = cb;
}
void TcpConnection::Impl::setWriteCompleteCallback(
    const WriteCompleteCallback &cb) {
  writeCompleteCallback_ = cb;
}

void TcpConnection::Impl::handleRead() {
  if (newPackage_) {
    // 开始接收新的数据包
    newPackage_ = false;
    readHeader_ = true;

    readOffset_ = 0;
    contentLength_ = 0;

    headerSize_ = sizeof(MsgHeader);
    netReq_.ioBuf.reset(); // 设置为 nullptr
    memset(&(netReq_.msgHeader), 0, headerSize_);
  }

  int rt = 0;
  // 读头
  if (readHeader_) {
    rt = readHeader();
    if (0 == rt) {
      handleClose();
    } else if (rt < 0) {
      handleError();
    } else {
      // do nothing
    }
  }
  // 读负载
  rt = readContent();
  if (0 == rt) {
    handleClose();
  } else if (rt < 0) {
    handleError();
  } else {
    // do nothing
  }
}

int TcpConnection::Impl::readHeader() {
  int rt = 0;
  rt = socket_.read(
      static_cast<char *>(static_cast<void *>(&(netReq_.msgHeader))) +
          readOffset_,
      headerSize_ - readOffset_);
  if (0 == rt) {
    handleClose();
  } else if (rt < 0) {
    handleError();
  } else {
    readOffset_ += rt;
    if (readOffset_ == headerSize_) {
      // 读头结束
      readOffset_ = 0;
      readHeader_ = false;

      contentLength_ = netReq_.msgHeader.length;
      if (0 == contentLength_) {
        // 无负载
        newPackage_ = true;
        if (messageCallback_) {
          messageCallback_(ownCon_, netReq_);
        } else {
          LOG(WARNING) << "MessageCallback is not set";
        }
      } else if (contentLength_ > 0) {
        netReq_.ioBuf = std::make_shared<char>(new char[contentLength_],
                                               [](char *p) { delete[] p; });
        memset(netReq_.ioBuf.get(), 0, contentLength_);
      } else {
        LOG(WARNING) << "MsgHeader.length is set to less than 0";
      }
    }
  }
  return rt;
}

int TcpConnection::Impl::readContent() {
  int rt = 0;
  rt = socket_.read(static_cast<char *>(netReq_.ioBuf.get()) + readOffset_,
                    contentLength_ - readOffset_);
  if (0 == rt) {
    handleClose();
    netReq_.ioBuf.reset(); // 释放内存
  } else if (rt < 0) {
    handleError();
  } else {
    readOffset_ += rt;
    if (readOffset_ == contentLength_) {
      // 读负载结束
      readOffset_ = 0;
      newPackage_ = true;

      if (messageCallback_) {
        messageCallback_(ownCon_, netReq_);
      } else {
        LOG(WARNING) << "MessageCallback is not set";
      }
      netReq_.ioBuf.reset(); // 内存交由应用程序去管理
    }
  }
  return rt;
}

void TcpConnection::Impl::handleWrite() {
  ownerLoop_->assertInLoopThread();
  if (!channel_.isWriting()) {
    return;
  }
  // write
  int rt = doWrite();
  if (rt < 0) {
    LOG(ERROR) << "handleWrite error";
  }
  // 对于已经发生完毕的，回调
  for (auto it = outputBuffers_.begin(); it != outputBuffers_.end();) {
    if (0 == it->ReadableBytes()) {
      outputBuffers_.erase(it++);
      writeCompleteCallback_(ownCon_);
    } else {
      break;
    }
  }
}

int TcpConnection::Impl::doWrite() {
  // 构造struct iovec数组
  int iovcnt = outputBuffers_.size();
  if (iovcnt > IOV_MAX) {
    iovcnt = IOV_MAX;
  }
  struct iovec *pIovec = new struct iovec[iovcnt];
  memset(pIovec, 0, sizeof(struct iovec) * iovcnt);

  int i = 0;
  for (auto it = outputBuffers_.begin(); it != outputBuffers_.end();
       ++it, ++i) {
    if (i < iovcnt) {
      pIovec[i].iov_base = static_cast<void *>(const_cast<char *>(it->peek()));
      pIovec[i].iov_len = it->ReadableBytes();
    } else {
      break;
    }
  }

  // 调用 writev 发送数据
  int rt = socket_.writev(pIovec, iovcnt);

  delete[] pIovec;
  pIovec = NULL;

  // error ocurred
  if (rt < 0) {
    LOG(ERROR) << "doWrite writev error: " << strerror(errno);
    return rt;
  }

  // 标记已发送的数据块
  uint32_t writen = (uint32_t)rt;
  for (auto it = outputBuffers_.begin(); it != outputBuffers_.end(); ++it) {
    if (writen > it->ReadableBytes()) {
      writen -= it->ReadableBytes();
      it->hasReaded(it->ReadableBytes());
    } else {
      it->hasReaded(writen);
      break;
    }
  }

  return rt;
}

void TcpConnection::Impl::handleClose() {
  ownerLoop_->assertInLoopThread();
  state_ = StateE::kDisconnected;
  channel_.disableAll(); // 如果还有数据未发送，怎么办？
}

void TcpConnection::Impl::handleError() {
  LOG(ERROR) << "hanle error: " << strerror(errno);
}

/***********************************TcpConnection*************************************/

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name,
                             int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : impl_(std::make_unique<Impl>(this, loop, name, sockfd, localAddr,
                                   peerAddr)) {}
TcpConnection::~TcpConnection() {}
// send
void TcpConnection::send(const char *message, int len) {
  impl_->send(message, len);
}

// reading or not
void TcpConnection::startRead() { impl_->startRead(); }
void TcpConnection::stopRead() { impl_->stopRead(); }

// shutdown connection
void TcpConnection::shutdownWrite() { impl_->shutdownWrite(); }

// callback
void TcpConnection::setConnectionCallback(const ConnectionCallback &cb) {
  impl_->setConnectionCallback(cb);
}

void TcpConnection::setMessageCallback(const MessageCallback &cb) {
  impl_->setMessageCallback(cb);
}

void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback &cb) {
  impl_->setWriteCompleteCallback(cb);
}

} // namespace net