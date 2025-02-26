#include <cstddef>
#include <vector>

class RingBuffer {
 public:
  explicit RingBuffer(size_t capacity) { values_.resize(capacity); }

  size_t Size() const { return size_; }

  bool Empty() const { return Size() == 0; }

  bool TryPush(int element) {
    if ((!Empty()) && (begin_ == end_)) {
      return false;
    }

    values_[end_] = element;
    end_ = GetFixedIndex(end_ + 1);
    size_ += 1;

    return true;
  }

  bool TryPop(int* element) {
    if (Empty()) {
      return false;
    }

    *element = values_[begin_];
    begin_ = GetFixedIndex(begin_ + 1);
    size_ -= 1;

    return true;
  }

 private:
  std::vector<int> values_;
  size_t begin_ = 0;
  size_t end_ = 0;
  size_t size_ = 0;

  size_t GetFixedIndex(size_t index) const { return index % values_.size(); }
};
