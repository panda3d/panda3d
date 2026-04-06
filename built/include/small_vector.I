/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file small_vector.I
 * @author rdb
 * @date 2023-01-25
 */

/**
 *
 */
template<class T, unsigned N>
INLINE small_vector<T, N>::
small_vector(TypeHandle type_handle) {
}

/**
 * Initializer list constructor.
 */
template<class T, unsigned N>
INLINE small_vector<T, N>::
small_vector(std::initializer_list<T> init) :
  _storage(init.size() > N ? (T *)PANDA_MALLOC_ARRAY(sizeof(T) * init.size()) : nullptr),
  _size(init.size()),
  _capacity(std::max((size_type)N, (size_type)init.size())) {

  T *to = this->data();
  for (const T &value : init) {
    new (to++) T(value);
  }
}

/**
 * Copy constructor.
 */
template<class T, unsigned N>
INLINE small_vector<T, N>::
small_vector(const small_vector &copy) :
  _storage(copy._size > N ? (T *)PANDA_MALLOC_ARRAY(sizeof(T) * copy._size) : nullptr),
  _size(copy._size),
  _capacity(std::max((size_type)N, copy._size)) {

  const T *from = copy.data();
  T *to = this->data();
  for (size_type i = 0; i < _size; ++i) {
    new (to + i) T(from[i]);
  }
}

/**
 * Move constructor.
 */
template<class T, unsigned N>
INLINE small_vector<T, N>::
small_vector(small_vector &&other) noexcept :
  _storage(other._storage._large),
  _size(other._size),
  _capacity(other._capacity) {

  if (_size <= N) {
    T *to = _storage._small;
    T *from = other.data();
    for (size_type i = 0; i < _size; ++i) {
      new (to + i) T(std::move(from[i]));
      from[i].~T();
    }
    _capacity = N;
  }
  other._storage._large = nullptr;
  other._size = 0;
  other._capacity = N;
}

/**
 * Destructor.
 */
template<class T, unsigned N>
INLINE small_vector<T, N>::
~small_vector() {
  this->clear();
  if (!this->is_small()) {
    PANDA_FREE_ARRAY(_storage._large);
    _capacity = N;
  }
}

/**
 * Copy assignment operator.
 */
template<class T, unsigned N>
INLINE small_vector<T, N> &small_vector<T, N>::
operator =(const small_vector &copy) {
  this->clear();
  this->reserve(copy._size);

  const T *from = copy.data();
  T *to = this->data();
  for (size_type i = 0; i < copy._size; ++i) {
    new (to + i) T(from[i]);
  }
  _size = copy._size;
  return *this;
}

/**
 * Move assignment operator.
 */
template<class T, unsigned N>
INLINE small_vector<T, N> &small_vector<T, N>::
operator =(small_vector &&from) noexcept {
  if (UNLIKELY(this == &from)) {
    return *this;
  }

  this->clear();
  if (!this->is_small()) {
    PANDA_FREE_ARRAY(_storage._large);
    _capacity = N;
  }

  _storage._large = from._storage._large;
  _size = from._size;
  _capacity = from._capacity;

  if (this->is_small()) {
#if defined(__has_builtin) && __has_builtin(__builtin_assume)
    __builtin_assume(_size <= N);
#endif
    T *to = this->_storage._small;
    for (size_type i = 0; i < _size; ++i) {
      new (to + i) T(std::move(from._storage._small[i]));
      from._storage._small[i].~T();
    }
  }
  from._storage._large = nullptr;
  from._size = 0;
  from._capacity = N;
  return *this;
}

/**
 * Returns true if there are no elements in this vector.
 */
template<class T, unsigned N>
constexpr bool small_vector<T, N>::
empty() const {
  return _size == 0;
}

/**
 * Returns the number of elements stored in the vector.
 */
template<class T, unsigned N>
constexpr typename small_vector<T, N>::size_type small_vector<T, N>::
size() const {
  return _size;
}

/**
 * Returns the total capacity of the vector.
 */
template<class T, unsigned N>
constexpr typename small_vector<T, N>::size_type small_vector<T, N>::
capacity() const {
  return _capacity;
}

/**
 * Returns the maximum size of the vector.
 */
template<class T, unsigned N>
constexpr typename small_vector<T, N>::size_type small_vector<T, N>::
max_size() const {
  return SIZE_MAX;
}

/**
 * Returns the ith element with bounds checking.
 */
template<class T, unsigned N>
INLINE typename small_vector<T, N>::reference small_vector<T, N>::
at(size_type i) {
  assert(i < size());
  return *(this->begin() + i);
}

/**
 * Returns the ith element with bounds checking.
 */
template<class T, unsigned N>
INLINE typename small_vector<T, N>::const_reference small_vector<T, N>::
at(size_type i) const {
  assert(i < size());
  return *(this->begin() + i);
}

/**
 * Returns the ith element without bounds checking.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::reference small_vector<T, N>::
operator [](size_type i) {
  return *(this->begin() + i);
}

/**
 * Returns the ith element without bounds checking.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::const_reference small_vector<T, N>::
operator [](size_type i) const {
  return *(this->begin() + i);
}

/**
 * Returns a reference to the first element.
 */
template<class T, unsigned N>
INLINE typename small_vector<T, N>::reference small_vector<T, N>::
front() {
  return *(this->begin());
}

/**
 * Returns a const reference to the first element.
 */
template<class T, unsigned N>
INLINE typename small_vector<T, N>::const_reference small_vector<T, N>::
front() const {
  return *(this->begin());
}

/**
 * Returns a reference to the last element.
 */
template<class T, unsigned N>
INLINE typename small_vector<T, N>::reference small_vector<T, N>::
back() {
  return *(this->end() - 1);
}

/**
 * Returns a const reference to the last element.
 */
template<class T, unsigned N>
INLINE typename small_vector<T, N>::const_reference small_vector<T, N>::
back() const {
  return *(this->end() - 1);
}

/**
 * Returns a pointer to the data.
 */
template<class T, unsigned N>
ALWAYS_INLINE T *small_vector<T, N>::
data() {
  return this->is_small() ? this->_storage._small : this->_storage._large;
}

/**
 * Returns a pointer to the data.
 */
template<class T, unsigned N>
ALWAYS_INLINE const T *small_vector<T, N>::
data() const {
  return this->is_small() ? this->_storage._small : this->_storage._large;
}

/**
 * Returns an iterator to the first element of the vector.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::iterator small_vector<T, N>::
begin() {
  return this->is_small() ? this->_storage._small : this->_storage._large;
}

/**
 * Returns an iterator one element past the last element in the vector.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::iterator small_vector<T, N>::
end() {
  return this->begin() + _size;
}

/**
 * Returns a const iterator to the first element of the vector.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::const_iterator small_vector<T, N>::
begin() const {
  return this->is_small() ? this->_storage._small : this->_storage._large;
}

/**
 * Returns a const iterator one element past the last element in the vector.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::const_iterator small_vector<T, N>::
end() const {
  return this->begin() + _size;
}

/**
 * Returns a const iterator to the first element of the vector.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::const_iterator small_vector<T, N>::
cbegin() const {
  return this->is_small() ? this->_storage._small : this->_storage._large;
}

/**
 * Returns a const iterator one element past the last element in the vector.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::const_iterator small_vector<T, N>::
cend() const {
  return this->begin() + _size;
}

/**
 * Returns a reverse iterator to the end of the vector.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::reverse_iterator small_vector<T, N>::
rbegin() {
  return reverse_iterator(this->end());
}

/**
 * Returns a reverse iterator to the beginning of the vector.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::reverse_iterator small_vector<T, N>::
rend() {
  return reverse_iterator(this->begin());
}

/**
 * Returns a reverse iterator to the end of the vector.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::const_reverse_iterator small_vector<T, N>::
rbegin() const {
  return const_reverse_iterator(this->end());
}

/**
 * Returns a reverse iterator to the beginning of the vector.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::const_reverse_iterator small_vector<T, N>::
rend() const {
  return const_reverse_iterator(this->begin());
}

/**
 * Returns a reverse iterator to the end of the vector.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::const_reverse_iterator small_vector<T, N>::
crbegin() const {
  return const_reverse_iterator(this->end());
}

/**
 * Returns a reverse iterator to the beginning of the vector.
 */
template<class T, unsigned N>
ALWAYS_INLINE typename small_vector<T, N>::const_reverse_iterator small_vector<T, N>::
crend() const {
  return const_reverse_iterator(this->begin());
}

/**
 * Removes all elements from the vector.  Does not deallocate storage, consider
 * calling shrink_to_fit afterwards.
 */
template<class T, unsigned N>
INLINE void small_vector<T, N>::
clear() {
  for (T &element : *this) {
    element.~T();
  }
  _size = 0;
}

/**
 * Reallocates the size down to the current content size.
 */
template<class T, unsigned N>
INLINE void small_vector<T, N>::
shrink_to_fit() {
  if (_capacity > N && _capacity > _size) {
    T *from = _storage._large;
    T *to;
    if (_size > N) {
      _capacity = _size;
      to = (T *)PANDA_MALLOC_ARRAY(sizeof(T) * _size);
    } else {
      _capacity = N;
      to = _storage._small;
    }
    for (size_type i = 0; i < _size; ++i) {
      new (to + i) T(std::move(from[i]));
      from[i].~T();
    }
    PANDA_FREE_ARRAY(from);
    _capacity = std::max(_size, (size_type)N);
  }
}

/**
 * Allocates enough capacity for the given number of elements.
 */
template<class T, unsigned N>
INLINE void small_vector<T, N>::
reserve(size_type n) {
  if (n > _capacity) {
    T *from = this->data();
    T *to = (T *)PANDA_MALLOC_ARRAY(sizeof(T) * n);
    for (size_type i = 0; i < _size; ++i) {
      new (to + i) T(std::move(from[i]));
      from[i].~T();
    }

    if (!this->is_small()) {
      PANDA_FREE_ARRAY(from);
    }

    //NB. It's not safe to overwrite this until we're done reading from the
    // internal storage (which overlaps with this pointer).
    _storage._large = to;
    _capacity = n;
  }
}

/**
 * Resizes the vector to the given number of elements.
 */
template<class T, unsigned N>
INLINE void small_vector<T, N>::
resize(size_type n, T value) {
  if (n > _size) {
    this->reserve(n);

    T *to = this->data();
    for (size_type i = _size; i < n; ++i) {
      new (to + i) T(std::move(value));
    }
  }
  else {
    T *to = this->data();
    for (size_type i = n; i < _size; ++i) {
      to[i].~T();
    }
  }

  _size = n;
}

/**
 * Constructs a new element at the end of the vector.
 */
template<class T, unsigned N>
template<class... Args>
INLINE typename small_vector<T, N>::reference small_vector<T, N>::
emplace_back(Args&&... args) {
  iterator it = this->append();
  new (it) T(std::forward<Args>(args)...);
  return *it;
}

/**
 * Appends an element to the end of the vector.
 */
template<class T, unsigned N>
INLINE void small_vector<T, N>::
push_back(const T &value) {
  new (this->append()) T(value);
}

/**
 * Appends an element to the end of the vector.
 */
template<class T, unsigned N>
INLINE void small_vector<T, N>::
push_back(T &&value) {
  new (this->append()) T(std::move(value));
}

/**
 * Removes the last element from the vector.  Does not deallocate storage,
 * consider calling shrink_to_fit afterwards.
 */
template<class T, unsigned N>
INLINE void small_vector<T, N>::
pop_back() {
  assert(!this->empty());

  (this->data() + --_size)->~T();
}

/**
 * Inserts the given element before the indicated position.
 */
template<class T, unsigned N>
INLINE typename small_vector<T, N>::iterator small_vector<T, N>::
insert(const_iterator pos, const T &value) {
  iterator new_pos = this->insert_gap(pos, 1);
  new (new_pos) T(value);
  return new_pos;
}

/**
 * Inserts the given element before the indicated position.
 */
template<class T, unsigned N>
INLINE typename small_vector<T, N>::iterator small_vector<T, N>::
insert(const_iterator pos, T &&value) {
  iterator new_pos = this->insert_gap(pos, 1);
  new (new_pos) T(std::move(value));
  return new_pos;
}

/**
 * Removes the element at the given position from the vector.  Usually does not
 * reallocate storage (except when removing from the beginning if the new size
 * is smaller than the preallocated storage), consider calling shrink_to_fit()
 * afterwards.
 */
template<class T, unsigned N>
INLINE typename small_vector<T, N>::iterator small_vector<T, N>::
erase(const_iterator pos) {
  T *data = this->data();
  size_type i = pos - data;
  assert(i < _size);
  --_size;

  T *to = (T *)pos;
  T *to_end = data + _size;
  T *from = to + 1;

  T *free_ptr = nullptr;
  if (i == 0 && _size <= N && !this->is_small()) {
    // We're moving everything anyway, just move it to internal storage
    to = this->_storage._small;
    to_end = to + _size;
    _capacity = N;
    free_ptr = data;
  }

  pos->~T();

  while (to < to_end) {
    new (to++) T(std::move(*from));
    from->~T();
    ++from;
  }

  if (free_ptr != nullptr) {
    PANDA_FREE_ARRAY(free_ptr);
  }

  return to;
}

/**
 * Removes the given range of elements from the vector.  Usually does not
 * reallocate storage (except when removing from the beginning if the new size
 * is smaller than the preallocated storage), consider calling shrink_to_fit()
 * afterwards.
 */
template<class T, unsigned N>
INLINE typename small_vector<T, N>::iterator small_vector<T, N>::
erase(const_iterator begin, const_iterator end) {
  if (begin == end) {
    return (iterator)end;
  }

  T *data = this->data();
  size_type i = begin - data;
  assert(i < _size);
  _size -= (end - begin);
  assert(_size <= _capacity);

  T *to = (T *)begin;
  T *to_end = data + _size;
  T *from = (T *)end;

  T *free_ptr = nullptr;
  if (i == 0 && _size <= N && !this->is_small()) {
    // We're moving everything anyway, just move it to internal storage
    to = this->_storage._small;
    to_end = to + _size;
    _capacity = N;
    free_ptr = data;
  }

  while (begin < end) {
    begin->~T();
    ++begin;
  }

  while (to < to_end) {
    new (to++) T(std::move(*from));
    from->~T();
    ++from;
  }

  if (free_ptr != nullptr) {
    PANDA_FREE_ARRAY(free_ptr);
  }

  return to;
}

/**
 * Shifts elements right starting from the given position.  The gap remains
 * uninitialized.
 */
template<class T, unsigned N>
INLINE typename small_vector<T, N>::iterator small_vector<T, N>::
insert_gap(const_iterator pos, size_type count) {
  T *data = this->data();
  size_type offset = pos - data;
  assert(offset <= _size);
  _size += count;

  if (_size <= _capacity) {
    // Just move everything back.
    for (size_type i = _size - 1; i >= offset + count; --i) {
      new (data + i) T(std::move(data[i - count]));
      data[i - count].~T();
    }
  }
  else {
    // Need to resize, so move everything to the new array.
    size_type new_cap = std::max(_size, _capacity << 1u);

    T *old_data = data;
    data = (T *)PANDA_MALLOC_ARRAY(sizeof(T) * new_cap);

    T *from = old_data;
    T *to = data;
    for (size_type i = 0; i < offset; ++i) {
      new (to++) T(std::move(*from));
      from->~T();
      ++from;
    }

    to += count;

    T *to_end = data + _size;
    while (to < to_end) {
      new (to++) T(std::move(*from));
      from->~T();
      ++from;
    }

    if (old_data != _storage._small) {
      PANDA_FREE_ARRAY(old_data);
    }

    //NB. It's not safe to overwrite this until we're done reading from the
    // internal storage (which overlaps with this pointer).
    _storage._large = data;
    _capacity = new_cap;
  }

  return data + offset;
}

/**
 * Grows the vector in size by one, leaving the new element uninitialized.
 * Returns an iterator to the back.
 */
template<class T, unsigned N>
INLINE typename small_vector<T, N>::iterator small_vector<T, N>::
append() {
  size_type size = _size;
  if (size == _capacity) {
    this->reserve(_capacity << 1u);
  }
  ++_size;
  return this->begin() + size;
}
