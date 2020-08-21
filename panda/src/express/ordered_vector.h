/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ordered_vector.h
 * @author drose
 * @date 2002-02-20
 */

#ifndef ORDERED_VECTOR_H
#define ORDERED_VECTOR_H
#ifdef CPPPARSER // hack around this for  interigate...
// ****** HACK allert *** this code is intended to tell interigate to not
// expand this class definition past basic names It drops the interigate
// memory foot pront and user time by a bunch on pc cygwin from  3 minutes to
// 17 seconds ?? really need to explore interigate to figure out what is going
// on ..
template<class Key, class Compare = std::less<int>, class Vector = pvector<Key> > class ov_multiset
{
};

template<class Key, class Compare = std::less<int>, class Vector = pvector<Key> > class ov_set
{
};

template<class Key, class Compare = std::less<int>, class Vector = pvector<Key> > class ordered_vector
{
};

#else  // cppparser


#include "pandabase.h"

#include "pvector.h"
#include "pset.h"
#include "pnotify.h"
#include <algorithm>

// There are some inheritance issues with template classes and typedef names.
// Template classes that inherit typedef names from their base class, which is
// also a template class, may confuse the typedef names with globally scoped
// template names.  In particular, the local "iterator" type is easily
// confused with the std::iterator template class.

// To work around this problem, as well as a problem in gcc 2.95.3 with
// value_type etc.  not inheriting properly (even though we explicitly typedef
// them in the derived class), we rename the questionable typedefs here so
// that they no longer conflict with the global template classes.

#define KEY_TYPE key_type_0
#define VALUE_TYPE value_type_0
#define REFERENCE reference_0
#define CONST_REFERENCE const_reference_0
#define KEY_COMPARE key_compare_0
#define VALUE_COMPARE value_compare_0
#define ITERATOR iterator_0
#define CONST_ITERATOR const_iterator_0
#define REVERSE_ITERATOR reverse_iterator_0
#define CONST_REVERSE_ITERATOR const_reverse_iterator_0
#define DIFFERENCE_TYPE difference_type_0
#define SIZE_TYPE size_type_0

/**
 * This template class presents an interface similar to the STL set or
 * multiset (and ov_set and ov_multiset are implemented specifically, below),
 * but it is implemented using a vector that is kept always in sorted order.
 *
 * In most cases, an ov_set or ov_multiset may be dropped in transparently in
 * place of a set or multiset, but the implementation difference has a few
 * implications:
 *
 * (1) The ov_multiset will maintain stability of order between elements that
 * sort equally: they are stored in the order in which they were added, from
 * back to front.
 *
 * (2) Insert and erase operations into the middle of the set can be slow,
 * just as inserting into the middle of a vector can be slow.  In fact,
 * building up an ov_set by inserting elements one at a time is an n^2
 * operation.  On the other hand, building up an ov_set by adding elements to
 * the end, one at time, is somewhat faster than building up a traditional
 * set; and you can even add unsorted elements with push_back() and then call
 * sort() when you're done, for a log(n) operation.
 *
 * (3) Iterators may not be valid for the life of the ordered_vector.  If the
 * vector reallocates itself, all iterators are invalidated.
 *
 * (4) Random access into the set is easy with the [] operator.
 */
template<class Key, class Compare = std::less<Key>, class Vector = pvector<Key> >
class ordered_vector {
public:
  // Typedefs
  typedef Key KEY_TYPE;
  typedef Key VALUE_TYPE;
  typedef Key &REFERENCE;
  typedef const Key &CONST_REFERENCE;
  typedef Compare KEY_COMPARE;
  typedef Compare VALUE_COMPARE;

  // Be careful when using the non-const iterators that you do not disturb the
  // sorted order of the vector, or that if you do, you call sort() when you
  // are done.
  typedef typename Vector::iterator ITERATOR;
  typedef typename Vector::const_iterator CONST_ITERATOR;
  typedef typename Vector::reverse_iterator REVERSE_ITERATOR;
  typedef typename Vector::const_reverse_iterator CONST_REVERSE_ITERATOR;

  typedef typename Vector::difference_type DIFFERENCE_TYPE;
  typedef typename Vector::size_type SIZE_TYPE;

  // Since the #define symbols do not actually expand to the correct names, we
  // have to re-typedef them so callers can reference them by their correct,
  // lowercase names.
  typedef KEY_TYPE key_type;
  typedef VALUE_TYPE value_type;
  typedef REFERENCE reference;
  typedef CONST_REFERENCE const_reference;
  typedef KEY_COMPARE key_compare;
  typedef VALUE_COMPARE value_compare;
  typedef ITERATOR iterator;
  typedef CONST_ITERATOR const_iterator;
  typedef REVERSE_ITERATOR reverse_iterator;
  typedef CONST_REVERSE_ITERATOR const_reverse_iterator;
  typedef DIFFERENCE_TYPE difference_type;
  typedef SIZE_TYPE size_type;

public:
  // Constructors.  We don't implement the whole slew of STL constructors here
  // yet.
  INLINE ordered_vector(TypeHandle type_handle = ov_set_type_handle);
  INLINE ordered_vector(const Compare &compare,
                        TypeHandle type_handle = ov_set_type_handle);

  // Iterator access.
  INLINE ITERATOR begin();
  INLINE ITERATOR end();
  INLINE REVERSE_ITERATOR rbegin();
  INLINE REVERSE_ITERATOR rend();

  INLINE CONST_ITERATOR begin() const;
  INLINE CONST_ITERATOR end() const;
  INLINE CONST_REVERSE_ITERATOR rbegin() const;
  INLINE CONST_REVERSE_ITERATOR rend() const;

  INLINE CONST_ITERATOR cbegin() const;
  INLINE CONST_ITERATOR cend() const;
  INLINE CONST_REVERSE_ITERATOR crbegin() const;
  INLINE CONST_REVERSE_ITERATOR crend() const;

  // Random access.
  INLINE reference operator [] (SIZE_TYPE n);
  INLINE const_reference operator [] (SIZE_TYPE n) const;

  INLINE reference front();
  INLINE const_reference front() const;

  INLINE reference back();
  INLINE const_reference back() const;

  // Size information.
  INLINE SIZE_TYPE size() const;
  INLINE SIZE_TYPE max_size() const;
  INLINE bool empty() const;

  // Equivalence and lexicographical comparisons.
  INLINE bool operator == (const ordered_vector<Key, Compare, Vector> &other) const;
  INLINE bool operator != (const ordered_vector<Key, Compare, Vector> &other) const;

  INLINE bool operator < (const ordered_vector<Key, Compare, Vector> &other) const;
  INLINE bool operator > (const ordered_vector<Key, Compare, Vector> &other) const;
  INLINE bool operator <= (const ordered_vector<Key, Compare, Vector> &other) const;
  INLINE bool operator >= (const ordered_vector<Key, Compare, Vector> &other) const;

  // Insert operations.
  ITERATOR insert_unique(ITERATOR position, const VALUE_TYPE &key);
  ITERATOR insert_nonunique(ITERATOR position, const VALUE_TYPE &key);
  INLINE std::pair<ITERATOR, bool> insert_unique(const VALUE_TYPE &key);
  INLINE ITERATOR insert_nonunique(const VALUE_TYPE &key);
  INLINE ITERATOR insert_unverified(ITERATOR position, const VALUE_TYPE &key);

  // Erase operations.
  INLINE ITERATOR erase(ITERATOR position);
  INLINE SIZE_TYPE erase(const KEY_TYPE &key);
  INLINE void erase(ITERATOR first, ITERATOR last);
  INLINE void clear();

  // Find operations.
  INLINE ITERATOR find(const KEY_TYPE &key);
  INLINE CONST_ITERATOR find(const KEY_TYPE &key) const;
  INLINE ITERATOR find_particular(const KEY_TYPE &key);
  INLINE CONST_ITERATOR find_particular(const KEY_TYPE &key) const;
  INLINE SIZE_TYPE count(const KEY_TYPE &key) const;

  INLINE ITERATOR lower_bound(const KEY_TYPE &key);
  INLINE CONST_ITERATOR lower_bound(const KEY_TYPE &key) const;
  INLINE ITERATOR upper_bound(const KEY_TYPE &key);
  INLINE CONST_ITERATOR upper_bound(const KEY_TYPE &key) const;
  INLINE std::pair<ITERATOR, ITERATOR> equal_range(const KEY_TYPE &key);
  INLINE std::pair<CONST_ITERATOR, CONST_ITERATOR> equal_range(const KEY_TYPE &key) const;

  // Special operations.
  INLINE void swap(ordered_vector<Key, Compare, Vector> &other);
  INLINE void reserve(SIZE_TYPE n);
  INLINE void sort_unique();
  INLINE void sort_nonunique();
  bool verify_list_unique() const;
  bool verify_list_nonunique() const;

  INLINE void push_back(const VALUE_TYPE &key);
  INLINE void push_back(VALUE_TYPE &&key);
  INLINE void pop_back();
  INLINE void resize(SIZE_TYPE n);
  INLINE void resize(SIZE_TYPE n, const VALUE_TYPE &value);

private:
  INLINE ITERATOR nci(CONST_ITERATOR i);
  INLINE ITERATOR find_insert_position(ITERATOR first, ITERATOR last,
                                       const KEY_TYPE &key);
  ITERATOR r_find_insert_position(ITERATOR first, ITERATOR last,
                                  const KEY_TYPE &key);
  CONST_ITERATOR r_find(CONST_ITERATOR first, CONST_ITERATOR last,
                        CONST_ITERATOR not_found,
                        const KEY_TYPE &key) const;
  CONST_ITERATOR r_find_particular(CONST_ITERATOR first, CONST_ITERATOR last,
                                   CONST_ITERATOR not_found,
                                   const KEY_TYPE &key) const;
  SIZE_TYPE r_count(CONST_ITERATOR first, CONST_ITERATOR last,
                    const KEY_TYPE &key) const;
  CONST_ITERATOR r_lower_bound(CONST_ITERATOR first, CONST_ITERATOR last,
                               const KEY_TYPE &key) const;
  CONST_ITERATOR r_upper_bound(CONST_ITERATOR first, CONST_ITERATOR last,
                               const KEY_TYPE &key) const;
  std::pair<CONST_ITERATOR, CONST_ITERATOR>
  r_equal_range(CONST_ITERATOR first, CONST_ITERATOR last,
                const KEY_TYPE &key) const;

  // This function object is used in sort_unique().  It returns true if two
  // consecutive sorted elements are equivalent.
  class EquivalentTest {
  public:
    // For some reason, VC++ won't allow us to define these bodies outside the
    // class; they must be defined here.  The error message is C3206: "member
    // functions of nested classes of a template class cannot be defined
    // outside the class".
    INLINE EquivalentTest(const Compare &compare) :
      _compare(compare) { }
    INLINE bool operator () (const KEY_TYPE &a, const KEY_TYPE &b) {
      nassertr(!_compare(b, a), false);
      return !_compare(a, b);
    }

    Compare _compare;
  };

  Compare _compare;
  Vector _vector;
};

/**
 * A specialization of ordered_vector that emulates a standard STL set: one
 * copy of each element is allowed.
 */
template<class Key, class Compare = std::less<Key>, class Vector = pvector<Key> >
class ov_set : public ordered_vector<Key, Compare, Vector> {
public:
  typedef typename ordered_vector<Key, Compare, Vector>::ITERATOR ITERATOR;
  typedef typename ordered_vector<Key, Compare, Vector>::VALUE_TYPE VALUE_TYPE;

  INLINE ov_set(TypeHandle type_handle = ov_set_type_handle);
  INLINE ov_set(const Compare &compare,
                TypeHandle type_handle = ov_set_type_handle);

  INLINE ITERATOR insert(ITERATOR position, const VALUE_TYPE &key0);
  INLINE std::pair<ITERATOR, bool> insert(const VALUE_TYPE &key0);

  INLINE void sort();
  INLINE bool verify_list() const;
};

/**
 * A specialization of ordered_vector that emulates a standard STL set: many
 * copies of each element are allowed.
 */
template<class Key, class Compare = std::less<Key>, class Vector = pvector<Key> >
class ov_multiset : public ordered_vector<Key, Compare, Vector> {
public:
  typedef typename ordered_vector<Key, Compare, Vector>::ITERATOR ITERATOR;
  typedef typename ordered_vector<Key, Compare, Vector>::VALUE_TYPE VALUE_TYPE;

  INLINE ov_multiset(TypeHandle type_handle = ov_set_type_handle);
  INLINE ov_multiset(const Compare &compare,
                     TypeHandle type_handle = ov_set_type_handle);

  INLINE ITERATOR insert(ITERATOR position, const VALUE_TYPE &key);
  INLINE ITERATOR insert(const VALUE_TYPE &key);

  INLINE void sort();
  INLINE bool verify_list() const;
};

#include "ordered_vector.I"
#include "ordered_vector.T"
#endif // cppparser ..
#endif
