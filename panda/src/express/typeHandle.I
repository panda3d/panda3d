// Filename: typeHandle.I
// Created by:  drose (22Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "config_express.h"

#if defined(_DEBUG) && defined(_WIN32)
#include <windows.h>
#endif

// In general, we use the express_cat->info() syntax in this file
// (instead of express_cat.info()), because much of this work is done at
// static init time, and we must use the arrow syntax to force
// initialization of the express_cat category.

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::Constructor
//       Access: Public
//  Description: The default constructor must do nothing, because we
//               can't guarantee ordering of static initializers.  If
//               the constructor tried to initialize its value, it
//               might happen after the value had already been set
//               previously by another static initializer!
////////////////////////////////////////////////////////////////////
INLINE TypeHandle::
TypeHandle() { 
}

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TypeHandle::
TypeHandle(const TypeHandle &copy) : _index(copy._index) { 
}
 
////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TypeHandle::
operator == (const TypeHandle &other) const {
  return (_index == other._index);
}

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TypeHandle::
operator != (const TypeHandle &other) const {
  return (_index != other._index);
}

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::Ordering Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TypeHandle::
operator < (const TypeHandle &other) const {
  return (_index < other._index);
}

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::get_name
//       Access: Public
//  Description: Returns the name of the type.
//
//               The "object" pointer is an optional pointer to the
//               TypedObject class that owns this TypeHandle.  It is
//               only used in case the TypeHandle is inadvertantly
//               undefined.
////////////////////////////////////////////////////////////////////
INLINE string TypeHandle::
get_name(TypedObject *object) const {
  if ((*this) == TypeHandle::none()) {
    return "none";
  } else {
    return TypeRegistry::ptr()->get_name(*this, object);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::is_derived_from
//       Access: Public
//  Description: Returns true if this type is derived from the
//               indicated type, false otherwise.
//
//               The "object" pointer is an optional pointer to the
//               TypedObject class that owns this TypeHandle.  It is
//               only used in case the TypeHandle is inadvertantly
//               undefined.
////////////////////////////////////////////////////////////////////
INLINE bool TypeHandle::
is_derived_from(TypeHandle parent, TypedObject *object) const {
  return TypeRegistry::ptr()->is_derived_from(*this, parent, object);
}

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::get_num_parent_classes
//       Access: Public
//  Description: Returns the number of parent classes that this
//               type is known to have.  This may then be used to
//               index into get_parent_class().  The result will be 0
//               if this class does not inherit from any other
//               classes, 1 if normal, single inheritance is in
//               effect, or greater than one if multiple inheritance
//               is in effect.
//
//               The "object" pointer is an optional pointer to the
//               TypedObject class that owns this TypeHandle.  It is
//               only used in case the TypeHandle is inadvertantly
//               undefined.
////////////////////////////////////////////////////////////////////
INLINE int TypeHandle::
get_num_parent_classes(TypedObject *object) const {
  return TypeRegistry::ptr()->get_num_parent_classes(*this, object);
}

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::get_num_parent_classes
//       Access: Public
//  Description: Returns the nth parent class of this type.  The index
//               should be in the range 0 <= index <
//               get_num_parent_classes().
////////////////////////////////////////////////////////////////////
INLINE TypeHandle TypeHandle::
get_parent_class(int index) const {
  return TypeRegistry::ptr()->get_parent_class(*this, index);
}

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::get_num_child_classes
//       Access: Public
//  Description: Returns the number of child classes that this
//               type is known to have.  This may then be used to
//               index into get_child_class().
//
//               The "object" pointer is an optional pointer to the
//               TypedObject class that owns this TypeHandle.  It is
//               only used in case the TypeHandle is inadvertantly
//               undefined.
////////////////////////////////////////////////////////////////////
INLINE int TypeHandle::
get_num_child_classes(TypedObject *object) const {
  return TypeRegistry::ptr()->get_num_child_classes(*this, object);
}

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::get_num_child_classes
//       Access: Public
//  Description: Returns the nth child class of this type.  The index
//               should be in the range 0 <= index <
//               get_num_child_classes().
////////////////////////////////////////////////////////////////////
INLINE TypeHandle TypeHandle::
get_child_class(int index) const {
  return TypeRegistry::ptr()->get_child_class(*this, index);
}

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::get_parent_towards
//       Access: Public
//  Description: Returns the parent class that is in a direct line of
//               inheritance to the indicated ancestor class.  This is
//               useful in the presence of multiple inheritance to try
//               to determine what properties an unknown type may
//               have.
//
//               The return value is TypeHandle::none() if the type
//               does not inherit from the ancestor.  If ancestor is
//               the same as this type, the return value is this type.
//
//               The "object" pointer is an optional pointer to the
//               TypedObject class that owns this TypeHandle.  It is
//               only used in case the TypeHandle is inadvertantly
//               undefined.
////////////////////////////////////////////////////////////////////
INLINE TypeHandle TypeHandle::
get_parent_towards(TypeHandle ancestor, TypedObject *object) const {
  return TypeRegistry::ptr()->get_parent_towards(*this, ancestor, object);
}

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::get_index
//       Access: Public
//  Description: Returns the integer index associated with this
//               TypeHandle. Each different TypeHandle will have a
//               different index.  However, you probably shouldn't be
//               using this method; you should just treat the
//               TypeHandles as opaque classes.  This is provided for
//               the convenience of non-C++ scripting languages to
//               build a hashtable of TypeHandles.
////////////////////////////////////////////////////////////////////
INLINE int TypeHandle::
get_index() const {
  return _index;
}

////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::none
//       Access: Public, Static
//  Description: Returns a special zero-valued TypeHandle that is used
//               to indicate no type.
////////////////////////////////////////////////////////////////////
INLINE TypeHandle TypeHandle::
none() {
  return _none;
}


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
register_type(TypeHandle &type_handle, const string &name) {
  TypeRegistry::ptr()->register_type(type_handle, name);
}
INLINE void
register_type(TypeHandle &type_handle, const string &name,
	      TypeHandle parent1) {
  if (TypeRegistry::ptr()->register_type(type_handle, name)) {
    TypeRegistry::ptr()->record_derivation(type_handle, parent1);
  }
}
INLINE void
register_type(TypeHandle &type_handle, const string &name,
	      TypeHandle parent1, TypeHandle parent2) {
  if (TypeRegistry::ptr()->register_type(type_handle, name)) {
    TypeRegistry::ptr()->record_derivation(type_handle, parent1);
    TypeRegistry::ptr()->record_derivation(type_handle, parent2);
  }
}
INLINE void
register_type(TypeHandle &type_handle, const string &name,
	      TypeHandle parent1, TypeHandle parent2,
	      TypeHandle parent3) {
  if (TypeRegistry::ptr()->register_type(type_handle, name)) {
    TypeRegistry::ptr()->record_derivation(type_handle, parent1);
    TypeRegistry::ptr()->record_derivation(type_handle, parent2);
    TypeRegistry::ptr()->record_derivation(type_handle, parent3);
  }
}
INLINE void
register_type(TypeHandle &type_handle, const string &name,
	      TypeHandle parent1, TypeHandle parent2,
	      TypeHandle parent3, TypeHandle parent4) {
  if (TypeRegistry::ptr()->register_type(type_handle, name)) {
    TypeRegistry::ptr()->record_derivation(type_handle, parent1);
    TypeRegistry::ptr()->record_derivation(type_handle, parent2);
    TypeRegistry::ptr()->record_derivation(type_handle, parent3);
    TypeRegistry::ptr()->record_derivation(type_handle, parent4);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: register_dynamic_type
//  Description: This is essentially similar to register_type(),
//               except that it doesn't store a reference to any
//               TypeHandle passed in and it therefore doesn't
//               complain if the type is registered more than once to
//               different TypeHandle reference.
////////////////////////////////////////////////////////////////////
INLINE TypeHandle
register_dynamic_type(const string &name) {
  return TypeRegistry::ptr()->register_dynamic_type(name);
}
INLINE TypeHandle
register_dynamic_type(const string &name, TypeHandle parent1) {
  TypeHandle type_handle = 
    TypeRegistry::ptr()->register_dynamic_type(name);
  TypeRegistry::ptr()->record_derivation(type_handle, parent1);
  return type_handle;
}
INLINE TypeHandle
register_dynamic_type(const string &name,
		      TypeHandle parent1, TypeHandle parent2) {
  TypeHandle type_handle = 
    TypeRegistry::ptr()->register_dynamic_type(name);
  TypeRegistry::ptr()->record_derivation(type_handle, parent1);
  TypeRegistry::ptr()->record_derivation(type_handle, parent2);
  return type_handle;
}
INLINE TypeHandle
register_dynamic_type(const string &name,
		      TypeHandle parent1, TypeHandle parent2,
		      TypeHandle parent3) {
  TypeHandle type_handle = 
    TypeRegistry::ptr()->register_dynamic_type(name);
  TypeRegistry::ptr()->record_derivation(type_handle, parent1);
  TypeRegistry::ptr()->record_derivation(type_handle, parent2);
  TypeRegistry::ptr()->record_derivation(type_handle, parent3);
  return type_handle;
}
INLINE TypeHandle
register_dynamic_type(const string &name,
		      TypeHandle parent1, TypeHandle parent2,
		      TypeHandle parent3, TypeHandle parent4) {
  TypeHandle type_handle = 
    TypeRegistry::ptr()->register_dynamic_type(name);
  TypeRegistry::ptr()->record_derivation(type_handle, parent1);
  TypeRegistry::ptr()->record_derivation(type_handle, parent2);
  TypeRegistry::ptr()->record_derivation(type_handle, parent3);
  TypeRegistry::ptr()->record_derivation(type_handle, parent4);
  return type_handle;
}


////////////////////////////////////////////////////////////////////
//     Function: TypedObject::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TypedObject::
TypedObject() { 
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TypedObject::
TypedObject(const TypedObject &) { 
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TypedObject::
operator = (const TypedObject &) {
} 

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::is_of_type
//       Access: Public
//  Description: Returns true if the current object is or derives from
//               the indicated type.
////////////////////////////////////////////////////////////////////
INLINE bool TypedObject::
is_of_type(TypeHandle handle) const {
  return get_type().is_derived_from(handle, (TypedObject *)this);
}

////////////////////////////////////////////////////////////////////
//     Function: TypedObject::is_exact_type
//       Access: Public
//  Description: Returns true if the current object is the indicated
//               type exactly.
////////////////////////////////////////////////////////////////////
INLINE bool TypedObject::
is_exact_type(TypeHandle handle) const {
#ifndef NDEBUG
  // Call get_name() to force the type to look itself up if necessary.
  get_type().get_name((TypedObject *)this);
#endif
  return get_type() == handle;
}


////////////////////////////////////////////////////////////////////
//     Function: _dcast_get_typehandle
//  Description: Returns the TypeHandle associated with the type of
//               the parameter, if it can be determined.  This is a
//               support function for _dcast, below.
////////////////////////////////////////////////////////////////////
template<class WantType>
INLINE TypeHandle 
_dcast_get_typehandle(WantType *) {
  TypeHandle handle = WantType::get_class_type();
  if (handle == TypeHandle::none()) {
    // This type handle is unregistered.  Oops!
    WantType::init_type();
    handle = WantType::get_class_type();
    express_cat->warning()
      << "Type " << handle << " was unregistered!\n";
  }
  return handle;
}


////////////////////////////////////////////////////////////////////
//     Function: _dcast
//  Description: The implementation of the DCAST macro, this checks
//               the actual type of the pointer before performing a
//               downcast operation.  In NDEBUG mode, it simply
//               downcasts.
//
//               This flavor of _dcast works on non-const pointers.
////////////////////////////////////////////////////////////////////
template<class WantType>
INLINE WantType *
_dcast(WantType *, TypedObject *ptr) {
#ifndef NDEBUG
  TypeHandle want_handle = _dcast_get_typehandle((WantType *)0);
 #if defined(_DEBUG) && defined(_WIN32)
  if ((ptr == (TypedObject *)NULL) || IsBadWritePtr(ptr,sizeof(TypedObject))) { 
 #else
  if (ptr == (TypedObject *)NULL) {
 #endif
    express_cat->warning()
      << "Attempt to cast NULL pointer to " << want_handle << "\n";
    return (WantType *)NULL;
  }
  if (!ptr->is_of_type(want_handle)) {
    express_cat->error()
      << "Attempt to cast pointer from " << ptr->get_type()
      << " to " << want_handle << "\n";
    if (ptr->get_type() == TypedObject::get_class_type()) {
      express_cat->error(false)
	<< "Perhaps pointer was inadvertently deleted?\n";
    }
    return (WantType *)NULL;
  }
#endif
  return (WantType *)ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: _dcast
//  Description: The implementation of the DCAST macro, this checks
//               the actual type of the pointer before performing a
//               downcast operation.  In NDEBUG mode, it simply
//               downcasts.
//
//               This flavor of _dcast works on const pointers.
////////////////////////////////////////////////////////////////////
template<class WantType>
INLINE const WantType *
_dcast(WantType *, const TypedObject *ptr) {
#ifndef NDEBUG
  TypeHandle want_handle = _dcast_get_typehandle((WantType *)0);
  if (ptr == (const TypedObject *)NULL) {
    express_cat->warning()
      << "Attempt to cast NULL pointer to " << want_handle << "\n";
    return (const WantType *)NULL;
  }
  if (!ptr->is_of_type(want_handle)) {
    express_cat->error()
      << "Attempt to cast pointer from " << ptr->get_type()
      << " to " << want_handle << "\n";
    if (ptr->get_type() == TypedObject::get_class_type()) {
      express_cat->error(false)
	<< "Perhaps pointer was inadvertently deleted?\n";
    }
    return (const WantType *)NULL;
  }
#endif
  return (const WantType *)ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: _dcast_ref
//  Description: Similar to the above, with a pointer reference as the
//               first parameter.  Just for fiddly compiler reasons;
//               the reference isn't used.
////////////////////////////////////////////////////////////////////
template<class WantType>
INLINE WantType *
_dcast_ref(WantType *&, TypedObject *ptr) {
  return _dcast((WantType *)NULL, ptr);
}

template<class WantType>
INLINE const WantType *
_dcast_ref(WantType *&, const TypedObject *ptr) {
  return _dcast((WantType *)NULL, ptr);
}
