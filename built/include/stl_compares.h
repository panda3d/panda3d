/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stl_compares.h
 * @author drose
 * @date 2004-09-28
 */

#ifndef STL_COMPARES_H
#define STL_COMPARES_H

#include "dtoolbase.h"
#include "cmath.h"
#include "nearly_zero.h"
#include "addHash.h"

#include <assert.h>

#ifdef HAVE_STL_HASH
#include <hash_map>  // for hash_compare

template<class Key, class Compare = std::less<Key> >
class stl_hash_compare : public stdext::hash_compare<Key, Compare> {
public:
  INLINE bool is_equal(const Key &a, const Key &b) const {
    return !operator()(a, b) && !operator()(b, a);
  }
};

#else

#include <map>  // for less

// This is declared for the cases in which we don't have STL_HASH available.
template<class Key, class Compare = std::less<Key> >
class stl_hash_compare : public Compare {
public:
  INLINE size_t operator () (const Key &key) const {
    return (size_t)key;
  }
  INLINE bool operator () (const Key &a, const Key &b) const {
    return Compare::operator ()(a, b);
  }
  INLINE bool is_equal(const Key &a, const Key &b) const {
    return !operator()(a, b) && !operator()(b, a);
  }
};

#endif  // HAVE_STL_HASH

/**
 * Compares two floating point numbers, within threshold of equivalence.
 */
template<class Key>
class floating_point_threshold {
public:
  INLINE floating_point_threshold(Key threshold = get_nearly_zero_value((Key)0));
  INLINE bool operator () (const Key &a, const Key &b) const;
  const Key _threshold;
};

/**
 * An STL function object class, this is intended to be used on any ordered
 * collection of class objects that contain a compare_to() method.  It defines
 * the order of the objects via compare_to().
 */
template<class Key>
class compare_to {
public:
  INLINE bool operator () (const Key &a, const Key &b) const;
  INLINE bool is_equal(const Key &a, const Key &b) const;
};

/**
 * An STL function object class, this is intended to be used on any ordered
 * collection of pointers to classes that contain an operator <() method.  It
 * defines the order of the pointers via operator <().
 */
template<class Key>
class indirect_less {
public:
  INLINE bool operator () (const Key &a, const Key &b) const;
};

/**
 * An STL function object class, this is intended to be used on any ordered
 * collection of pointers to classes that contain a compare_to() method.  It
 * defines the order of the pointers via compare_to().
 */
template<class Key>
class indirect_compare_to {
public:
  INLINE bool operator () (const Key &a, const Key &b) const;
  INLINE bool is_equal(const Key &a, const Key &b) const;
};

/**
 * An STL function object class, this is intended to be used on any ordered
 * collection of pointers to classes that define a get_name() method,
 * particularly for things that derive from Namable.  It defines the order of
 * the pointers by case-sensitive name comparison.
 */
template<class Key>
class indirect_compare_names {
public:
  INLINE bool operator () (const Key &a, const Key &b) const;
  INLINE bool is_equal(const Key &a, const Key &b) const;
};

/**
 * This is the default hash_compare class, which assumes the Key is a size_t
 * value or can be implicitly converted to a size_t value (for instance, via a
 * size_t typecast operator).  It is the same as the system-provided
 * hash_compare.
 */
template<class Key, class Compare = std::less<Key> >
class integer_hash : public stl_hash_compare<Key, Compare> {
public:
  INLINE static size_t add_hash(size_t start, const Key &key);
};

/**
 * This is the default hash_compare class, which assumes the Key is a pointer
 * value.  It is the same as the system-provided hash_compare.
 */
class pointer_hash : public stl_hash_compare<const void *, std::less<const void *> > {
public:
  INLINE static size_t add_hash(size_t start, const void *key);
};

/**
 * This hash_compare class hashes a float or a double.
 */
template<class Key>
class floating_point_hash : public stl_hash_compare<Key> {
public:
  INLINE floating_point_hash(Key threshold = get_nearly_zero_value((Key)0));
  INLINE size_t operator () (const Key &key) const;
  INLINE bool operator () (const Key &a, const Key &b) const;
  INLINE size_t add_hash(size_t start, const Key &key) const;
  const Key _threshold;
};

/**
 * This hash_compare class hashes a string.  It assumes the Key is a string or
 * provides begin() and end() methods that iterate through Key::value_type.
 */
template<class Key, class Compare = std::less<Key> >
class sequence_hash : public stl_hash_compare<Key, Compare> {
public:
  INLINE size_t operator () (const Key &key) const;
  INLINE bool operator () (const Key &a, const Key &b) const {
    return stl_hash_compare<Key, Compare>::operator () (a, b);
  }
  INLINE static size_t add_hash(size_t start, const Key &key);
};

/**
 * This hash_compare class hashes a class object.  It assumes the Key provides
 * a method called get_hash() that returns a size_t.
 */
template<class Key, class Compare = std::less<Key> >
class method_hash : public stl_hash_compare<Key, Compare> {
public:
  INLINE size_t operator () (const Key &key) const;
  INLINE bool operator () (const Key &a, const Key &b) const {
    return stl_hash_compare<Key, Compare>::operator () (a, b);
  }
};

/**
 * This hash_compare class hashes a pointer to a class object.  It assumes the
 * Key is a pointer to a class that provides a method called get_hash() that
 * returns a size_t.
 */
template<class Key, class Compare>
class indirect_method_hash : public stl_hash_compare<Key, Compare> {
public:
  INLINE size_t operator () (const Key &key) const;
  INLINE bool operator () (const Key &a, const Key &b) const {
    return stl_hash_compare<Key, Compare>::operator () (a, b);
  }
};

/**
 * An STL function object class, this is intended to be used on any ordered
 * collection of pointers to classes that contain an operator ==() method.  It
 * defines the equality of the pointers via operator ==().
 *
 * Since it doesn't define the ordering of the pointers, it can only be used
 * with hash containers.
 */
template<class Key>
class indirect_equals_hash {
public:
  INLINE size_t operator () (const Key &key) const;
  INLINE bool is_equal(const Key &a, const Key &b) const;
};

#include "stl_compares.I"

typedef floating_point_hash<float> float_hash;
typedef floating_point_hash<double> double_hash;
typedef integer_hash<int> int_hash;
typedef integer_hash<size_t> size_t_hash;
typedef sequence_hash<std::string> string_hash;
typedef sequence_hash<std::wstring> wstring_hash;

template<class Key>
class indirect_less_hash : public indirect_method_hash<Key, indirect_less<Key> > {
};

template<class Key>
class indirect_compare_to_hash : public indirect_method_hash<Key, indirect_compare_to<Key> > {
};

template<class Key>
class indirect_compare_names_hash : public indirect_method_hash<Key, indirect_compare_names<Key> > {
};

#endif
