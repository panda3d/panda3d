// Filename: ordered_vector.h
// Created by:  drose (20Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef ORDERED_VECTOR_H
#define ORDERED_VECTOR_H

#include "pandabase.h"
#include "config_util.h"

#include "pvector.h"
#include "pset.h"
#include <algorithm>

////////////////////////////////////////////////////////////////////
//       Class : ordered_vector
// Description : This template class presents an interface similar to
//               the STL multiset, but it is implemented using a vector
//               that is kept always in sorted order.
//
//               This allows the ordered_vector to maintain stability
//               of order between elements that sort equally: they are
//               stored in the order in which they were added, from
//               back to front.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare = less<Key> >
class ordered_vector {
private:
  typedef pvector<Key> Vector;
  
public:
  // Typedefs
  typedef Key key_type;
  typedef Key value_type;
  typedef Key &reference;
  typedef const Key &const_reference;
  typedef Compare key_compare;
  typedef Compare value_compare;

  // Be careful when using the non-const iterators that you do not
  // disturb the sorted order of the vector, or that if you do, you
  // call sort() when you are done.
  typedef Vector::iterator iterator;
  typedef Vector::const_iterator const_iterator;
  typedef Vector::reverse_iterator reverse_iterator;
  typedef Vector::const_reverse_iterator const_reverse_iterator;

  typedef Vector::difference_type difference_type;
  typedef Vector::size_type size_type;

public:
  // Constructors.  We don't implement the whole slew of STL
  // constructors here yet.
  INLINE ordered_vector(const Compare &compare = Compare());
  INLINE ordered_vector(const ordered_vector<Key, Compare> &copy);
  INLINE ordered_vector<Key, Compare> &operator = (const ordered_vector<Key, Compare> &copy);
  INLINE ~ordered_vector();

  // Iterator access.
  INLINE iterator begin();
  INLINE iterator end();
  INLINE reverse_iterator rbegin();
  INLINE reverse_iterator rend();

  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;
  INLINE const_reverse_iterator rbegin() const;
  INLINE const_reverse_iterator rend() const;

  // Size information.
  INLINE size_type size() const;
  INLINE size_type max_size() const;
  INLINE bool empty() const;

  // Equivalence and lexicographical comparisons.
  INLINE bool operator == (const ordered_vector<Key, Compare> &other) const;
  INLINE bool operator != (const ordered_vector<Key, Compare> &other) const;

  INLINE bool operator < (const ordered_vector<Key, Compare> &other) const;
  INLINE bool operator > (const ordered_vector<Key, Compare> &other) const;
  INLINE bool operator <= (const ordered_vector<Key, Compare> &other) const;
  INLINE bool operator >= (const ordered_vector<Key, Compare> &other) const;

  // Insert operations.
  iterator insert(iterator position, const value_type &key);
  INLINE iterator insert(const value_type &key);

  // Erase operations.
  INLINE iterator erase(iterator position);
  INLINE size_type erase(const key_type &key);
  INLINE void erase(iterator first, iterator last);
  INLINE void clear();

  // Find operations.
  INLINE iterator find(const key_type &key);
  INLINE const_iterator find(const key_type &key) const;
  INLINE iterator find_particular(const key_type &key);
  INLINE const_iterator find_particular(const key_type &key) const;
  INLINE size_type count(const key_type &key) const;

  INLINE iterator lower_bound(const key_type &key);
  INLINE const_iterator lower_bound(const key_type &key) const;
  INLINE iterator upper_bound(const key_type &key);
  INLINE const_iterator upper_bound(const key_type &key) const;
  INLINE pair<iterator, iterator> equal_range(const key_type &key);
  INLINE pair<const_iterator, const_iterator> equal_range(const key_type &key) const;

  // Special operations.
  INLINE void swap(ordered_vector<Key, Compare> &other);
  INLINE void reserve(size_type n);
  INLINE void sort();

private:
  INLINE iterator nci(const_iterator iterator);
  INLINE iterator find_insert_position(iterator first, iterator last, 
                                       const key_type &key);
  iterator r_find_insert_position(iterator first, iterator last, 
                                  const key_type &key);
  const_iterator r_find(const_iterator first, const_iterator last,
                        const_iterator not_found,
                        const key_type &key) const;
  const_iterator r_find_particular(const_iterator first, const_iterator last,
                                   const_iterator not_found,
                                   const key_type &key) const;
  size_type r_count(const_iterator first, const_iterator last,
                    const key_type &key) const;
  const_iterator r_lower_bound(const_iterator first, const_iterator last,
                               const key_type &key) const;
  const_iterator r_upper_bound(const_iterator first, const_iterator last,
                               const key_type &key) const;
  pair<const_iterator, const_iterator>
  r_equal_range(const_iterator first, const_iterator last,
                const key_type &key) const;
  INLINE bool verify_list();

#ifndef NDEBUG
  bool verify_list_impl(iterator first, iterator last);
#endif

  Compare _compare;
  Vector _vector;
};

#include "ordered_vector.I"
#include "ordered_vector.T"

#endif
