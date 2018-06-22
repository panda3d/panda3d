/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerToArray.h
 * @author drose
 * @date 1999-01-14
 */

#ifndef POINTERTOARRAY_H
#define POINTERTOARRAY_H

/*
 * This file defines the classes PointerToArray and ConstPointerToArray (and
 * their abbreviations, PTA and CPTA), which are extensions to the PointerTo
 * class that support reference-counted arrays.
 *
 * You may think of a PointerToArray as the same thing as a traditional
 * C-style array.  However, it actually stores a pointer to an STL vector,
 * which is then reference-counted.  Thus, most vector operations may be
 * applied directly to a PointerToArray object, including dynamic resizing via
 * push_back() and pop_back().
 *
 * Unlike the PointerTo class, the PointerToArray may store pointers to any
 * kind of object, not just those derived from ReferenceCount.
 *
 * Like PointerTo and ConstPointerTo, the macro abbreviations PTA and CPTA are
 * defined for convenience.
 *
 * Some examples of syntax:              instead of:
 *
 * PTA(int) array(10);                     int *array = new int[10];
 * memset(array, 0, sizeof(int) * 10);     memset(array, 0, sizeof(int) * 10);
 * array[i] = array[i+1];                  array[i] = array[i+1];
 * num_elements = array.size();             (no equivalent)
 *
 * PTA(int) copy = array;                  int *copy = array;
 *
 * Note that in the above example, unlike an STL vector (but like a C-style
 * array), assigning a PointerToArray object from another simply copies the
 * pointer, and does not copy individual elements. (Of course, reference
 * counts are adjusted appropriately.)  If you actually wanted an
 * element-by-element copy of the array, you would do this:
 *
 * PTA(int) copy(0);              // Create a pointer to an empty vector.
 * copy.v() = array.v();          // v() is the STL vector itself.
 *
 * The (0) parameter to the constructor in the above example is crucial.  When
 * a numeric length parameter, such as zero, is given to the constructor, it
 * means to define a new STL vector with that number of elements initially in
 * it.  If no parameter is given, on the other hand, it means the
 * PointerToArray should point to nothing--no STL vector is created.  This is
 * equivalent to a C array that points to NULL.
 */

#include "pandabase.h"

#include "pointerToArrayBase.h"

#if (defined(WIN32_VC) || defined(WIN64_VC)) && !defined(__INTEL_COMPILER)
// disable mysterious MSVC warning for static inline PTA::empty_array method
// need to chk if vc 7.0 still has this problem, would like to keep it enabled
#pragma warning (disable : 4506)
#endif

template <class Element>
class ConstPointerToArray;

/**
 * A special kind of PointerTo that stores an array of the indicated element
 * type, instead of a single element.  This is actually implemented as an STL
 * vector, using the RefCountObj class to wrap it up with a reference count.
 *
 * We actually inherit from NodeRefCountObj these days, which adds node_ref()
 * and node_unref() to the standard ref() and unref().  This is particularly
 * useful for GeomVertexArrayData; other classes may or may not find this
 * additional counter useful, but since it adds relatively little overhead
 * (compared with what is presumably a largish array), we go ahead and add it
 * here, even though it is inherited by many different parts of the system
 * that may not use it.
 */
template <class Element>
class PointerToArray : public PointerToArrayBase<Element> {
public:
  // By hiding this template from interrogate, we would improve compile-time
  // speed and memory utilization.  However, we do want to export a minimal
  // subset of this class.  So we define just the exportable interface here.
#ifdef CPPPARSER
PUBLISHED:
  typedef typename pvector<Element>::size_type size_type;
  INLINE PointerToArray(TypeHandle type_handle = get_type_handle(Element));
  INLINE static PointerToArray<Element> empty_array(size_type n, TypeHandle type_handle = get_type_handle(Element));
  INLINE PointerToArray(const PointerToArray<Element> &copy);

  EXTENSION(PointerToArray(PyObject *self, PyObject *source));

  INLINE void clear();

  INLINE size_type size() const;
  INLINE void push_back(const Element &x);
  INLINE void pop_back();
  INLINE const Element &get_element(size_type n) const;
  INLINE void set_element(size_type n, const Element &value);
  EXTENSION(const Element &__getitem__(size_type n) const);
  EXTENSION(void __setitem__(size_type n, const Element &value));
  EXTENSION(PyObject *get_data() const);
  EXTENSION(void set_data(PyObject *data));
  EXTENSION(PyObject *get_subdata(size_type n, size_type count) const);
  INLINE void set_subdata(size_type n, size_type count, const std::string &data);
  INLINE int get_ref_count() const;
  INLINE int get_node_ref_count() const;

  INLINE size_t count(const Element &) const;

#ifdef HAVE_PYTHON
  EXTENSION(int __getbuffer__(PyObject *self, Py_buffer *view, int flags));
  EXTENSION(void __releasebuffer__(PyObject *self, Py_buffer *view) const);
#endif

#else  // CPPPARSER
  // This is the actual, complete interface.
  typedef typename PointerToArrayBase<Element>::To To;
  typedef typename pvector<Element>::value_type value_type;
  typedef typename pvector<Element>::reference reference;
  typedef typename pvector<Element>::const_reference const_reference;
  typedef typename pvector<Element>::iterator iterator;
  typedef typename pvector<Element>::const_iterator const_iterator;
  typedef typename pvector<Element>::reverse_iterator reverse_iterator;
  typedef typename pvector<Element>::const_reverse_iterator const_reverse_iterator;
  typedef typename pvector<Element>::difference_type difference_type;
  typedef typename pvector<Element>::size_type size_type;

public:
  INLINE PointerToArray(TypeHandle type_handle = get_type_handle(Element));
  INLINE static PointerToArray<Element> empty_array(size_type n, TypeHandle type_handle = get_type_handle(Element));
  INLINE PointerToArray(size_type n, const Element &value, TypeHandle type_handle = get_type_handle(Element));
  INLINE PointerToArray(const Element *begin, const Element *end, TypeHandle type_handle = get_type_handle(Element));
  INLINE PointerToArray(const PointerToArray<Element> &copy);
  INLINE PointerToArray(PointerToArray<Element> &&from) noexcept;
  INLINE explicit PointerToArray(pvector<Element> &&from, TypeHandle type_handle = get_type_handle(Element));

public:
  // Duplicating the interface of vector.  The following member functions are
  // all const, because they do not reassign the pointer--they operate only
  // within the vector itself, which is non-const in this class.

  INLINE iterator begin() const;
  INLINE iterator end() const;
  INLINE typename PointerToArray<Element>::reverse_iterator rbegin() const;
  INLINE typename PointerToArray<Element>::reverse_iterator rend() const;

  // Equality and comparison operators are pointerwise for PointerToArrays,
  // not elementwise as in vector.
  INLINE size_type size() const;
  INLINE size_type max_size() const;
  INLINE bool empty() const;

  INLINE void clear();

  // Functions specific to vectors.
  INLINE void reserve(size_type n);
  INLINE void resize(size_type n);
  INLINE size_type capacity() const;
  INLINE reference front() const;
  INLINE reference back() const;
  INLINE iterator insert(iterator position, const Element &x);
  INLINE void insert(iterator position, size_type n, const Element &x);

  // We don't define the insert() method that accepts a pair of iterators to
  // copy from.  That's problematic because of the whole member template
  // thing.  If you really need this, use pta.v().insert(...); if you're doing
  // this on a vector that has to be exported from the DLL, you should use
  // insert_into_vector(pta.v(), ...).

  INLINE void erase(iterator position);
  INLINE void erase(iterator first, iterator last);

#if !defined(WIN32_VC) && !defined (WIN64_VC)
  INLINE reference operator [](size_type n) const;
  INLINE reference operator [](int n) const;
#endif

  INLINE void push_back(const Element &x);
  INLINE void pop_back();
  INLINE void make_empty();

  INLINE operator Element *() const;
  INLINE Element *p() const;
  INLINE pvector<Element> &v() const;
  INLINE ReferenceCountedVector<Element> *v0() const;

  // Methods to help out Python and other high-level languages.
  INLINE const Element &get_element(size_type n) const;
  INLINE void set_element(size_type n, const Element &value);
  INLINE std::string get_data() const;
  INLINE void set_data(const std::string &data);
  INLINE std::string get_subdata(size_type n, size_type count) const;
  INLINE void set_subdata(size_type n, size_type count, const std::string &data);

  // These functions are only to be used in Reading through BamReader.  They
  // are designed to work in pairs, so that you register what is returned by
  // get_void_ptr with BamReader and when you are setting another PTA with
  // what is returned by BamReader, you set it with set_void_ptr.  If you used
  // the provided macro of READ_PTA, this is done for you.  So you should
  // never call these functions directly
  INLINE void *get_void_ptr() const;
  INLINE void set_void_ptr(void* p);

  INLINE int get_ref_count() const;
  INLINE void ref() const;
  INLINE bool unref() const;

  INLINE int get_node_ref_count() const;
  INLINE void node_ref() const;
  INLINE bool node_unref() const;

  INLINE size_t count(const Element &) const;

#endif  // CPPPARSER

public:
  // Reassignment is by pointer, not memberwise as with a vector.
  INLINE PointerToArray<Element> &
  operator = (ReferenceCountedVector<Element> *ptr);
  INLINE PointerToArray<Element> &
  operator = (const PointerToArray<Element> &copy);
  INLINE PointerToArray<Element> &
  operator = (PointerToArray<Element> &&from) noexcept;

private:
  TypeHandle _type_handle;

  // This static empty array is kept around just so we can return something
  // meaningful when begin() or end() is called and we have a NULL pointer.
  // It might not be shared properly between different .so's, since it's a
  // static member of a template class, but we don't really care.
  static pvector<Element> _empty_array;

  friend class ConstPointerToArray<Element>;
};

/**
 * Similar to PointerToArray, except that its contents may not be modified.
 */
template <class Element>
class ConstPointerToArray : public PointerToArrayBase<Element> {
public:
  INLINE ConstPointerToArray(TypeHandle type_handle = get_type_handle(Element));

  // By hiding this template from interrogate, we would improve compile-time
  // speed and memory utilization.  However, we do want to export a minimal
  // subset of this class.  So we define just the exportable interface here.
#ifdef CPPPARSER
PUBLISHED:
  INLINE ConstPointerToArray(const PointerToArray<Element> &copy);
  INLINE ConstPointerToArray(const ConstPointerToArray<Element> &copy);

  INLINE void clear();

  typedef typename pvector<Element>::size_type size_type;
  INLINE size_type size() const;
  INLINE const Element &get_element(size_type n) const;
  EXTENSION(const Element &__getitem__(size_type n) const);
  EXTENSION(PyObject *get_data() const);
  EXTENSION(PyObject *get_subdata(size_type n, size_type count) const);
  INLINE int get_ref_count() const;
  INLINE int get_node_ref_count() const;

  INLINE size_t count(const Element &) const;

#ifdef HAVE_PYTHON
  EXTENSION(int __getbuffer__(PyObject *self, Py_buffer *view, int flags) const);
  EXTENSION(void __releasebuffer__(PyObject *self, Py_buffer *view) const);
#endif

#else  // CPPPARSER
  // This is the actual, complete interface.
  typedef typename PointerToArrayBase<Element>::To To;
  typedef typename pvector<Element>::value_type value_type;
  typedef typename pvector<Element>::const_reference reference;
  typedef typename pvector<Element>::const_reference const_reference;
  typedef typename pvector<Element>::const_iterator iterator;
  typedef typename pvector<Element>::const_iterator const_iterator;
#if defined(WIN32_VC) || defined(WIN64_VC)
  // VC++ seems to break the const_reverse_iterator definition somehow.
  typedef typename pvector<Element>::reverse_iterator reverse_iterator;
#else
  typedef typename pvector<Element>::const_reverse_iterator reverse_iterator;
#endif
  typedef typename pvector<Element>::const_reverse_iterator const_reverse_iterator;
  typedef typename pvector<Element>::difference_type difference_type;
  typedef typename pvector<Element>::size_type size_type;

  INLINE ConstPointerToArray(const Element *begin, const Element *end, TypeHandle type_handle = get_type_handle(Element));
  INLINE ConstPointerToArray(const PointerToArray<Element> &copy);
  INLINE ConstPointerToArray(const ConstPointerToArray<Element> &copy);
  INLINE ConstPointerToArray(PointerToArray<Element> &&from) noexcept;
  INLINE ConstPointerToArray(ConstPointerToArray<Element> &&from) noexcept;
  INLINE explicit ConstPointerToArray(pvector<Element> &&from, TypeHandle type_handle = get_type_handle(Element));

  // Duplicating the interface of vector.

  INLINE iterator begin() const;
  INLINE iterator end() const;
  INLINE typename ConstPointerToArray<Element>::reverse_iterator rbegin() const;
  INLINE typename ConstPointerToArray<Element>::reverse_iterator rend() const;

  // Equality and comparison operators are pointerwise for PointerToArrays,
  // not elementwise as in vector.

  INLINE size_type size() const;
  INLINE size_type max_size() const;
  INLINE bool empty() const;

  INLINE void clear();

  // Functions specific to vectors.
  INLINE size_type capacity() const;
  INLINE reference front() const;
  INLINE reference back() const;

#if !defined(WIN32_VC) && !defined(WIN64_VC)
  INLINE reference operator [](size_type n) const;
  INLINE reference operator [](int n) const;
#endif

  INLINE operator const Element *() const;
  INLINE const Element *p() const;
  INLINE const pvector<Element> &v() const;
  INLINE const ReferenceCountedVector<Element> *v0() const;
  INLINE PointerToArray<Element> cast_non_const() const;

  // Methods to help out Python and other high-level languages.
  INLINE const Element &get_element(size_type n) const;
  INLINE std::string get_data() const;
  INLINE std::string get_subdata(size_type n, size_type count) const;

  INLINE int get_ref_count() const;
  INLINE void ref() const;
  INLINE bool unref() const;

  INLINE int get_node_ref_count() const;
  INLINE void node_ref() const;
  INLINE bool node_unref() const;

  INLINE size_t count(const Element &) const;

#endif  // CPPPARSER

public:
  // Reassignment is by pointer, not memberwise as with a vector.
  INLINE ConstPointerToArray<Element> &
  operator = (ReferenceCountedVector<Element> *ptr);
  INLINE ConstPointerToArray<Element> &
  operator = (const PointerToArray<Element> &copy);
  INLINE ConstPointerToArray<Element> &
  operator = (const ConstPointerToArray<Element> &copy);
  INLINE ConstPointerToArray<Element> &
  operator = (PointerToArray<Element> &&from) noexcept;
  INLINE ConstPointerToArray<Element> &
  operator = (ConstPointerToArray<Element> &&from) noexcept;

private:
  TypeHandle _type_handle;

  // This static empty array is kept around just so we can return something
  // meangful when begin() or end() is called and we have a NULL pointer.  It
  // might not be shared properly between different .so's, since it's a static
  // member of a template class, but we don't really care.
  static pvector<Element> _empty_array;

  friend class PointerToArray<Element>;
};

// And the brevity macros.
#define PTA(type) PointerToArray< type >
#define CPTA(type) ConstPointerToArray< type >

#include "pointerToArray.I"

#endif  // HAVE_POINTERTOARRAY_H
