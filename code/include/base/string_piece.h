/* =====================================================================================
 *
 *       Filename:  string_piece.h
 *
 *    Description:  字符串切片
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

#ifndef __STRING_PIECE_H__
#define __STRING_PIECE_H__

#include <cstring>
#include <string>
using std::string;

namespace base {
class StringPiece {
private:
  const char *ptr_;
  size_t length_;

public:
  // constructor
  StringPiece() : ptr_(nullptr), length_(0) {}
  StringPiece(const char *str) : ptr_(str), length_(strlen(ptr_)) {}
  StringPiece(const unsigned char *str)
      : ptr_(reinterpret_cast<const char *>(str)), length_(strlen(ptr_)) {}
  StringPiece(const string &str) : ptr_(str.data()), length_(str.size()) {}
  StringPiece(const char *offset, int len) : ptr_(offset), length_(len) {}

  // access
  const char *data() const { return ptr_; }
  int size() const { return length_; }
  bool empty() const { return length_ == 0; }
  const char *begin() const { return ptr_; }
  const char *end() const { return ptr_ + length_; }
  char operator[](int i) const { return ptr_[i]; }

  // reset
  void clear() {
    ptr_ = nullptr;
    length_ = 0;
  }
  void set(const char *buffer, int len) {
    ptr_ = buffer;
    length_ = len;
  }
  void set(const char *str) {
    ptr_ = str;
    length_ = strlen(str);
  }
  void set(const void *buffer, int len) {
    ptr_ = reinterpret_cast<const char *>(buffer);
    length_ = len;
  }

  // remove
  void remove_prefix(int n) {
    ptr_ += n;
    length_ -= n;
  }
  void remove_suffix(int n) { length_ -= n; }

  // compare
  bool operator==(const StringPiece &x) const {
    return ((length_ == x.length_) && (memcmp(ptr_, x.ptr_, length_) == 0));
  }
  bool operator!=(const StringPiece &x) const { return !(*this == x); }

  int compare(const StringPiece &x) const {
    int r = memcmp(ptr_, x.ptr_, length_ < x.length_ ? length_ : x.length_);
    if (r == 0) {
      return length_ > x.length_ ? +1 : -1;
    }
    return r;
  }

  // to string
  string as_string() const { return string(data(), size()); }
  void CopyToString(string *target) const { target->assign(ptr_, length_); }

  // Does "this" start with "x"
  bool starts_with(const StringPiece &x) const {
    return ((length_ >= x.length_) && (memcmp(ptr_, x.ptr_, x.length_) == 0));
  }
};
} // namespace base
#endif