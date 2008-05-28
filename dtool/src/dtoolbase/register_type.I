// Filename: register_type.I
// Created by:  drose (06Aug01)
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
