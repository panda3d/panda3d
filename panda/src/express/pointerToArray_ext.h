/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerToArray_ext.h
 * @author rdb
 * @date 2015-02-08
 */

#ifndef POINTERTOARRAY_EXT_H
#define POINTERTOARRAY_EXT_H

#ifndef CPPPARSER

#include "extension.h"
#include "py_panda.h"
#include "pointerToArray.h"
#include "luse.h"

/**
 * This class defines the extension methods for PointerToArray, which are
 * called instead of any C++ methods with the same prototype.
 *
 * This is a little bit awkward because of the nested templating, but it does
 * the job.
 */
template<class Element>
class Extension<PointerToArray<Element> > : public ExtensionBase<PointerToArray<Element> > {
public:
  INLINE void __init__(PyObject *self, PyObject *source);

  INLINE const Element &__getitem__(size_t n) const;
  INLINE void __setitem__(size_t n, const Element &value);

  INLINE PyObject *get_data() const;
  INLINE void set_data(PyObject *data);
  INLINE PyObject *get_subdata(size_t n, size_t count) const;

  INLINE int __getbuffer__(PyObject *self, Py_buffer *view, int flags);
  INLINE void __releasebuffer__(PyObject *self, Py_buffer *view) const;
};

template<>
INLINE int Extension<PointerToArray<LMatrix3f> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags);

template<>
INLINE int Extension<PointerToArray<LMatrix3d> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags);

template<>
INLINE int Extension<PointerToArray<UnalignedLMatrix4f> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags);

template<>
INLINE int Extension<PointerToArray<UnalignedLMatrix4d> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags);

/**
 * This class defines the extension methods for ConstPointerToArray, which are
 * called instead of any C++ methods with the same prototype.
 *
 * This is a little bit awkward because of the nested templating, but it does
 * the job.
 */
template<class Element>
class Extension<ConstPointerToArray<Element> > : public ExtensionBase<ConstPointerToArray<Element> > {
public:
  INLINE const Element &__getitem__(size_t n) const;

  INLINE PyObject *get_data() const;
  INLINE PyObject *get_subdata(size_t n, size_t count) const;

  INLINE int __getbuffer__(PyObject *self, Py_buffer *view, int flags) const;
  INLINE void __releasebuffer__(PyObject *self, Py_buffer *view) const;
};

template<>
INLINE int Extension<ConstPointerToArray<LMatrix3f> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const;

template<>
INLINE int Extension<ConstPointerToArray<LMatrix3d> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const;

template<>
INLINE int Extension<ConstPointerToArray<UnalignedLMatrix4f> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const;

template<>
INLINE int Extension<ConstPointerToArray<UnalignedLMatrix4d> >::
__getbuffer__(PyObject *self, Py_buffer *view, int flags) const;

#ifdef _MSC_VER
// Ugh... MSVC needs this because they still don't have a decent linker.
#include "pta_uchar.h"
#include "pta_ushort.h"
#include "pta_float.h"
#include "pta_double.h"
#include "pta_int.h"

template class EXPORT_THIS Extension<PTA_uchar>;
template class EXPORT_THIS Extension<PTA_ushort>;
template class EXPORT_THIS Extension<PTA_float>;
template class EXPORT_THIS Extension<PTA_double>;
template class EXPORT_THIS Extension<PTA_int>;
template class EXPORT_THIS Extension<CPTA_uchar>;
template class EXPORT_THIS Extension<CPTA_ushort>;
template class EXPORT_THIS Extension<CPTA_float>;
template class EXPORT_THIS Extension<CPTA_double>;
template class EXPORT_THIS Extension<CPTA_int>;
#endif

// This macro is used to map a data type to a format code as used in the
// Python 'struct' and 'array' modules.
#define get_format_code(type) _get_format_code((const type *)0)
#define define_format_code(code, type) template<> \
  INLINE const char *_get_format_code(const type *) { \
    return code; \
  }

template<class T>
INLINE const char *_get_format_code(const T *) {
  return nullptr;
}

define_format_code("c", char);
define_format_code("b", signed char);
define_format_code("B", unsigned char);
define_format_code("h", short);
define_format_code("H", unsigned short);
define_format_code("i", int);
define_format_code("I", unsigned int);
define_format_code("l", long);
define_format_code("L", unsigned long);
define_format_code("q", long long);
define_format_code("Q", unsigned long long);
define_format_code("f", float);
define_format_code("d", double);

define_format_code("2f", LVecBase2f);
define_format_code("2d", LVecBase2d);
define_format_code("2i", LVecBase2i);
define_format_code("3f", LVecBase3f);
define_format_code("3d", LVecBase3d);
define_format_code("3i", LVecBase3i);
define_format_code("4f", UnalignedLVecBase4f);
define_format_code("4d", UnalignedLVecBase4d);
define_format_code("4i", UnalignedLVecBase4i);
// define_format_code("9f", LMatrix3f);
// define_format_code("9d", LMatrix3d);
// define_format_code("16f", UnalignedLMatrix4f);
// define_format_code("16d", UnalignedLMatrix4d);

#include "pointerToArray_ext.I"

#endif  // CPPPARSER

#endif  // HAVE_POINTERTOARRAY_EXT_H
