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
//               the STL set or multiset (and ov_set and ov_multiset
//               are implemented specifically, below), but it is
//               implemented using a vector that is kept always in
//               sorted order.
//
//               In most cases, an ov_set or ov_multiset may be
//               dropped in transparently in place of a set or
//               multiset, but the implementation difference has a few
//               implications:
//
//               (1) The ov_multiset will maintain stability of order
//               between elements that sort equally: they are stored
//               in the order in which they were added, from back to
//               front.
//
//               (2) Insert and erase operations into the middle of
//               the set can be slow, just as inserting into the
//               middle of a vector can be slow.  In fact, building up
//               an ov_set by inserting elements one at a time is an
//               n^2 operation.  On the other hand, building up an
//               ov_set by adding elements to the end, one at time, is
//               somewhat faster than building up a traditional set;
//               and you can even add unsorted elements with
//               push_back() and then call sort() when you're done,
//               for a log(n) operation.
//
//               (3) Iterators may not be valid for the life of the
//               ordered_vector.  If the vector reallocates itself,
//               all iterators are invalidated.
//
//               (4) Random access into the set is easy with the []
//               operator.
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

  // Random access.
  INLINE reference operator [] (size_type n);
  INLINE const_reference operator [] (size_type n) const;

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
  iterator insert_unique(iterator position, const value_type &key);
  iterator insert_nonunique(iterator position, const value_type &key);
  INLINE pair<iterator, bool> insert_unique(const value_type &key);
  INLINE iterator insert_nonunique(const value_type &key);

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
  INLINE void sort_unique();
  INLINE void sort_nonunique();

  INLINE void push_back(const value_type &key);

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

  // This function object is used in sort_unique().  It returns true
  // if two consecutive sorted elements are equivalent.
  class EquivalentTest {
  public:
    // For some reason, VC++ won't allow us to define these bodies
    // outside the class; they must be defined here.  The error
    // message is C3206: "member functions of nested classes of a
    // template class cannot be defined outside the class".
    INLINE EquivalentTest(const Compare &compare) :
      _compare(compare) { }
    INLINE bool operator () (const key_type &a, const key_type &b) {
      nassertr(!_compare(b, a), false);
      return !_compare(a, b);
    }

    Compare _compare;
  };

  Compare _compare;
  Vector _vector;
};

////////////////////////////////////////////////////////////////////
//       Class : ov_set
// Description : A specialization of ordered_vector that emulates a
//               standard STL set: one copy of each element is
//               allowed.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare = less<Key> >
class ov_set : public ordered_vector<Key, Compare> {
public:
  // The Intel compiler doesn't seem to inherit these typedefs
  // completely--it gets confused in certain cases.  We'll make it
  // explicit.
  typedef ordered_vector<Key, Compare>::key_type key_type;
  typedef ordered_vector<Key, Compare>::value_type value_type;
  typedef ordered_vector<Key, Compare>::reference reference;
  typedef ordered_vector<Key, Compare>::const_reference const_reference;
  typedef ordered_vector<Key, Compare>::key_compare key_compare;
  typedef ordered_vector<Key, Compare>::value_compare value_compare;
  typedef ordered_vector<Key, Compare>::iterator iterator;
  typedef ordered_vector<Key, Compare>::const_iterator const_iterator;
  typedef ordered_vector<Key, Compare>::reverse_iterator reverse_iterator;
  typedef ordered_vector<Key, Compare>::const_reverse_iterator const_reverse_iterator;
  typedef ordered_vector<Key, Compare>::difference_type difference_type;
  typedef ordered_vector<Key, Compare>::size_type size_type;

  INLINE ov_set(const Compare &compare = Compare());
  INLINE ov_set(const ov_set<Key, Compare> &copy);
  INLINE ov_set<Key, Compare> &operator = (const ov_set<Key, Compare> &copy);

  INLINE iterator insert(iterator position, const value_type &key);
  INLINE pair<iterator, bool> insert(const value_type &key);

  INLINE void sort();
};

////////////////////////////////////////////////////////////////////
//       Class : ov_multiset
// Description : A specialization of ordered_vector that emulates a
//               standard STL set: many copies of each element are
//               allowed.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare = less<Key> >
class ov_multiset : public ordered_vector<Key, Compare> {
public:
  // The Intel compiler doesn't seem to inherit these typedefs
  // completely--it gets confused in certain cases.  We'll make it
  // explicit.
  typedef ordered_vector<Key, Compare>::key_type key_type;
  typedef ordered_vector<Key, Compare>::value_type value_type;
  typedef ordered_vector<Key, Compare>::reference reference;
  typedef ordered_vector<Key, Compare>::const_reference const_reference;
  typedef ordered_vector<Key, Compare>::key_compare key_compare;
  typedef ordered_vector<Key, Compare>::value_compare value_compare;
  typedef ordered_vector<Key, Compare>::iterator iterator;
  typedef ordered_vector<Key, Compare>::const_iterator const_iterator;
  typedef ordered_vector<Key, Compare>::reverse_iterator reverse_iterator;
  typedef ordered_vector<Key, Compare>::const_reverse_iterator const_reverse_iterator;
  typedef ordered_vector<Key, Compare>::difference_type difference_type;
  typedef ordered_vector<Key, Compare>::size_type size_type;

  INLINE ov_multiset(const Compare &compare = Compare());
  INLINE ov_multiset(const ov_multiset<Key, Compare> &copy);
  INLINE ov_multiset<Key, Compare> &operator = (const ov_multiset<Key, Compare> &copy);

  INLINE iterator insert(iterator position, const value_type &key);
  INLINE iterator insert(const value_type &key);

  INLINE void sort();
};

#include "ordered_vector.I"
#include "ordered_vector.T"

#endif
