// Filename: pointerToArray_ext.h
// Created by:  rdb (08Feb15)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef POINTERTOARRAY_EXT_H
#define POINTERTOARRAY_EXT_H

#ifndef CPPPARSER

#include "extension.h"
#include "py_panda.h"
#include "pointerToArray.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<PointerToArray>
// Description : This class defines the extension methods for
//               PointerToArray, which are called instead of
//               any C++ methods with the same prototype.
//
//               This is a little bit awkward because of the nested
//               templating, but it does the job.
////////////////////////////////////////////////////////////////////
template<class Element>
class Extension<PointerToArray<Element> > : public ExtensionBase<PointerToArray<Element> > {
public:
  INLINE void __init__(PyObject *self, PyObject *source);

  INLINE const Element &__getitem__(size_t n) const;
  INLINE void __setitem__(size_t n, const Element &value);

#if PY_VERSION_HEX >= 0x02060000
  INLINE int __getbuffer__(PyObject *self, Py_buffer *view, int flags);
  INLINE void __releasebuffer__(PyObject *self, Py_buffer *view) const;
#endif
};

////////////////////////////////////////////////////////////////////
//       Class : Extension<ConstPointerToArray>
// Description : This class defines the extension methods for
//               ConstPointerToArray, which are called instead of
//               any C++ methods with the same prototype.
//
//               This is a little bit awkward because of the nested
//               templating, but it does the job.
////////////////////////////////////////////////////////////////////
template<class Element>
class Extension<ConstPointerToArray<Element> > : public ExtensionBase<ConstPointerToArray<Element> > {
public:
  INLINE void __init__(PyObject *self, PyObject *source);

  INLINE const Element &__getitem__(size_t n) const;

#if PY_VERSION_HEX >= 0x02060000
  INLINE int __getbuffer__(PyObject *self, Py_buffer *view, int flags) const;
  INLINE void __releasebuffer__(PyObject *self, Py_buffer *view) const;
#endif
};

#ifdef _MSC_VER
// Ugh... MSVC needs this because they still don't have a decent linker.
#include "PTA_uchar.h"
#include "PTA_ushort.h"
#include "PTA_float.h"
#include "PTA_double.h"
#include "PTA_int.h"

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

// This macro is used to map a data type to a format code
// as used in the Python 'struct' and 'array' modules.
#define get_format_code(type) _get_format_code((const type *)0)
#define define_format_code(code, type) template<> \
  INLINE const char *_get_format_code(const type *) { \
    return code; \
  }

template<class T>
INLINE const char *_get_format_code(const T *) {
  return NULL;
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

#include "pointerToArray_ext.I"

#endif  // CPPPARSER

#endif  // HAVE_POINTERTOARRAY_EXT_H
