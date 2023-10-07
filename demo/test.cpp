#include <iostream>
#include <string.h>
#include <vector>

class Buffer {
public:
  explicit Buffer(const char *msg, int len)
      : buffer_(msg, msg + len), readerIndex_(0) {}

  const char *peek() const { return buffer_.data() + readerIndex_; }

  void hasReaded(size_t len) { readerIndex_ + len; }

  size_t getLen() const { return buffer_.size(); }

private:
  size_t ReadableBytes() { return buffer_.size() - readerIndex_; }

private:
  std::vector<char> buffer_;
  size_t readerIndex_;
};

int main() {
  char str[] = "hello, world!";
  std::cout << strlen(str) << std::endl;
  Buffer buffer(str, strlen(str));

  auto fun = [buff = std::move(buffer)]() {
    std::cout << buff.getLen() << std::endl;
    std::cout << buff.peek()[0] << std::endl;
  };
  std::cout << buffer.getLen() << std::endl;
  fun();
  return 0;
}