// Filename: typedObject.h
// Created by:  drose (11May01)
// 
////////////////////////////////////////////////////////////////////

#ifndef TYPEDOBJECT_H
#define TYPEDOBJECT_H

#include <pandabase.h>

#include "typeHandle.h"


////////////////////////////////////////////////////////////////////
//       Class : TypedObject
// Description : This is an abstract class that all classes which
//               use TypeHandle, and also provide virtual functions to
//               support polymorphism, should inherit from.  Each
//               derived class should define get_type(), which should
//               return the specific type of the derived class.
//               Inheriting from this automatically provides support
//               for is_of_type() and is_exact_type().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS TypedObject {
public:
  INLINE TypedObject();
  INLINE TypedObject(const TypedObject &copy);
  INLINE void operator = (const TypedObject &copy);

PUBLISHED: 
  // A virtual destructor is just a good idea.
  virtual ~TypedObject();

  // Derived classes should override this function to return
  // get_class_type().
  virtual TypeHandle get_type() const=0;

  INLINE int get_type_index() const;
  INLINE bool is_of_type(TypeHandle handle) const;
  INLINE bool is_exact_type(TypeHandle handle) const;

public:
  // Derived classes should override this function to call
  // init_type().  It will only be called in error situations when the
  // type was for some reason not properly initialized.
  virtual TypeHandle force_init_type()=0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "TypedObject");
  }

private:
  static TypeHandle _type_handle;
};

// The DCAST (downcast) macro is defined as a convenience for
// downcasting from some TypedObject pointer (or a PointerTo).  It's
// just a normal C++-style downcast, except it first checks get_type()
// to make sure the downcasting is safe.  If you compile with NDEBUG,
// this check is removed.

// DCAST will return NULL if the downcasting is unsafe.  If you'd
// rather it abort out of the function (ala nassertv/nassertr), then
// see DCAST_INTO_V and DCAST_INTO_R, below.

template<class WantType>
INLINE WantType *_dcast(WantType *, TypedObject *ptr);
template<class WantType>
INLINE const WantType *_dcast(WantType *, const TypedObject *ptr);

// Note: it is important that DCAST not repeat the pointer parameter,
// since many users of DCAST may want to use the result of a function
// as the pointer parameter, and it could be terribly confusing and
// difficult to trace if the function was executed twice.  This
// happened!
#define DCAST(want_type, pointer) _dcast((want_type*)0, pointer)

// DCAST_INTO_V and DCAST_INTO_R are similar in purpose to DCAST,
// except they: (a) automatically assign a variable instead of
// returning the downcasted pointer, and (b) they immediately return
// out of the function if the downcasting fails.  DCAST_INTO_V is for
// use in a void function and returns nothing; DCAST_INTO_R is for use
// in a non-void function and returns the indicated value.

// Both DCAST_INTO_V and DCAST_INTO_R accept as the first parameter a
// variable of type (want_type *) or (const want_type *), instead of
// the name of the type.  This variable will be filled with the new
// pointer.


// _dcast_ref is used to implement DCAST_INTO_V and DCAST_INTO_R.  Its
// difference from _dcast is that it takes a reference to a pointer as
// a first parameter.  The main point of this is to shut up the
// compiler about pointers used before their value is assigned.
template<class WantType>
INLINE WantType *_dcast_ref(WantType *&, TypedObject *ptr);
template<class WantType>
INLINE const WantType *_dcast_ref(WantType *&, const TypedObject *ptr);


#define DCAST_INTO_V(to_pointer, from_pointer) \
  { \
    (to_pointer) = _dcast_ref(to_pointer, from_pointer); \
    nassertv((void *)(to_pointer) != (void *)NULL); \
  }

#define DCAST_INTO_R(to_pointer, from_pointer, return_value) \
  { \
    (to_pointer) = _dcast_ref(to_pointer, from_pointer); \
    nassertr((void *)(to_pointer) != (void *)NULL, return_value); \
  }

#include "typedObject.I"

#endif
