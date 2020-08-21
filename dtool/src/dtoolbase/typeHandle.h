/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typeHandle.h
 * @author drose
 * @date 1998-10-23
 */

#ifndef TYPEHANDLE_H
#define TYPEHANDLE_H

#include "dtoolbase.h"

#include <set>

/**
 * The following illustrates the convention for declaring a type that uses
 * TypeHandle.  In this example, ThisThingie inherits from TypedObject, which
 * automatically supplies some type-differentiation functions at the cost of
 * one virtual function, get_type(); however, this inheritance is optional,
 * and may be omitted to avoid the virtual function pointer overhead.  (If you
 * do use TypedObject, be sure to consider whether your destructor should also
 * be virtual.)
 *
 * @code
 * class ThatThingie : public SimpleTypedObject {
 * public:
 *   static TypeHandle get_class_type() {
 *     return _type_handle;
 *   }
 *   static void init_type() {
 *     register_type(_type_handle, "ThatThingie");
 *   }
 *
 * private:
 *   static TypeHandle _type_handle;
 * };
 *
 * class ThisThingie : public ThatThingie, publid TypedObject {
 * public:
 *   static TypeHandle get_class_type() {
 *     return _type_handle;
 *   }
 *   static void init_type() {
 *     ThatThingie::init_type();
 *     TypedObject::init_type();
 *     register_type(_type_handle, "ThisThingie",
 *                  ThatThingie::get_class_type(),
 *                  TypedObject::get_class_type());
 *   }
 *   virtual TypeHandle get_type() const {
 *     return get_class_type();
 *   }
 *
 * private:
 *   static TypeHandle _type_handle;
 * };
 * @endcode
 */

class TypedObject;

/**
 * TypeHandle is the identifier used to differentiate C++ class types.  Any
 * C++ classes that inherit from some base class, and must be differentiated
 * at run time, should store a static TypeHandle object that can be queried
 * through a static member function named get_class_type().  Most of the time,
 * it is also desirable to inherit from TypedObject, which provides some
 * virtual functions to return the TypeHandle for a particular instance.
 *
 * At its essence, a TypeHandle is simply a unique identifier that is assigned
 * by the TypeRegistry.  The TypeRegistry stores a tree of TypeHandles, so
 * that ancestry of a particular type may be queried, and the type name may be
 * retrieved for run-time display.
 */
class EXPCL_DTOOL_DTOOLBASE TypeHandle final {
PUBLISHED:
  TypeHandle() noexcept = default;

  enum MemoryClass {
    MC_singleton,
    MC_array,
    MC_deleted_chain_active,
    MC_deleted_chain_inactive,

    MC_limit  // Not a real value, just a placeholder for the maximum
              // enum value.
  };

  // The default constructor must do nothing, because we can't guarantee
  // ordering of static initializers.  If the constructor tried to initialize
  // its value, it  might happen after the value had already been set
  // previously by another static initializer!

  EXTENSION(static TypeHandle make(PyTypeObject *classobj));

  INLINE bool operator == (const TypeHandle &other) const;
  INLINE bool operator != (const TypeHandle &other) const;
  INLINE bool operator < (const TypeHandle &other) const;
  INLINE bool operator <= (const TypeHandle &other) const;
  INLINE bool operator > (const TypeHandle &other) const;
  INLINE bool operator >= (const TypeHandle &other) const;
  INLINE int compare_to(const TypeHandle &other) const;
  INLINE size_t get_hash() const;

  INLINE std::string get_name(TypedObject *object = nullptr) const;
  INLINE bool is_derived_from(TypeHandle parent,
                              TypedObject *object = nullptr) const;

  INLINE int get_num_parent_classes(TypedObject *object = nullptr) const;
  INLINE TypeHandle get_parent_class(int index) const;

  INLINE int get_num_child_classes(TypedObject *object = nullptr) const;
  INLINE TypeHandle get_child_class(int index) const;

  INLINE TypeHandle get_parent_towards(TypeHandle ancestor,
                                       TypedObject *object = nullptr) const;

  int get_best_parent_from_Set(const std::set< int > &legal_vals) const;

  size_t get_memory_usage(MemoryClass memory_class) const;
  void inc_memory_usage(MemoryClass memory_class, size_t size);
  void dec_memory_usage(MemoryClass memory_class, size_t size);

  INLINE int get_index() const;
  INLINE void output(std::ostream &out) const;
  constexpr static TypeHandle none() { return TypeHandle(0); }
  INLINE operator bool () const;

  MAKE_PROPERTY(index, get_index);
  MAKE_PROPERTY(name, get_name);
  MAKE_SEQ_PROPERTY(parent_classes, get_num_parent_classes, get_parent_class);
  MAKE_SEQ_PROPERTY(child_classes, get_num_child_classes, get_child_class);

public:
#ifdef HAVE_PYTHON
  PyObject *get_python_type() const;
#endif

  void *allocate_array(size_t size) RETURNS_ALIGNED(MEMORY_HOOK_ALIGNMENT);
  void *reallocate_array(void *ptr, size_t size) RETURNS_ALIGNED(MEMORY_HOOK_ALIGNMENT);
  void deallocate_array(void *ptr);

  constexpr static TypeHandle from_index(int index) { return TypeHandle(index); }

private:
  constexpr TypeHandle(int index);

  int _index;
  friend class TypeRegistry;
};


// It's handy to be able to output a TypeHandle directly, and see the type
// name.
INLINE std::ostream &operator << (std::ostream &out, TypeHandle type) {
  type.output(out);
  return out;
}

EXPCL_DTOOL_DTOOLBASE std::ostream &operator << (std::ostream &out, TypeHandle::MemoryClass mem_class);

// We must include typeRegistry at this point so we can call it from our
// inline functions.  This is a circular include that is strategically placed
// to do no harm.
/* okcircular */
#include "typeRegistry.h"

#include "typeHandle.I"

#endif
