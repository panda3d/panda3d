// Filename: pointerToArray.h
// Created by:  drose (14Jan99)
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

#ifndef POINTERTOARRAY_H
#define POINTERTOARRAY_H


////////////////////////////////////////////////////////////////////
//
// This file defines the classes PointerToArray and
// ConstPointerToArray (and their abbreviations, PTA and CPTA), which
// are extensions to the PointerTo class that support
// reference-counted arrays.
//
// You may think of a PointerToArray as the same thing as a
// traditional C-style array.  However, it actually stores a pointer
// to an STL vector, which is then reference-counted.  Thus, most
// vector operations may be applied directly to a PointerToArray
// object, including dynamic resizing via push_back() and pop_back().
//
// Unlike the PointerTo class, the PointerToArray may store pointers
// to any kind of object, not just those derived from ReferenceCount.
//
// Like PointerTo and ConstPointerTo, the macro abbreviations PTA and
// CPTA are defined for convenience.
//
// Some examples of syntax:              instead of:
//
// PTA(int) array(10);                     int *array = new int[10];
// memset(array, 0, sizeof(int) * 10);     memset(array, 0, sizeof(int) * 10);
// array[i] = array[i+1];                  array[i] = array[i+1];
// num_elements = array.size();             (no equivalent)
//
// PTA(int) copy = array;                  int *copy = array;
//
// Note that in the above example, unlike an STL vector (but like a
// C-style array), assigning a PointerToArray object from another
// simply copies the pointer, and does not copy individual elements.
// (Of course, reference counts are adjusted appropriately.)  If you
// actually wanted an element-by-element copy of the array, you would
// do this:
//
// PTA(int) copy(0);              // Create a pointer to an empty vector.
// copy.v() = array.v();          // v() is the STL vector itself.
//
// The (0) parameter to the constructor in the above example is
// crucial.  When a numeric length parameter, such as zero, is given
// to the constructor, it means to define a new STL vector with that
// number of elements initially in it.  If no parameter is given, on
// the other hand, it means the PointerToArray should point to
// nothing--no STL vector is created.  This is equivalent to a C array
// that points to NULL.
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include "referenceCount.h"
#include "pointerTo.h"
#include "pvector.h"

#if defined(WIN32_VC) && !defined(__INTEL_COMPILER)
// disable mysterious MSVC warning for static inline PTA::empty_array method
// need to chk if vc 7.0 still has this problem, would like to keep it enabled
#pragma warning (disable : 4506)
#endif

////////////////////////////////////////////////////////////////////
//       Class : PointerToArray
// Description : A special kind of PointerTo that stores an array of
//               the indicated element type, instead of a single
//               element.  This is actually implemented as an STL
//               vector, using the RefCountObj class to wrap it up
//               with a reference count.
////////////////////////////////////////////////////////////////////
template <class Element>
class PointerToArray : public PointerToBase<RefCountObj<pvector<Element> > > {
public:
  typedef TYPENAME PointerToBase<RefCountObj<pvector<Element> > >::To To;
  typedef TYPENAME pvector<Element>::value_type value_type;
  typedef TYPENAME pvector<Element>::reference reference;
  typedef TYPENAME pvector<Element>::const_reference const_reference;
  typedef TYPENAME pvector<Element>::iterator iterator;
  typedef TYPENAME pvector<Element>::const_iterator const_iterator;
  typedef TYPENAME pvector<Element>::reverse_iterator reverse_iterator;
  typedef TYPENAME pvector<Element>::const_reverse_iterator const_reverse_iterator;
  typedef TYPENAME pvector<Element>::difference_type difference_type;
  typedef TYPENAME pvector<Element>::size_type size_type;

PUBLISHED:
  INLINE PointerToArray();
  //  INLINE PointerToArray(size_type n);  this is too dangerous to use, since arrays created automatically for any const parameter, use empty_array instead
  INLINE static PointerToArray<Element> empty_array(size_type n);
  INLINE PointerToArray(size_type n, const Element &value);
  INLINE PointerToArray(const PointerToArray<Element> &copy);

public:
  // Duplicating the interface of vector.  The following member
  // functions are all const, because they do not reassign the
  // pointer--they operate only within the vector itself, which is
  // non-const in this class.

  INLINE iterator begin() const;
  INLINE iterator end() const;
  INLINE TYPENAME PointerToArray<Element>::reverse_iterator rbegin() const;
  INLINE TYPENAME PointerToArray<Element>::reverse_iterator rend() const;

  // Equality and comparison operators are pointerwise for
  // PointerToArrays, not elementwise as in vector.

PUBLISHED:
  INLINE size_type size() const;

public:
  INLINE size_type max_size() const;
  INLINE bool empty() const;

  // Functions specific to vectors.
  INLINE void reserve(size_type n);
  INLINE size_type capacity() const;
  INLINE reference front() const;
  INLINE reference back() const;
  INLINE iterator insert(iterator position, const Element &x);
  INLINE void insert(iterator position, size_type n, const Element &x);

  // We don't define the insert() method that accepts a pair of
  // iterators to copy from.  That's problematic because of the whole
  // member template thing.  If you really need this, use
  // pta.v().insert(...); if you're doing this on a vector that has to
  // be exported from the DLL, you should use
  // insert_into_vector(pta.v(), ...).

  INLINE void erase(iterator position);
  INLINE void erase(iterator first, iterator last);

PUBLISHED:
#if !defined(WIN32_VC)
  INLINE reference operator [](size_type n) const;
  INLINE reference operator [](int n) const;
#endif
  INLINE const Element &get_element(size_type n) const;
  INLINE void set_element(size_type n, const Element &value);

  INLINE void push_back(const Element &x);
  INLINE void pop_back();
  INLINE void make_empty();

public:

  INLINE operator Element *() const;
  INLINE Element *p() const;
  INLINE pvector<Element> &v() const;

  //These functions are only to be used in Reading through BamReader.
  //They are designed to work in pairs, so that you register what is
  //returned by get_void_ptr with BamReader and when you are setting
  //another PTA with what is returned by BamReader, you set it with
  //set_void_ptr.  If you used the provided macro of READ_PTA, this is
  //done for you. So you should never call these functions directly
  INLINE void *get_void_ptr() const;
  INLINE void set_void_ptr(void* p);

  INLINE int get_ref_count() const;

  // Reassignment is by pointer, not memberwise as with a vector.
  INLINE PointerToArray<Element> &
  operator = (RefCountObj<pvector<Element> > *ptr);
  INLINE PointerToArray<Element> &
  operator = (const PointerToArray<Element> &copy);
  INLINE void clear();

private:
  // This static empty array is kept around just so we can return
  // something meangful when begin() or end() is called and we have a
  // NULL pointer.  It might not be shared properly between different
  // .so's, since it's a static member of a template class, but we
  // don't really care.
  static pvector<Element> _empty_array;
};

////////////////////////////////////////////////////////////////////
//       Class : ConstPointerToArray
// Description : Similar to PointerToArray, except that its contents
//               may not be modified.
////////////////////////////////////////////////////////////////////
template <class Element>
class ConstPointerToArray : public PointerToBase<RefCountObj<pvector<Element> > > {
public:
  typedef TYPENAME PointerToBase<RefCountObj<pvector<Element> > >::To To;
  typedef TYPENAME pvector<Element>::value_type value_type;
  typedef TYPENAME pvector<Element>::const_reference reference;
  typedef TYPENAME pvector<Element>::const_reference const_reference;
  typedef TYPENAME pvector<Element>::const_iterator iterator;
  typedef TYPENAME pvector<Element>::const_iterator const_iterator;
#ifdef WIN32_VC
  // VC++ seems to break the const_reverse_iterator definition somehow.
  typedef TYPENAME pvector<Element>::reverse_iterator reverse_iterator;
#else
  typedef TYPENAME pvector<Element>::const_reverse_iterator reverse_iterator;
#endif
  typedef TYPENAME pvector<Element>::const_reverse_iterator const_reverse_iterator;
  typedef TYPENAME pvector<Element>::difference_type difference_type;
  typedef TYPENAME pvector<Element>::size_type size_type;

  INLINE ConstPointerToArray();
  INLINE ConstPointerToArray(const PointerToArray<Element> &copy);
  INLINE ConstPointerToArray(const ConstPointerToArray<Element> &copy);

  // Duplicating the interface of vector.

  INLINE iterator begin() const;
  INLINE iterator end() const;
  INLINE TYPENAME ConstPointerToArray<Element>::reverse_iterator rbegin() const;
  INLINE TYPENAME ConstPointerToArray<Element>::reverse_iterator rend() const;

  // Equality and comparison operators are pointerwise for
  // PointerToArrays, not elementwise as in vector.

  INLINE size_type size() const;
  INLINE size_type max_size() const;
  INLINE bool empty() const;

  // Functions specific to vectors.
  INLINE size_type capacity() const;
#ifndef WIN32_VC
  INLINE reference operator [](size_type n) const;
  INLINE reference operator [](int n) const;
#endif
  INLINE reference front() const;
  INLINE reference back() const;

  INLINE operator const Element *() const;
  INLINE const Element *p() const;
  INLINE const pvector<Element> &v() const;

  INLINE int get_ref_count() const;

  // Reassignment is by pointer, not memberwise as with a vector.
  INLINE ConstPointerToArray<Element> &
  operator = (RefCountObj<pvector<Element> > *ptr);
  INLINE ConstPointerToArray<Element> &
  operator = (const PointerToArray<Element> &copy);
  INLINE ConstPointerToArray<Element> &
  operator = (const ConstPointerToArray<Element> &copy);
  INLINE void clear();

private:
  // This static empty array is kept around just so we can return
  // something meangful when begin() or end() is called and we have a
  // NULL pointer.  It might not be shared properly between different
  // .so's, since it's a static member of a template class, but we
  // don't really care.
  static pvector<Element> _empty_array;
};


// And the brevity macros.

#define PTA(type) PointerToArray< type >
#define CPTA(type) ConstPointerToArray< type >

#include "pointerToArray.I"

#endif
