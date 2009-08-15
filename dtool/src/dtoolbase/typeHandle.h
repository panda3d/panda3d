// Filename: typeHandle.h
// Created by:  drose (23Oct98)
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

#ifndef TYPEHANDLE_H
#define TYPEHANDLE_H

#include "dtoolbase.h"
#include "typeRegistry.h"

#include <set>

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

#ifdef HAVE_PYTHON
#include "Python.h"
#endif

////////////////////////////////////////////////////////////////////
//       Class : TypeHandle
// Description : TypeHandle is the identifier used to differentiate
//               C++ class types.  Any C++ classes that inherit from
//               some base class, and must be differentiated at run
//               time, should store a static TypeHandle object that
//               can be queried through a static member function
//               named get_class_type().  Most of the time, it is also
//               desirable to inherit from TypedObject, which provides
//               some virtual functions to return the TypeHandle for a
//               particular instance.
//
//               At its essence, a TypeHandle is simply a unique
//               identifier that is assigned by the TypeRegistry.  The
//               TypeRegistry stores a tree of TypeHandles, so that
//               ancestry of a particular type may be queried, and the
//               type name may be retrieved for run-time display.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL TypeHandle {
PUBLISHED:
  enum MemoryClass {
    MC_singleton,
    MC_array,
    MC_deleted_chain_active,
    MC_deleted_chain_inactive,

    MC_limit  // Not a real value, just a placeholder for the maximum
              // enum value.
  };

  INLINE TypeHandle();
  INLINE TypeHandle(const TypeHandle &copy);

#ifdef HAVE_PYTHON
  static PyObject *make(PyObject *classobj);
#endif  // HAVE_PYTHON

  INLINE bool operator == (const TypeHandle &other) const;
  INLINE bool operator != (const TypeHandle &other) const;
  INLINE bool operator < (const TypeHandle &other) const;
  INLINE bool operator <= (const TypeHandle &other) const;
  INLINE bool operator > (const TypeHandle &other) const;
  INLINE bool operator >= (const TypeHandle &other) const;
  INLINE int compare_to(const TypeHandle &other) const;
  INLINE size_t get_hash() const;

  INLINE string get_name(TypedObject *object = (TypedObject *)NULL) const;
  INLINE bool is_derived_from(TypeHandle parent,
                              TypedObject *object = (TypedObject *)NULL) const;

  INLINE int get_num_parent_classes(TypedObject *object = (TypedObject *)NULL) const;
  INLINE TypeHandle get_parent_class(int index) const;

  INLINE int get_num_child_classes(TypedObject *object = (TypedObject *)NULL) const;
  INLINE TypeHandle get_child_class(int index) const;

  INLINE TypeHandle get_parent_towards(TypeHandle ancestor,
                                       TypedObject *object = (TypedObject *)NULL) const;
  
  INLINE  int get_best_parent_from_Set(const std::set< int > &legal_vals) const;

#ifdef DO_MEMORY_USAGE
  int get_memory_usage(MemoryClass memory_class) const;
  void inc_memory_usage(MemoryClass memory_class, int size);
  void dec_memory_usage(MemoryClass memory_class, int size);
#else
  INLINE int get_memory_usage(MemoryClass) const { return 0; }
  INLINE void inc_memory_usage(MemoryClass, int) { }
  INLINE void dec_memory_usage(MemoryClass, int) { }
#endif  // DO_MEMORY_USAGE

  INLINE int get_index() const;
  INLINE void output(ostream &out) const;
  INLINE static TypeHandle none();

private:
  int _index;
  static TypeHandle _none;

friend class TypeRegistry;
};


// It's handy to be able to output a TypeHandle directly, and see the
// type name.
INLINE ostream &operator << (ostream &out, TypeHandle type) {
  type.output(out);
  return out;
}

EXPCL_DTOOL ostream &operator << (ostream &out, TypeHandle::MemoryClass mem_class);

// We must include typeRegistry at this point so we can call it from
// our inline functions.  This is a circular include that is
// strategically placed to do no harm.
/* okcircular */
#include "typeRegistry.h"

#include "typeHandle.I"

#endif

