#include <functional>
#include <iterator>
#include <memory>
#include <utility>

namespace internal {
template <typename Functor>
class Defer {
 public:
  Defer(const Functor& functor) : functor_(std::move(functor)) {}
  ~Defer() {
    if (released_) {
      return;
    }
    functor_();
  };

  void release() { released_ = true; };

 private:
  Functor functor_;
  bool released_ = false;
};

template <typename Allocator>
class AllocationGuard {
 public:
  using allocator_type = Allocator;
  using alloc_traits = std::allocator_traits<allocator_type>;
  using pointer = typename alloc_traits::pointer;
  using size_type = typename alloc_traits::size_type;

  explicit AllocationGuard(Allocator& allocator, size_type size);
  ~AllocationGuard() { destroy(); };

  pointer release();
  [[nodiscard]] pointer get() { return pointer_; };

 private:
  void destroy();

  allocator_type& alloc_;
  size_type size_;
  pointer pointer_;
};

template <typename Allocator>
AllocationGuard<Allocator>::AllocationGuard(Allocator& allocator,
                                            size_type size)
    : alloc_(allocator),
      size_(size),
      pointer_(alloc_traits::allocate(alloc_, size_)) {}

template <typename Allocator>
AllocationGuard<Allocator>::pointer AllocationGuard<Allocator>::release() {
  return std::exchange(pointer_, nullptr);
}

template <typename Allocator>
void AllocationGuard<Allocator>::destroy() {
  alloc_traits::deallocate(alloc_, pointer_, size_);
  pointer_ = nullptr;
}
}  // namespace internal

template <typename T, typename Allocator = std::allocator<T>>
class List {
 public:
  template <bool IsConst>
  class ListIterator;

  using value_type = T;
  using allocator_type = Allocator;
  using alloc_traits = std::allocator_traits<allocator_type>;

  using iterator = ListIterator<false>;
  using const_iterator = ListIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  List() = default;
  List(size_t count, const T& value, const Allocator& alloc = Allocator());
  explicit List(size_t count, const Allocator& alloc = Allocator());
  List(const List& other);
  List(std::initializer_list<T> init, const Allocator& alloc = Allocator());

  ~List() { clear(); };

  List& operator=(const List& other);

  [[nodiscard]] size_t size() const { return size_; };
  [[nodiscard]] bool empty() const { return size() == 0; };
  [[nodiscard]] allocator_type get_allocator() { return alloc_; }

  iterator begin() { return iterator{fake_node_.next, 0}; };
  iterator end() { return iterator{&fake_node_, size_}; };
  const_iterator begin() const { return cbegin(); };
  const_iterator end() const { return cend(); };
  reverse_iterator rbegin() { return std::make_reverse_iterator(end()); };
  reverse_iterator rend() { return std::make_reverse_iterator(begin()); };
  const_iterator cbegin() const { return const_iterator{fake_node_.next, 0}; };
  const_iterator cend() const { return const_iterator{&fake_node_, size_}; };

  void push_back(const T& value) { emplace_back(value); };
  void push_front(const T& value) { emplace_front(value); };

  void push_back(T&& value) { emplace_back(std::move(value)); };
  void push_front(T&& value) { emplace_front(std::move(value)); };

  template <typename... Args>
  void emplace_back(Args&&... args);

  template <typename... Args>
  void emplace_front(Args&&... args);

  void pop_back();
  void pop_front();

  void erase(iterator begin, iterator end);

 private:
  struct BaseNode;
  struct Node;

  using node_allocator = typename alloc_traits::template rebind_alloc<Node>;
  using node_alloc_traits = typename alloc_traits::template rebind_traits<Node>;

  void clear() noexcept;

  template <typename Iterator, typename Sentinel>
    requires std::forward_iterator<Iterator> &&
             std::sentinel_for<Sentinel, Iterator>
  void assign_with_sentinel(Iterator iterator, Sentinel sent);

  void copy_assign_alloc(const node_allocator& alloc);

  template <typename... Args>
  Node* create_node(BaseNode* prev, BaseNode* next, Args&&... args);
  void unlink_node(BaseNode* node) noexcept;
  void delete_node(Node* node) noexcept;

  BaseNode fake_node_{};
  size_t size_ = 0;
  [[no_unique_address]] node_allocator alloc_{};
};

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const T& value, const Allocator& alloc)
    : alloc_(alloc) {
  internal::Defer defer{std::bind(&List::clear, this)};
  for (size_t i = 0; i < count; ++i) {
    emplace_back(value);
  }
  defer.release();
}

template <typename T, typename Allocator>
List<T, Allocator>::List(size_t count, const Allocator& alloc) : alloc_(alloc) {
  internal::Defer defer{std::bind(&List::clear, this)};
  for (size_t i = 0; i < count; ++i) {
    emplace_back();
  }
  defer.release();
}

template <typename T, typename Allocator>
List<T, Allocator>::List(std::initializer_list<T> init, const Allocator& alloc)
    : alloc_(alloc) {
  internal::Defer defer{std::bind(&List::clear, this)};
  for (auto iter = init.begin(); iter != init.end(); ++iter) {
    emplace_back(std::forward<decltype(*iter)>(*iter));
  }
  defer.release();
}

template <typename T, typename Allocator>
List<T, Allocator>::List(const List& other)
    : alloc_(node_alloc_traits::select_on_container_copy_construction(
          other.alloc_)) {
  internal::Defer defer{std::bind(&List::clear, this)};
  for (auto iter = other.cbegin(); iter != other.cend(); ++iter) {
    emplace_back(*iter);
  }
  defer.release();
}

template <typename T, typename Allocator>
void List<T, Allocator>::clear() noexcept {
  erase(begin(), end());
}

template <typename T, typename Allocator>
List<T, Allocator>& List<T, Allocator>::operator=(
    const List<T, Allocator>& other) {
  if (this == std::addressof(other)) {
    return *this;
  }

  copy_assign_alloc(other.alloc_);
  assign_with_sentinel(other.cbegin(), other.cend());

  return *this;
}

template <typename T, typename Allocator>
template <typename... Args>
void List<T, Allocator>::emplace_back(Args&&... args) {
  auto node =
      create_node(fake_node_.prev, &fake_node_, std::forward<Args>(args)...);

  fake_node_.prev->next = node;
  fake_node_.prev = node;
  ++size_;
}

template <typename T, typename Allocator>
template <typename... Args>
void List<T, Allocator>::emplace_front(Args&&... args) {
  auto node =
      create_node(&fake_node_, fake_node_.next, std::forward<Args>(args)...);

  fake_node_.next->prev = node;
  fake_node_.next = node;
  ++size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_back() {
  auto back = static_cast<Node*>(fake_node_.prev);
  unlink_node(back);
  delete_node(back);

  --size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::pop_front() {
  auto front = static_cast<Node*>(fake_node_.next);
  unlink_node(front);
  delete_node(front);

  --size_;
}

template <typename T, typename Allocator>
void List<T, Allocator>::erase(List::iterator begin, List::iterator end) {
  if (begin == end) {
    return;
  }

  for (auto iter = begin; iter != end;) {
    auto node = iter.as_node();
    ++iter;
    unlink_node(node);
    delete_node(node);
    --size_;
  }
}

template <typename T, typename Allocator>
void List<T, Allocator>::copy_assign_alloc(const List::node_allocator& alloc) {
  if (!node_alloc_traits::propagate_on_container_copy_assignment::value) {
    return;
  }
  if (alloc_ != alloc) {
    clear();
  }
  alloc_ = alloc;
}

template <typename T, typename Allocator>
template <typename Iterator, typename Sentinel>
  requires std::forward_iterator<Iterator> &&
           std::sentinel_for<Sentinel, Iterator>
void List<T, Allocator>::assign_with_sentinel(Iterator iterator,
                                              Sentinel sentinel) {
  auto current = begin();

  for (; iterator != sentinel && current != end(); ++iterator, ++current) {
    *current = *iterator;
  }

  if (iterator == sentinel) {
    erase(current, end());
    return;
  }
  for (; iterator != sentinel; ++iterator) {
    emplace_back(*iterator);
  }
}

template <typename T, typename Allocator>
template <typename... Args>
List<T, Allocator>::Node* List<T, Allocator>::create_node(
    List<T, Allocator>::BaseNode* prev, List<T, Allocator>::BaseNode* next,
    Args&&... args) {
  internal::AllocationGuard guard(alloc_, 1);

  node_alloc_traits::construct(alloc_, guard.get(), std::forward<Args>(args)...,
                               prev, next);

  return guard.release();
}

template <typename T, typename Allocator>
void List<T, Allocator>::unlink_node(List::BaseNode* node) noexcept {
  node->prev->next = node->next;
  node->next->prev = node->prev;
}

template <typename T, typename Allocator>
void List<T, Allocator>::delete_node(List::Node* node) noexcept {
  node_alloc_traits::destroy(alloc_, node);
  node_alloc_traits::deallocate(alloc_, node, 1);
}

template <typename T, typename Allocator>
struct List<T, Allocator>::BaseNode {
  BaseNode() = default;
  BaseNode(BaseNode* prev, BaseNode* next) : prev(prev), next(next) {}

  BaseNode* prev{this};
  BaseNode* next{this};
};

template <typename T, typename Allocator>
struct List<T, Allocator>::Node : BaseNode {
  Node() : BaseNode() {}
  Node(const T& value, BaseNode* prev, BaseNode* next)
      : BaseNode(prev, next), value(value) {}
  Node(BaseNode* prev, BaseNode* next) : BaseNode(prev, next), value(T()) {}

  T value{};
};

template <typename T, typename Allocator>
template <bool IsConst>
class List<T, Allocator>::ListIterator {
 public:
  using value_type =
      std::conditional_t<IsConst, const List::value_type, List::value_type>;
  using node_base_type =
      std::conditional_t<IsConst, const List::BaseNode, List::BaseNode>;
  using node_type = std::conditional_t<IsConst, const List::Node, List::Node>;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = std::ptrdiff_t;

  using iterator_category = std::bidirectional_iterator_tag;

  ListIterator() = default;
  ListIterator(node_base_type* node, size_t index)
      : node_(node), index_(index) {}

  ListIterator(const ListIterator& other) = default;

  ListIterator& operator=(const ListIterator& iterator) = default;

  reference operator*() const { return as_node()->value; }

  pointer operator->() const { return std::addressof(operator*()); }

  ListIterator& operator++() {
    node_ = node_->next;
    ++index_;
    return *this;
  }

  ListIterator operator++(int) {
    ListIterator copy = *this;
    ++(*this);
    return copy;
  }

  ListIterator& operator--() {
    node_ = node_->prev;
    --index_;
    return *this;
  }

  ListIterator operator--(int) {
    ListIterator copy = *this;
    --(*this);
    return copy;
  }

  difference_type operator-(const ListIterator& other) const {
    return index_ - other.index_;
  }

  bool operator==(const ListIterator& other) const {
    return node_ == other.node_;
  };
  auto operator<=>(const ListIterator& other) const = default;

  operator ListIterator<true>() const {
    return ListIterator<true>{node_, index_};
  }

  node_type* as_node() const { return static_cast<node_type*>(node_); }

 private:
  node_base_type* node_ = nullptr;
  size_t index_ = 0;
};