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

#include "noncopyable.h"
#include "string_piece.h"
#include <cstddef>
#include <string>

using std::string;

namespace base {

constexpr int kSmallBuffer = 4000;
constexpr int kLargeBuffer = 4000 * 1000;
/**
 * fixed buffer: 以'\0'作为结尾
 * */
template <size_t SIZE> class FixedBuffer {
  NOCOPYABLE_DECLARE(FixedBuffer);

public:
  FixedBuffer() : cur_(data_) {}
  ~FixedBuffer() {}

  // append
  void append(const char *msg, size_t len) {
    if (avail() > len) {
      memcpy(cur_, msg, len);
      cur_ += len;
    }
  }

  // data
  const char *data() const { return data_; }
  size_t length() const { return cur_ - data_; }
  char *current() const { return cur_; }

  // write to data_ directly
  char *current() { return cur_; }
  int avail() const { return static_cast<int>(end() - cur_); }
  void add(size_t len) { cur_ += len; }

  // reset
  void reset() { cur_ = data_; }
  void bzero() { memset(data_, 0, SIZE); }

  // to string
  string toString() const { return string(data_, length()); }
  // StringPiece toStringPiece() const { return StringPiece(data_, length()); }

private:
  const char *end() const { return data_ + SIZE; }

private:
  char data_[SIZE];
  char *cur_;
};
} // namespace base
#endif