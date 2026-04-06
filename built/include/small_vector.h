/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file small_vector.h
 * @author rdb
 * @date 2023-01-25
 */

#ifndef SMALL_VECTOR_H
#define SMALL_VECTOR_H

#include "dtoolbase.h"
#include "typeHandle.h"

#include <stdint.h>
#include <utility>
#include <iterator>

/**
 * A vector type that is particularly optimized for the case where there is a
 * small number of elements (by default as many as fit inside the space of a
 * single pointer, but at least one).  No heap allocations are required as long
 * as the vector stays below this size.  It also has the same footprint as
 * std::vector if the element is only a single pointer in size.
 *
 * The number of preallocated elements may also be 0, in which case the vector
 * behaves as a regular vector, only benefiting from the reduced footprint.
 *
 * Furthermore, it can be safely used at static init time due to the existence
 * of a constexpr constructor.
 *
 * However, some other operations are probably less efficient.  For example,
 * swapping a small_vector is more expensive, so this operation is not provided.
 *
 * If memory is allocated, it is not automatically deallocated (even if the
 * vector is resized down to zero) but see shrink_to_fit() to accomplish this.
 */
template<class T, unsigned N = (sizeof(T) >= sizeof(void *) ? 1 : sizeof(void *) / sizeof(T))>
class small_vector {
public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T &reference;
  typedef const T &const_reference;
  typedef T *iterator;
  typedef const T *const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  ALWAYS_INLINE constexpr small_vector() = default;
  INLINE small_vector(TypeHandle type_handle);
  INLINE small_vector(std::initializer_list<T> init);
  INLINE small_vector(const small_vector &copy);
  INLINE small_vector(small_vector &&from) noexcept;
  INLINE ~small_vector();

  INLINE small_vector &operator =(const small_vector &copy);
  INLINE small_vector &operator =(small_vector &&from) noexcept;

  constexpr bool empty() const;
  constexpr size_type size() const;
  constexpr size_type capacity() const;
  constexpr size_type max_size() const;

  INLINE reference at(size_type i);
  INLINE const_reference at(size_type i) const;
  ALWAYS_INLINE reference operator [](size_type i);
  ALWAYS_INLINE const_reference operator [](size_type i) const;

  ALWAYS_INLINE reference front();
  ALWAYS_INLINE const_reference front() const;
  ALWAYS_INLINE reference back();
  ALWAYS_INLINE const_reference back() const;
  ALWAYS_INLINE T *data();
  ALWAYS_INLINE const T *data() const;

  ALWAYS_INLINE iterator begin();
  ALWAYS_INLINE iterator end();
  ALWAYS_INLINE const_iterator begin() const;
  ALWAYS_INLINE const_iterator end() const;
  ALWAYS_INLINE const_iterator cbegin() const;
  ALWAYS_INLINE const_iterator cend() const;

  ALWAYS_INLINE reverse_iterator rbegin();
  ALWAYS_INLINE reverse_iterator rend();
  ALWAYS_INLINE const_reverse_iterator rbegin() const;
  ALWAYS_INLINE const_reverse_iterator rend() const;
  ALWAYS_INLINE const_reverse_iterator crbegin() const;
  ALWAYS_INLINE const_reverse_iterator crend() const;

  INLINE void clear();
  INLINE void shrink_to_fit();
  INLINE void reserve(size_type n);
  INLINE void resize(size_type n, T value = T());

  template<class... Args>
  INLINE reference emplace_back(Args&&... args);
  INLINE void push_back(const T &value);
  INLINE void push_back(T &&value);
  INLINE void pop_back();
  INLINE iterator insert(const_iterator pos, const T &value);
  INLINE iterator insert(const_iterator pos, T &&value);
  INLINE iterator erase(const_iterator pos);
  INLINE iterator erase(const_iterator begin, const_iterator end);

private:
  INLINE iterator insert_gap(const_iterator pos, size_type count);
  INLINE iterator append();

  constexpr bool is_small() const { return LIKELY(_capacity <= N); }

  union Storage {
    constexpr Storage() : _large(nullptr) {}
    constexpr Storage(T *large) : _large(large) {}
    ~Storage() {}

    T *_large;
    T _small[N];
  } _storage;

  size_type _size = 0;
  size_type _capacity = N;
};

#include "small_vector.I"

#endif
