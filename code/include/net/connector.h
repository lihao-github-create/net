#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "base/noncopyable.h"
#include <memory>

class Connector {
  NOCOPYABLE_DECLARE(Connector)
public:
  Connector();
  ~Connector();

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

#endif