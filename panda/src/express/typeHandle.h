// Filename: typeHandle.h
// Created by:  drose (23Oct98)
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

#ifndef TYPEHANDLE_H
#define TYPEHANDLE_H

#include <pandabase.h>

#include <notify.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <stdlib.h>

// The following illustrates the convention for declaring a type that
// uses TypeHandle.  In this example, ThisThingie inherits from
// TypedObject, which automatically supplies some type-differentiation
// functions at the cost of one virtual function, get_type(); however,
// this inheritance is optional, and may be omitted to avoid the
// virtual function pointer overhead.  (If you do use TypedObject, be
// sure to consider whether your destructor should also be virtual.)

//
// class ThatThingie : public SimpleTypedObject {
// public:
//   static TypeHandle get_class_type() {
//     return _type_handle;
//   }
//   static void init_type() {
//     register_type(_type_handle, "ThatThingie");
//   }
//
// private:
//   static TypeHandle _type_handle;
// };
//
// class ThisThingie : public ThatThingie, publid TypedObject {
// public:
//   static TypeHandle get_class_type() {
//     return _type_handle;
//   }
//   static void init_type() {
//     ThatThingie::init_type();
//     TypedObject::init_type();
//     register_type(_type_handle, "ThisThingie",
//                  ThatThingie::get_class_type(),
//                  TypedObject::get_class_type());
//   }
//   virtual TypeHandle get_type() const {
//     return get_class_type();
//   }
//
// private:
//   static TypeHandle _type_handle;
// };
//

class TypedObject;

////////////////////////////////////////////////////////////////////
//       Class : TypeHandle
// Description : TypeHandle is the identifier used to differentiate
//               C++ class types.  Any C++ classes that inherit from
//               some base class, and must be differentiated at run
//               time, should store a static TypeHandle object that
//               can be queried through a static member function
//               named get_class_type().  Most of the time, it is also
//               desirable to inherit from TypedObject (defined
//               below), which provides some virtual functions to
//               return the TypeHandle for a particular instance.
//
//               At its essence, a TypeHandle is simply a unique
//               identifier that is assigned by the TypeRegistry.  The
//               TypeRegistry stores a tree of TypeHandles, so that
//               ancestry of a particular type may be queried, and the
//               type name may be retrieved for run-time display.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS TypeHandle {
PUBLISHED:
  INLINE TypeHandle();
  INLINE TypeHandle(const TypeHandle &copy);

  INLINE bool operator == (const TypeHandle &other) const;
  INLINE bool operator != (const TypeHandle &other) const;
  INLINE bool operator < (const TypeHandle &other) const;

  INLINE string get_name(TypedObject *object = (TypedObject *)NULL) const;
  INLINE bool is_derived_from(TypeHandle parent,
                              TypedObject *object = (TypedObject *)NULL) const;

  INLINE int get_num_parent_classes(TypedObject *object = (TypedObject *)NULL) const;
  INLINE TypeHandle get_parent_class(int index) const;

  INLINE int get_num_child_classes(TypedObject *object = (TypedObject *)NULL) const;
  INLINE TypeHandle get_child_class(int index) const;

  INLINE TypeHandle get_parent_towards(TypeHandle ancestor,
                                       TypedObject *object = (TypedObject *)NULL) const;

  INLINE int get_index() const;

  INLINE static TypeHandle none();

private:
  int _index;
  static TypeHandle _none;

friend class TypeRegistry;
};


// It's handy to be able to output a TypeHandle directly, and see the
// type name.
INLINE ostream &operator << (ostream &out, TypeHandle type) {
  return out << type.get_name();
}

////////////////////////////////////////////////////////////////////
//       Class : TypeRegistry
// Description : The TypeRegistry class maintains all the assigned
//               TypeHandles in a given system.  There should be only
//               one TypeRegistry class during the lifetime of the
//               application.  It will be created on the local heap
//               initially, and it should be migrated to shared memory
//               as soon as shared memory becomes available.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS TypeRegistry {
protected:
  class RegistryNode {
  public:
    RegistryNode(TypeHandle handle, const string &name, TypeHandle &ref);
    bool is_derived_from(TypeHandle type) const;
    TypeHandle get_parent_towards(TypeHandle type) const;

    TypeHandle _handle;
    string _name;
    TypeHandle &_ref;
    typedef vector<RegistryNode *> Classes;
    Classes _parent_classes;
    Classes _child_classes;
  };

public:
  // User code shouldn't generally need to call
  // TypeRegistry::register_type() or record_derivation() directly;
  // instead, use the register_type convenience function, defined
  // below.
  bool register_type(TypeHandle &type_handle, const string &name);
  TypeHandle register_dynamic_type(const string &name);

  void record_derivation(TypeHandle child, TypeHandle parent);
  void record_alternate_name(TypeHandle type, const string &name);

PUBLISHED:
  TypeHandle find_type(const string &name) const;

  string get_name(TypeHandle type, TypedObject *object) const;
  bool is_derived_from(TypeHandle child, TypeHandle parent,
                       TypedObject *child_object) const;

  int get_num_root_classes();
  TypeHandle get_root_class(int n);

  int get_num_parent_classes(TypeHandle child,
                             TypedObject *child_object) const;
  TypeHandle get_parent_class(TypeHandle child, int index) const;

  int get_num_child_classes(TypeHandle child,
                            TypedObject *child_object) const;
  TypeHandle get_child_class(TypeHandle child, int index) const;

  TypeHandle get_parent_towards(TypeHandle child, TypeHandle ancestor,
                                TypedObject *child_object) const;

  static void reregister_types();

  void write(ostream &out) const;

  // ptr() returns the pointer to the global TypeRegistry object.
  static TypeRegistry *ptr();

private:
  // The TypeRegistry class should never be constructed by user code.
  // There is only one in the universe, and it constructs itself!
  TypeRegistry();

  static void init_global_pointer();

  RegistryNode *look_up(TypeHandle type, TypedObject *object) const;

  void freshen_root_classes();
  void write_node(ostream &out, int indent_level,
                  const RegistryNode *node) const;

  typedef vector<RegistryNode *> HandleRegistry;
  HandleRegistry _handle_registry;

  typedef map<string, RegistryNode *> NameRegistry;
  NameRegistry _name_registry;

  typedef vector<RegistryNode *> RootClasses;
  bool _root_classes_fresh;
  RootClasses _root_classes;

  static TypeRegistry *_global_pointer;
};


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
extern TypeHandle EXPCL_PANDAEXPRESS long_type_handle;
extern TypeHandle EXPCL_PANDAEXPRESS int_type_handle;
extern TypeHandle EXPCL_PANDAEXPRESS short_type_handle;
extern TypeHandle EXPCL_PANDAEXPRESS char_type_handle;
extern TypeHandle EXPCL_PANDAEXPRESS bool_type_handle;
extern TypeHandle EXPCL_PANDAEXPRESS double_type_handle;
extern TypeHandle EXPCL_PANDAEXPRESS float_type_handle;

extern TypeHandle long_p_type_handle;
extern TypeHandle int_p_type_handle;
extern TypeHandle short_p_type_handle;
extern TypeHandle char_p_type_handle;
extern TypeHandle bool_p_type_handle;
extern TypeHandle double_p_type_handle;
extern TypeHandle float_p_type_handle;
extern TypeHandle void_p_type_handle;

void EXPCL_PANDAEXPRESS init_system_type_handles();

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

#include "typeHandle.I"

#endif

