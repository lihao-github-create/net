# 关于muduo网络库的分析

## 关键类分析

### EventLoop Class

EventLoop从功能上看，和 Reactor 模型中的 Inititation Dispatcher。它的主要功能是运行事件循环 `EventLoop::loop()`。EventLoop 所在的线程也被称为 IO 线程。

muduo 采用的是 `one loop per thread` 的设计方案。为了保证这一点，它采用了线程局部存储来指向当前线程对应的 event loop 对象。如果当前线程已经创建了 eventloop, 则直接输出报错日志，然后退出。除此之外，EventLoop还提供`getEventLoopOfCurrentThread()`静态方法，返回当前线程对应的eventloop。具体实现如下：

```cpp
__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop()
  :threadId_(CurrentThread::tid()),
{
  LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread)
  {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  }
  else
  {
    t_loopInThisThread = this;
  }
  ...
}
// static function
EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
  return t_loopInThisThread;
}
```

### Channel Class

从功能上看，Channel Class 对应于 Reactor 中的 Event Handler。

### Poller Class

从功能看，Poller Class 对应于 Reactor 中的 Synchronous Event Demultiplexer。

### TimQueue Class

所有的 timer 复用一个 timefd。后续可以改为 one timer per timefd。这样就不用遍历所有的 timers 找出 expired timers。

### Acceptor Class

负责接收新 TCP 连接，并通过回调通知使用者。
