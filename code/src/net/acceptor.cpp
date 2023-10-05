#include "net/acceptor.h"
#include <memory>
namespace net {
class Acceptor::Impl {
public:
  Impl();
  ~Impl();

private:
  std::shared_ptr<EventLoop> ownerLoop_;

  Socket acceptSocket_;
  Channel acceptChannel_;

  NewConnectionCallback newConnectionCallback_;
};
} // namespace net