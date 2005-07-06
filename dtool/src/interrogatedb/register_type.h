// Filename: register_type.h
// Created by:  drose (06Aug01)
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

#ifndef REGISTER_TYPE_H
#define REGISTER_TYPE_H

#include "dtoolbase.h"

#include "typeHandle.h"
#include "typeRegistry.h"

////////////////////////////////////////////////////////////////////
//     Function: register_type
//  Description: This inline function is just a convenient way to call
//               TypeRegistry::register_type(), along with zero to four
//               record_derivation()s.  If for some reason you have a
//               class that has more than four base classes (you're
//               insane!), then you will need to call Register() and
//               record_derivation() yourself.
////////////////////////////////////////////////////////////////////
INLINE void
register_type(TypeHandle &type_handle, const string &name);

INLINE void
register_type(TypeHandle &type_handle, const string &name,
              TypeHandle parent1);

INLINE void
register_type(TypeHandle &type_handle, const string &name,
              TypeHandle parent1, TypeHandle parent2);

INLINE void
register_type(TypeHandle &type_handle, const string &name,
              TypeHandle parent1, TypeHandle parent2,
              TypeHandle parent3);

INLINE void
register_type(TypeHandle &type_handle, const string &name,
              TypeHandle parent1, TypeHandle parent2,
              TypeHandle parent3, TypeHandle parent4);


////////////////////////////////////////////////////////////////////
//     Function: register_dynamic_type
//  Description: This is essentially similar to register_type(),
//               except that it doesn't store a reference to any
//               TypeHandle passed in and it therefore doesn't
//               complain if the type is registered more than once to
//               different TypeHandle reference.
////////////////////////////////////////////////////////////////////
INLINE TypeHandle
register_dynamic_type(const string &name);

INLINE TypeHandle
register_dynamic_type(const string &name, TypeHandle parent1);

INLINE TypeHandle
register_dynamic_type(const string &name,
                      TypeHandle parent1, TypeHandle parent2);

INLINE TypeHandle
register_dynamic_type(const string &name,
                      TypeHandle parent1, TypeHandle parent2,
                      TypeHandle parent3);

INLINE TypeHandle
register_dynamic_type(const string &name,
                      TypeHandle parent1, TypeHandle parent2,
                      TypeHandle parent3, TypeHandle parent4);


// A few system-wide TypeHandles are defined for some basic types.
extern TypeHandle EXPCL_DTOOLCONFIG long_type_handle;
extern TypeHandle EXPCL_DTOOLCONFIG int_type_handle;
extern TypeHandle EXPCL_DTOOLCONFIG short_type_handle;
extern TypeHandle EXPCL_DTOOLCONFIG char_type_handle;
extern TypeHandle EXPCL_DTOOLCONFIG bool_type_handle;
extern TypeHandle EXPCL_DTOOLCONFIG double_type_handle;
extern TypeHandle EXPCL_DTOOLCONFIG float_type_handle;

extern TypeHandle long_p_type_handle;
extern TypeHandle int_p_type_handle;
extern TypeHandle short_p_type_handle;
extern TypeHandle char_p_type_handle;
extern TypeHandle bool_p_type_handle;
extern TypeHandle double_p_type_handle;
extern TypeHandle float_p_type_handle;
extern TypeHandle void_p_type_handle;

void EXPCL_DTOOLCONFIG init_system_type_handles();

// The following template function and its specializations will return
// a TypeHandle for any type in the world, from a pointer to that
// type.

template<class T>
INLINE TypeHandle _get_type_handle(const T *) {
  return T::get_class_type();
}

template<>
INLINE TypeHandle _get_type_handle(const long *) {
  return long_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const int *) {
  return int_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const short *) {
  return short_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const char *) {
  return char_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const bool *) {
  return bool_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const double *) {
  return double_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const float *) {
  return float_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const long * const *) {
  return long_p_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const int * const *) {
  return int_p_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const short * const *) {
  return short_p_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const char * const *) {
  return char_p_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const bool * const *) {
  return bool_p_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const double * const *) {
  return double_p_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const float * const *) {
  return float_p_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const void * const *) {
  return void_p_type_handle;
}


// The macro get_type_handle(type) is defined to make getting the type
// handle associated with a particular type a bit cleaner.
#define get_type_handle(type) _get_type_handle((const type *)0)


// The following template function and its specializations can be used
// to call init() on any unknown type.  Handy for use within a
// template class.

template<class T>
INLINE void _do_init_type(const T *) {
  T::init_type();
}

template<>
INLINE void _do_init_type(const long *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const int *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const short *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const char *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const bool *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const double *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const float *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const long * const *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const int * const *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const short * const *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const char * const *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const bool * const *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const double * const *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const float * const *) {
  init_system_type_handles();
}

template<>
INLINE void _do_init_type(const void * const *) {
  init_system_type_handles();
}

#define do_init_type(type) _do_init_type((const type *)0)

#include "register_type.I"

#endif
