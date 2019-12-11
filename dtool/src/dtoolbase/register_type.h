/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file register_type.h
 * @author drose
 * @date 2001-08-06
 */

#ifndef REGISTER_TYPE_H
#define REGISTER_TYPE_H

#include "dtoolbase.h"

#include "typeHandle.h"
#include "typeRegistry.h"

#if defined(HAVE_RTTI) && !defined(__clang__) && !defined(__GNUC__)
#include <typeinfo>
#endif

/**
 * This inline function is just a convenient way to call
 * TypeRegistry::register_type(), along with zero to four
 * record_derivation()s.  If for some reason you have a class that has more
 * than four base classes (you're insane!), then you will need to call
 * Register() and record_derivation() yourself.
 */
INLINE void
register_type(TypeHandle &type_handle, const std::string &name);

INLINE void
register_type(TypeHandle &type_handle, const std::string &name,
              TypeHandle parent1);

INLINE void
register_type(TypeHandle &type_handle, const std::string &name,
              TypeHandle parent1, TypeHandle parent2);

INLINE void
register_type(TypeHandle &type_handle, const std::string &name,
              TypeHandle parent1, TypeHandle parent2,
              TypeHandle parent3);

INLINE void
register_type(TypeHandle &type_handle, const std::string &name,
              TypeHandle parent1, TypeHandle parent2,
              TypeHandle parent3, TypeHandle parent4);


/**
 * This is essentially similar to register_type(), except that it doesn't
 * store a reference to any TypeHandle passed in and it therefore doesn't
 * complain if the type is registered more than once to different TypeHandle
 * reference.
 */
INLINE TypeHandle
register_dynamic_type(const std::string &name);

INLINE TypeHandle
register_dynamic_type(const std::string &name, TypeHandle parent1);

INLINE TypeHandle
register_dynamic_type(const std::string &name,
                      TypeHandle parent1, TypeHandle parent2);

INLINE TypeHandle
register_dynamic_type(const std::string &name,
                      TypeHandle parent1, TypeHandle parent2,
                      TypeHandle parent3);

INLINE TypeHandle
register_dynamic_type(const std::string &name,
                      TypeHandle parent1, TypeHandle parent2,
                      TypeHandle parent3, TypeHandle parent4);


/**
 * This is a helper that returns the type name for any given type.
 */
template<class T>
INLINE std::basic_string<char>
_get_type_name();

// The macro get_type_name(type) is defined to make getting the type name
// associated with a particular type a bit cleaner.
#define get_type_name(type) _get_type_name<type>()


// A few system-wide TypeHandles are defined for some basic types.
extern TypeHandle EXPCL_DTOOL_DTOOLBASE long_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE int_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE uint_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE short_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE ushort_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE char_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE uchar_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE bool_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE double_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE float_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE string_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE wstring_type_handle;

extern TypeHandle long_p_type_handle;
extern TypeHandle int_p_type_handle;
extern TypeHandle short_p_type_handle;
extern TypeHandle char_p_type_handle;
extern TypeHandle bool_p_type_handle;
extern TypeHandle double_p_type_handle;
extern TypeHandle float_p_type_handle;
extern TypeHandle void_p_type_handle;

extern TypeHandle EXPCL_DTOOL_DTOOLBASE pvector_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE ov_set_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE pdeque_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE plist_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE pmap_type_handle;
extern TypeHandle EXPCL_DTOOL_DTOOLBASE pset_type_handle;

void EXPCL_DTOOL_DTOOLBASE init_system_type_handles();

// The following template function and its specializations will return a
// TypeHandle for any type in the world, from a pointer to that type.

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
INLINE TypeHandle _get_type_handle(const unsigned int *) {
  return uint_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const short *) {
  return short_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const unsigned short *) {
  return ushort_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const char *) {
  return char_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const unsigned char *) {
  return uchar_type_handle;
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
INLINE TypeHandle _get_type_handle(const std::string *) {
  return string_type_handle;
}

template<>
INLINE TypeHandle _get_type_handle(const std::wstring *) {
  return wstring_type_handle;
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


// The macro get_type_handle(type) is defined to make getting the type handle
// associated with a particular type a bit cleaner.
#define get_type_handle(type) _get_type_handle((const type *)0)


// The following template function and its specializations can be used to call
// init() on any unknown type.  Handy for use within a template class.

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
