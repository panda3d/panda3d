// Filename: typeRegistry.I
// Created by:  drose (06Aug01)
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


#ifdef NDEBUG
// This function is only inline if NDEBUG is defined.  Otherwise, it
// is non-inline and its definition appears in typeRegistry.cxx.

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::look_up
//       Access: Private
//  Description: Returns the TypeRegistryNode associated with the
//               indicated TypeHandle.
////////////////////////////////////////////////////////////////////
INLINE TypeRegistryNode *TypeRegistry::
look_up(TypeHandle handle, TypedObject *) const {
  return _handle_registry[handle._index];
}
#endif  // NDEBUG


////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::freshen_derivations
//       Access: Private
//  Description: Rebuilds the derivation data structures after some
//               derivation relationship has been modified, so that
//               class relationships can quickly be determined.
////////////////////////////////////////////////////////////////////
INLINE void TypeRegistry::
freshen_derivations() {
  if (!_derivations_fresh) {
    rebuild_derivations();
    _derivations_fresh = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TypeRegistry::is_derived_from
//       Access: Public
//  Description: Returns true if the first type is derived from the
//               second type, false otherwise.
//
//               The "child_object" pointer is an optional pointer to
//               the TypedObject class that owns the child TypeHandle.
//               It is only used in case the TypeHandle is
//               inadvertently undefined.
//
//               This function definition follows the definitions for
//               look_up() and freshen_derivations() just to maximize
//               the chance the the compiler will be able to inline
//               the above functions.  Yeah, a compiler shouldn't
//               care, but there's a big different between "shouldn't"
//               and "doesn't".
////////////////////////////////////////////////////////////////////
INLINE bool TypeRegistry::
is_derived_from(TypeHandle child, TypeHandle base,
                TypedObject *child_object) {
  const TypeRegistryNode *child_node = look_up(child, child_object);
  const TypeRegistryNode *base_node = look_up(base, (TypedObject *)NULL);
  nassertr(child_node != (TypeRegistryNode *)NULL &&
           base_node != (TypeRegistryNode *)NULL, false);
  freshen_derivations();
  return TypeRegistryNode::is_derived_from(child_node, base_node);
}
