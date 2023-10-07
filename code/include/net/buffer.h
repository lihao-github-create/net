/* =====================================================================================
 *
 *       Filename:  buffer.h
 *
 *    Description:  字符缓冲区
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:
 *
 *         Author:
 *        Company:
 *
 * =====================================================================================
 */

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "base/noncopyable.h"
#include "base/string_piece.h"
#include <algorithm>
#include <vector>

#include <assert.h>
#include <string.h>

using std::string;

namespace net {

class Buffer {
public:
  explicit Buffer(const char *msg, int len)
      : buffer_(msg, msg + len), readerIndex_(0) {}

  const char *peek() const { return buffer_.data() + readerIndex_; }

  void hasReaded(size_t len) { readerIndex_ + len; }

  size_t ReadableBytes() { return buffer_.size() - readerIndex_; }

private:
  std::vector<char> buffer_;
  size_t readerIndex_;
};

} // namespace net

#endif