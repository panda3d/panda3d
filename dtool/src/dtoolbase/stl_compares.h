// Filename: stl_compares.h
// Created by:  drose (28Sep04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef STL_COMPARES_H
#define STL_COMPARES_H

#include "dtoolbase.h"
#include "cmath.h"
#include "nearly_zero.h"

#ifdef HAVE_STL_HASH
#include <hash_map>  // for hash_compare

#define stl_hash_compare hash_compare

#else

#include <map>  // for less

// This is declared for the cases in which we don't have STL_HASH
// available--it's just a name to inherit from, but there's no need to
// provide much functionality in the base class (since it won't
// actually be used for hashing, just for comparing).
template<class Key, class Compare = less<Key> >
class stl_hash_compare : public Compare {
};

#endif  // HAVE_STL_HASH

////////////////////////////////////////////////////////////////////
//       Class : floating_point_threshold
// Description : Compares two floating point numbers, within threshold
//               of equivalence.
////////////////////////////////////////////////////////////////////
template<class Key>
class floating_point_threshold {
public:
  INLINE floating_point_threshold(Key threshold = get_nearly_zero_value((Key)0));
  INLINE bool operator () (const Key &a, const Key &b) const;
  const Key _threshold;
};

////////////////////////////////////////////////////////////////////
//       Class : compare_to
// Description : An STL function object class, this is intended to be
//               used on any ordered collection of class objects that
//               contain a compare_to() method.  It defines the order
//               of the objects via compare_to().
////////////////////////////////////////////////////////////////////
template<class Key>
class compare_to {
public:
  INLINE bool operator () (const Key &a, const Key &b) const;
};

////////////////////////////////////////////////////////////////////
//       Class : indirect_less
// Description : An STL function object class, this is intended to be
//               used on any ordered collection of pointers to classes
//               that contain an operator <() method.  It defines the
//               order of the pointers via operator <().
////////////////////////////////////////////////////////////////////
template<class Key>
class indirect_less {
public:
  INLINE bool operator () (const Key &a, const Key &b) const;
};

////////////////////////////////////////////////////////////////////
//       Class : indirect_compare_to
// Description : An STL function object class, this is intended to be
//               used on any ordered collection of pointers to classes
//               that contain a compare_to() method.  It defines the
//               order of the pointers via compare_to().
////////////////////////////////////////////////////////////////////
template<class Key>
class indirect_compare_to {
public:
  INLINE bool operator () (const Key &a, const Key &b) const;
};

////////////////////////////////////////////////////////////////////
//       Class : indirect_compare_names
// Description : An STL function object class, this is intended to be
//               used on any ordered collection of pointers to classes
//               that define a get_name() method, particularly for
//               things that derive from Namable.  It defines the
//               order of the pointers by case-sensitive name
//               comparison.
////////////////////////////////////////////////////////////////////
template<class Key>
class indirect_compare_names {
public:
  INLINE bool operator () (const Key &a, const Key &b) const;
};

////////////////////////////////////////////////////////////////////
//       Class : integer_hash
// Description : This is the default hash_compare class, which assumes
//               the Key is a size_t value or can be implicitly
//               converted to a size_t value (for instance, via a
//               size_t typecast operator).  It is the same as the
//               system-provided hash_compare.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare = less<Key> >
class integer_hash : public stl_hash_compare<Key, Compare> {
public:
  INLINE static size_t add_hash(size_t start, const Key &key);
};

////////////////////////////////////////////////////////////////////
//       Class : floating_point_hash
// Description : This hash_compare class hashes a float or a double.
////////////////////////////////////////////////////////////////////
template<class Key>
class floating_point_hash : public stl_hash_compare<Key> {
public:
  INLINE floating_point_hash(Key threshold = get_nearly_zero_value((Key)0));
  INLINE size_t operator () (const Key &key) const;
  INLINE bool operator () (const Key &a, const Key &b) const;
  INLINE size_t add_hash(size_t start, const Key &key) const;
  const Key _threshold;
};

////////////////////////////////////////////////////////////////////
//       Class : sequence_hash
// Description : This hash_compare class hashes a string.  It assumes
//               the Key is a string or provides begin() and end()
//               methods that iterate through Key::value_type.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare = less<Key> >
class sequence_hash : public stl_hash_compare<Key, Compare> {
public:
  INLINE size_t operator () (const Key &key) const;
  INLINE bool operator () (const Key &a, const Key &b) const {
    return stl_hash_compare<Key, Compare>::operator () (a, b);
  }
  INLINE static size_t add_hash(size_t start, const Key &key);
};

////////////////////////////////////////////////////////////////////
//       Class : method_hash
// Description : This hash_compare class hashes a class object.  It
//               assumes the Key provides a method called get_hash()
//               that returns a size_t.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare = less<Key> >
class method_hash : public stl_hash_compare<Key, Compare> {
public:
  INLINE size_t operator () (const Key &key) const;
  INLINE bool operator () (const Key &a, const Key &b) const {
    return stl_hash_compare<Key, Compare>::operator () (a, b);
  }
};

////////////////////////////////////////////////////////////////////
//       Class : indirect_method_hash
// Description : This hash_compare class hashes a pointer to a class
//               object.  It assumes the Key is a pointer to a class
//               that provides a method called get_hash() that returns
//               a size_t.
////////////////////////////////////////////////////////////////////
template<class Key, class Compare>
class indirect_method_hash : public stl_hash_compare<Key, Compare> {
public:
  INLINE size_t operator () (const Key &key) const;
  INLINE bool operator () (const Key &a, const Key &b) const {
    return stl_hash_compare<Key, Compare>::operator () (a, b);
  }
};

#include "stl_compares.I"

typedef floating_point_hash<float> float_hash;
typedef floating_point_hash<double> double_hash;
typedef integer_hash<const void *> pointer_hash;
typedef integer_hash<int> int_hash;
typedef integer_hash<size_t> size_t_hash;
typedef sequence_hash<string> string_hash;
typedef sequence_hash<wstring> wstring_hash;

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


