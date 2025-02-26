#include <algorithm>
#include <iterator>
#include <memory>
#include <type_traits>
#include <iostream>

namespace internal {
template <typename T>
inline constexpr size_t kBlockSize = sizeof(T) < 256 ? 4096 / sizeof(T) : 16;

template <typename T, typename Allocator = std::allocator<T>>
class SplitBuffer {
 public:
  using allocator_type = Allocator;
  using alloc_traits = std::allocator_traits<Allocator>;
  using reference = T&;
  using const_reference = const T&;
  using pointer = typename alloc_traits::pointer;
  using const_pointer = typename alloc_traits::const_pointer;

  using iterator = pointer;
  using const_iterator = const_pointer;

  SplitBuffer() = default;
  explicit SplitBuffer(size_t capacity, size_t start,
                       const Allocator& allocator = Allocator());
  SplitBuffer(size_t capacity, const Allocator& allocator = Allocator());
  SplitBuffer(const SplitBuffer&) = delete;
  SplitBuffer& operator=(const SplitBuffer&) = delete;

  SplitBuffer(SplitBuffer&& other) noexcept;
  SplitBuffer& operator=(SplitBuffer&& other) noexcept;

  ~SplitBuffer() { clear(); }

  [[nodiscard]] bool empty() const noexcept { return first_ == last_; }
  [[nodiscard]] size_t size() const noexcept {
    return static_cast<size_t>(last_ - first_);
  }
  [[nodiscard]] size_t capacity() const noexcept {
    return static_cast<size_t>(end_ - begin_);
  }

  allocator_type get_allocator() const { return alloc_; }

  template <class... Args>
  void emplace_front(Args&&... args);
  template <class... Args>
  void emplace_back(Args&&... args);

  void pop_front() {
    alloc_traits::destroy(alloc_, std::to_address(first_));
    ++first_;
  }

  void pop_back() {
    --last_;
    alloc_traits::destroy(alloc_, std::to_address(last_));
  }

  reference front() { return *first_; }
  const_reference front() const { return *first_; }
  reference back() { return *std::prev(last_); }
  const_reference back() const { return *std::prev(last_); }

  size_t back_spare() const { return static_cast<size_t>(end_ - last_); }
  size_t front_spare() const { return static_cast<size_t>(first_ - begin_); }

  iterator begin() { return first_; }
  const_iterator cbegin() const { return first_; }

  iterator end() { return last_; }
  const_iterator cend() const { return last_; }

  template <typename Iterator>
    requires std::input_iterator<Iterator>
  void construct_at_last(Iterator begin, Iterator end);

  reference operator[](size_t index) { return *(first_ + index); }
  const_reference operator[](size_t index) const { return *(first_ + index); }

  void swap(SplitBuffer& other) noexcept;

  void clear();

 private:
  void swap_without_allocator(SplitBuffer<T, Allocator>& other) noexcept;

  void try_slide_at_front();
  void try_slide_at_back();

  void reallocate(size_t capacity, size_t start);

  T* first_ = nullptr;
  T* last_ = nullptr;
  T* begin_ = nullptr;
  T* end_ = nullptr;
  [[no_unique_address]] allocator_type alloc_{};
};

template <typename T, typename Allocator>
SplitBuffer<T, Allocator>::SplitBuffer(size_t capacity, size_t start,
                                       const Allocator& allocator)
    : alloc_(allocator) {
  if (capacity == 0) {
    return;
  }
  begin_ = alloc_traits::allocate(alloc_, capacity);

  last_ = first_ = begin_ + start;
  end_ = begin_ + capacity;
}

template <typename T, typename Allocator>
SplitBuffer<T, Allocator>::SplitBuffer(size_t capacity,
                                       const Allocator& allocator)
    : SplitBuffer(capacity, 0, allocator) {}

template <typename T, typename Allocator>
SplitBuffer<T, Allocator>::SplitBuffer(SplitBuffer&& other) noexcept
    : first_(std::move(other.first_)),
      begin_(std::move(other.begin_)),
      last_(std::move(other.last_)),
      end_(std::move(other.end_)),
      alloc_(std::move(other.alloc_)) {
  other.first_ = nullptr;
  other.begin_ = nullptr;
  other.last_ = nullptr;
  other.end_ = nullptr;
}

template <typename T, typename Allocator>
SplitBuffer<T, Allocator>& SplitBuffer<T, Allocator>::operator=(
    SplitBuffer&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  if (alloc_traits::propagate_on_container_move_assignment::value) {
    alloc_ = std::move(other.alloc_);
  }
  swap_without_allocator(other);

  return *this;
}

template <typename T, typename Allocator>
template <typename... Args>
void SplitBuffer<T, Allocator>::emplace_front(Args&&... args) {
  try_slide_at_front();
  alloc_traits::construct(alloc_, std::to_address(first_ - 1),
                          std::forward<Args>(args)...);
  --first_;
}

template <typename T, typename Allocator>
template <typename... Args>
void SplitBuffer<T, Allocator>::emplace_back(Args&&... args) {
  try_slide_at_back();
  alloc_traits::construct(alloc_, std::to_address(last_),
                          std::forward<Args>(args)...);
  ++last_;
}

template <typename T, typename Allocator>
void SplitBuffer<T, Allocator>::swap(SplitBuffer& other) noexcept {
  swap_without_allocator(other);
  if (alloc_traits::propagate_on_container_swap::value) {
    std::swap(alloc_, other.alloc_);
  }
}

template <typename T, typename Allocator>
void SplitBuffer<T, Allocator>::swap_without_allocator(
    SplitBuffer<T, Allocator>& other) noexcept {
  std::swap(first_, other.first_);
  std::swap(begin_, other.begin_);
  std::swap(last_, other.last_);
  std::swap(end_, other.end_);
}

template <typename T, typename Allocator>
template <typename Iterator>
  requires std::input_iterator<Iterator>
void SplitBuffer<T, Allocator>::construct_at_last(Iterator begin,
                                                  Iterator end) {
  for (auto& current = last_; begin != end; ++begin, ++current) {
    alloc_traits::construct(alloc_, std::to_address(current), *begin);
  }
}

template <typename T, typename Allocator>
void SplitBuffer<T, Allocator>::try_slide_at_front() {
  if (begin_ != first_) {
    return;
  }

  size_t cap = std::max<size_t>(2 * capacity(), 1);
  reallocate(cap, (cap + 3) / 4);
}

template <typename T, typename Allocator>
void SplitBuffer<T, Allocator>::try_slide_at_back() {
  if (last_ != end_) {
    return;
  }

  size_t cap = std::max<size_t>(2 * capacity(), 1);
  reallocate(cap, cap / 4);
}

template <typename T, typename Allocator>
void SplitBuffer<T, Allocator>::reallocate(size_t new_capacity, size_t start) {
  SplitBuffer<T, Allocator> buffer(new_capacity, start, alloc_);
  buffer.construct_at_last(std::make_move_iterator(first_),
                           std::make_move_iterator(last_));
  swap(buffer);
}

template <typename T, typename Allocator>
void SplitBuffer<T, Allocator>::clear() {
  if (first_ == last_) {
    alloc_traits::destroy(alloc_, std::to_address(first_));
    return;
  }
  for (auto current = first_; current != last_; ++current) {
    alloc_traits::destroy(alloc_, std::to_address(current));
  }
  alloc_traits::deallocate(alloc_, begin_, capacity());
}

void ThrowFmtOutOfRange(size_t index, size_t size) {
  throw std::out_of_range(
      "Deque::at: index (which is " + std::to_string(index) +
      ") >= this->size() (which is " + std::to_string(size) + ")");
}
}  // namespace internal

template <typename T, typename Allocator = std::allocator<T>>
class Deque {
 public:
  template <bool IsConst>
  class Iterator;

  using value_type = T;
  using allocator_type = Allocator;
  using alloc_traits = std::allocator_traits<allocator_type>;
  using size_type = typename alloc_traits::size_type;
  using difference_type = typename alloc_traits::difference_type;
  using pointer = typename alloc_traits::pointer;
  using const_pointer = typename alloc_traits::const_pointer;
  using reference = value_type&;
  using const_reference = const value_type&;

  using block = pointer;
  using block_allocator = typename alloc_traits::template rebind_alloc<block>;

  using map = internal::SplitBuffer<block, block_allocator>;
  using map_iterator = typename map::iterator;
  using map_const_iterator = typename map::const_iterator;

  using block_size =
      std::integral_constant<size_t, internal::kBlockSize<block>>;

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  Deque() = default;
  Deque(size_t count, const Allocator& allocator = Allocator());
  explicit Deque(size_t count, const T& value,
                 const Allocator& allocator = Allocator());
  Deque(std::initializer_list<T> init, const Allocator& alloc = Allocator());

  Deque(const Deque& other);
  Deque(Deque&& other) noexcept;
  ~Deque();

  Deque& operator=(const Deque& other);
  Deque& operator=(Deque&& other) noexcept;

  [[nodiscard]] size_t size() const noexcept { return size_; };
  [[nodiscard]] bool empty() const noexcept { return size_ == 0; };
  [[nodiscard]] size_t capacity() const {
    return blocks_.empty() ? 0 : ((blocks_.size() * block_size::value) - 1);
  }

  template <typename... Args>
  void emplace_back(Args&&... args);
  template <typename... Args>
  void emplace_front(Args&&... args);

  void push_back(const T& value);
  void push_back(T&& value);
  void push_front(const T& value);
  void push_front(T&& value);
  void pop_back();
  void pop_front();

  void insert(iterator pos, const T& value);
  void erase(iterator pos);

  const_reference operator[](size_t index) const;
  reference operator[](size_t index);
  const_reference at(size_t index) const;
  reference at(size_t index);

  void swap(Deque& other) noexcept;

  allocator_type get_allocator() { return alloc_; };

  iterator begin() {
    auto iter = blocks_.begin() + front_block_spare();
    size_t offset = get_offset(0);

    return iterator{blocks_.empty() ? nullptr : *iter + offset, iter};
  };

  const_iterator cbegin() const {
    auto iter = blocks_.cbegin() + front_block_spare();
    size_t offset = get_offset(0);

    return const_iterator{blocks_.empty() ? nullptr : *iter + offset, iter};
  };

  reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }

  const_reverse_iterator crbegin() const {
    return std::make_reverse_iterator(cbegin());
  }

  iterator end() {
    auto iter = blocks_.begin() + get_block_index(size());

    return iterator{blocks_.empty() ? nullptr : *iter + get_offset(size()),
                    iter};
  };

  const_iterator cend() const {
    auto iter = blocks_.cbegin() + get_block_index(size());

    return const_iterator{
        blocks_.empty() ? nullptr : *iter + get_offset(size()), iter};
  };

  reverse_iterator rend() { return std::make_reverse_iterator(begin()); }

  const_reverse_iterator crend() const {
    return std::make_reverse_iterator(cbegin());
  }

 private:
  void swap_without_allocator(Deque& other) noexcept;

  [[nodiscard]] size_t back_spare() const {
    return (capacity() - (start_index_ + size_));
  }
  [[nodiscard]] size_t front_spare() const { return start_index_; }
  [[nodiscard]] size_t back_block_spare() const {
    return back_spare() / block_size::value;
  }
  [[nodiscard]] size_t front_block_spare() const {
    return front_spare() / block_size::value;
  }

  [[nodiscard]] size_t get_block_index(size_t index) const {
    return (start_index_ + index) / block_size::value;
  };
  [[nodiscard]] size_t get_offset(size_t index) const {
    return (start_index_ + index) % block_size::value;
  };

  void clear() noexcept;

  void add_front_block();
  void add_back_block();

  [[no_unique_address]] allocator_type alloc_{};
  map blocks_{0, alloc_};
  size_t start_index_ = 0;
  size_t size_ = 0;
};

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const Allocator& allocator)
    : alloc_(allocator),
      blocks_(0, block_allocator(alloc_)),
      start_index_(0),
      size_(0) {
  try {
    for (size_t counter = 0; counter < count; ++counter) {
      emplace_back();
    }
  } catch (...) {
    clear();
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const T& value,
                           const Allocator& allocator)
    : alloc_(allocator),
      blocks_(0, block_allocator(alloc_)),
      start_index_(0),
      size_(0) {
  try {
    for (size_t counter = 0; counter < count; ++counter) {
      emplace_back(value);
    }
  } catch (...) {
    clear();
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(std::initializer_list<T> init,
                           const Allocator& allocator)
    : alloc_(allocator) {
  for (auto iter = init.begin(); iter != init.end(); ++iter) {
    emplace_back(std::forward<decltype(*iter)>(*iter));
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Deque& other)
    : start_index_(other.start_index_),
      size_(other.size_),
      blocks_(
          other.blocks_.capacity(), other.blocks_.front_spare(),
          alloc_traits::select_on_container_copy_construction(other.alloc_)),
      alloc_(
          alloc_traits::select_on_container_copy_construction(other.alloc_)) {
  size_t created = 0;
  try {
    for (size_t count = 0; count < other.blocks_.size(); ++count) {
      blocks_.emplace_back(alloc_traits::allocate(alloc_, block_size::value));
    }

    auto iter = begin();
    for (auto other_iter = other.cbegin(); other_iter != other.cend();
         ++other_iter, ++iter) {
      alloc_traits::construct(alloc_, std::addressof(*iter), *other_iter);
      ++created;
    }
  } catch (...) {
    size_ = created;
    clear();
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(Deque&& other) noexcept
    : start_index_(other.start_index_),
      size_(other.size_),
      blocks_(std::move(other.blocks_)),
      alloc_(std::move(other.alloc_)) {
  other.start_index_ = 0;
  other.size_ = 0;
  other.blocks_.clear();
}

template <typename T, typename Allocator>
Deque<T, Allocator>::~Deque() {
  clear();
}

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(const Deque& other) {
  if (this == &other) {
    return *this;
  }

  if (alloc_traits::propagate_on_container_copy_assignment::value) {
    alloc_ = other.alloc_;
  }
  Deque<T, Allocator> copy{other};
  swap_without_allocator(copy);

  return *this;
}

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(Deque&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  if (alloc_traits::propagate_on_container_move_assignment::value) {
    alloc_ = std::move(other.alloc_);
  }
  swap_without_allocator(other);

  return *this;
}

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::operator[](size_t index) const {
  size_t block = get_block_index(index);
  size_t offset = get_offset(index);

  return blocks_[block][offset];
}

template <typename T, typename Allocator>
T& Deque<T, Allocator>::operator[](size_t index) {
  size_t block = get_block_index(index);
  size_t offset = get_offset(index);

  return blocks_[block][offset];
}

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::at(size_t index) const {
  if (index >= size()) {
    internal::ThrowFmtOutOfRange(index, size());
  }
  return operator[](index);
}

template <typename T, typename Allocator>
T& Deque<T, Allocator>::at(size_t index) {
  if (index >= size()) {
    internal::ThrowFmtOutOfRange(index, size());
  }
  return operator[](index);
}

template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_back(Args&&... args) {
  if (back_spare() == 0) {
    add_back_block();
  }

  alloc_traits::construct(alloc_, std::addressof(*end()),
                          std::forward<Args>(args)...);
  ++size_;
}

template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_front(Args&&... args) {
  if (front_spare() == 0) {
    add_front_block();
  }

  alloc_traits::construct(alloc_, std::addressof(*std::prev(begin())),
                          std::forward<Args>(args)...);
  --start_index_;
  ++size_;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(const T& value) {
  emplace_back(value);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(T&& value) {
  emplace_back(std::move(value));
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(const T& value) {
  emplace_front(value);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(T&& value) {
  emplace_front(std::move(value));
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::insert(Deque::iterator pos, const T& value) {
  if (pos == end()) {
    emplace_back(value);
    return;
  }

  if (back_spare() == 0) {
    add_back_block();
  }
  std::move_backward(rbegin(), std::make_reverse_iterator(pos),
                     std::next(end()));

  *pos = value;
  ++size_;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::erase(Deque::iterator pos) {
  for (auto iter = std::next(pos); iter != end(); ++iter) {
    std::swap(*iter, *std::prev(iter));
  }
  pop_back();
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_back() {
  --size_;
  alloc_traits::destroy(alloc_, std::addressof(*end()));
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_front() {
  alloc_traits::destroy(alloc_, std::addressof(*begin()));
  ++start_index_;
  --size_;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::swap(Deque& other) noexcept {
  swap_without_allocator(other);
  if (alloc_traits::propagate_on_container_swap::value) {
    std::swap(alloc_, other.alloc_);
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::clear() noexcept {
  for (auto iter = begin(), end_iter = end(); iter != end_iter; ++iter) {
    alloc_traits::destroy(alloc_, std::addressof(*iter));
  }
  for (auto iter = blocks_.begin(); iter != blocks_.end(); ++iter) {
    alloc_traits::deallocate(alloc_, std::to_address(*iter), block_size::value);
  }
  size_ = 0;
  start_index_ = 0;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::swap_without_allocator(Deque& other) noexcept {
  std::swap(start_index_, other.start_index_);
  std::swap(size_, other.size_);
  std::swap(blocks_, other.blocks_);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::add_front_block() {
  if (back_spare() >= block_size::value) {
    auto block = blocks_.back();
    blocks_.pop_back();
    blocks_.emplace_front(block);
    start_index_ += block_size::value;
    return;
  }
  if (blocks_.size() < blocks_.capacity()) {
    if (blocks_.front_spare() != 0) {
      blocks_.emplace_front(alloc_traits::allocate(alloc_, block_size::value));
      start_index_ += block_size::value;
      return;
    }

    blocks_.emplace_back(alloc_traits::allocate(alloc_, block_size::value));

    auto back = blocks_.back();
    blocks_.pop_back();
    blocks_.emplace_front(back);
    start_index_ += block_size::value;
    return;
  }

  map buffer(std::max<size_t>(2 * blocks_.capacity(), 1),
             blocks_.get_allocator());

  block new_block = alloc_traits::allocate(alloc_, block_size::value);
  try {
    buffer.emplace_back(new_block);
  } catch (...) {
    alloc_traits::deallocate(alloc_, new_block, block_size::value);
    throw;
  }

  for (auto iter = blocks_.begin(); iter != blocks_.end(); ++iter) {
    buffer.emplace_back(*iter);
  }

  blocks_ = std::move(buffer);
  start_index_ += block_size::value;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::add_back_block() {
  if (front_spare() >= block_size::value) {
    auto block = blocks_.front();
    blocks_.pop_front();
    blocks_.emplace_back(block);
    start_index_ -= block_size::value;
    return;
  }
  if (blocks_.size() < blocks_.capacity()) {
    if (blocks_.back_spare() != 0) {
      blocks_.emplace_back(alloc_traits::allocate(alloc_, block_size::value));
      return;
    }
    blocks_.emplace_front(alloc_traits::allocate(alloc_, block_size::value));

    auto front = blocks_.front();
    blocks_.pop_front();
    blocks_.emplace_back(front);
    return;
  }

  map buffer(std::max<size_t>(2 * blocks_.capacity(), 1), blocks_.size(),
             blocks_.get_allocator());

  block new_block = alloc_traits::allocate(alloc_, block_size::value);
  try {
    buffer.emplace_back(new_block);
  } catch (...) {
    alloc_traits::deallocate(alloc_, new_block, block_size::value);
    throw;
  }

  for (auto iter = blocks_.end(); iter != blocks_.begin(); --iter) {
    buffer.emplace_front(*std::prev(iter));
  }

  blocks_ = std::move(buffer);
}

template <typename T, typename Allocator>
template <bool IsConst>
class Deque<T, Allocator>::Iterator {
 public:
  using value_type =
      std::conditional_t<IsConst, const Deque::value_type, Deque::value_type>;
  using map_iterator = std::conditional_t<IsConst, Deque::map_const_iterator,
                                          Deque::map_iterator>;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = std::ptrdiff_t;

  using iterator_category = std::random_access_iterator_tag;

  Iterator() = delete;
  explicit Iterator(pointer current, map_iterator iterator)
      : current_(current), map_iterator_(iterator) {}

  Iterator(const Iterator& other)
      : current_(other.current_), map_iterator_(other.map_iterator_) {}

  Iterator& operator=(const Iterator& iterator) = default;

  reference operator*() const { return *current_; }

  pointer operator->() const { return current_; }

  Iterator& operator++() {
    ++current_;
    if (get_offset() == block_size::value) {
      ++map_iterator_;
      current_ = *map_iterator_;
    }
    return *this;
  }

  Iterator operator++(int) {
    Iterator copy = *this;
    ++(*this);
    return copy;
  }

  Iterator& operator--() {
    if (current_ == *map_iterator_) {
      --map_iterator_;
      current_ = *map_iterator_ + block_size::value;
    }
    --current_;
    return *this;
  }

  Iterator operator--(int) {
    Iterator copy = *this;
    --(*this);
    return copy;
  }

  Iterator& operator+=(difference_type len) {
    if (len == 0) {
      return *this;
    }
    len += get_offset();
    if (len > 0) {
      map_iterator_ += (len / block_size::value);
      current_ = *map_iterator_ + len % block_size::value;
    } else {
      difference_type step = block_size::value - 1 - len;
      map_iterator_ -= step / block_size::value;
      current_ =
          *map_iterator_ + (block_size::value - 1 - step % block_size::value);
    }

    return *this;
  }

  Iterator& operator-=(difference_type len) {
    *this += -len;
    return *this;
  }

  Iterator operator+(difference_type len) const {
    Iterator copy = *this;
    copy += len;
    return copy;
  }

  Iterator operator-(difference_type len) const {
    Iterator copy = *this;
    copy -= len;
    return copy;
  }

  difference_type operator-(const Iterator& other) const {
    if (*this == other) {
      return 0;
    }
    return ((map_iterator_ - other.map_iterator_) * block_size::value) +
           get_offset() - other.get_offset();
  }

  bool operator==(const Iterator& other) const {
    return current_ == other.current_;
  };
  auto operator<=>(const Iterator& other) const = default;

  operator Iterator<true>() const {
    return Iterator<true>{current_, map_iterator_};
  }

 private:
  [[nodiscard]] difference_type get_offset() const {
    return current_ - *map_iterator_;
  }

  pointer current_;
  map_iterator map_iterator_;
};
