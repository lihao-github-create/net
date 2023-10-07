#ifndef __NET_REQ_H__
#define __NET_REQ_H__
#include <memory>
#include <stdint.h>
#include <sys/types.h>
namespace net {

struct MsgHeader {
  uint32_t cmd;        // 消息类型
  uint32_t length;     //数据消息长度
  uint32_t reserve[3]; // reserve

  MsgHeader() : cmd(0), length(0) {}

  ~MsgHeader() {}
} __attribute__((packed));

struct NetReq {
public:
  NetReq() : msgHeader(), ioBuf(nullptr) {}
  ~NetReq() {}

public:
  MsgHeader msgHeader;
  std::shared_ptr<char> ioBuf;
};
} // namespace net
#endif