// Filename: qpgeomMunger.I
// Created by:  drose (10Mar05)
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


////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::is_registered
//       Access: Published
//  Description: Returns true if this munger has been registered,
//               false if it has not.  It may not be used for a Geom
//               until it has been registered, but once registered, it
//               may no longer be modified.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomMunger::
is_registered() const {
  return _is_registered;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::register_munger
//       Access: Published, Static
//  Description: Adds the indicated munger to the registry, if there
//               is not an equivalent munger already there; in either
//               case, returns the pointer to the equivalent munger
//               now in the registry.
//
//               This must be called before a munger may be used in a
//               Geom.  After this call, you should discard the
//               original pointer you passed in (which may or may not
//               now be invalid) and let its reference count decrement
//               normally; you should use only the returned value from
//               this point on.
////////////////////////////////////////////////////////////////////
INLINE CPT(qpGeomMunger) qpGeomMunger::
register_munger(qpGeomMunger *munger) {
  return get_registry()->register_munger(munger);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::compare_to
//       Access: Public
//  Description: Provides an arbitrary ordering among all unique
//               GeomMungers, so we can store the essentially
//               different ones in a big set and throw away the rest.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomMunger::
compare_to(const qpGeomMunger &other) const {
  // First, we compare the types; if they are of different types then
  // they sort differently.
  TypeHandle type = get_type();
  TypeHandle other_type = other.get_type();
  if (type != other_type) {
    return type.get_index() - other_type.get_index();
  }

  // We only call compare_to_impl() if they have the same type.
  return compare_to_impl(&other);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::munge_data
//       Access: Public
//  Description: Given a source GeomVertexData, converts it if
//               necessary to the appropriate data for rendering.
////////////////////////////////////////////////////////////////////
INLINE CPT(qpGeomVertexData) qpGeomMunger::
munge_data(const qpGeomVertexData *data) const {
  // We cast away the const pointer, because do_munge_data() needs to
  // update caches and stuff, but we trust it not to change any
  // user-definable parameters.
  return ((qpGeomMunger *)this)->do_munge_data(data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::munge_format
//       Access: Public
//  Description: Given a source GeomVertexFormat, converts it if
//               necessary to the appropriate format for rendering.
////////////////////////////////////////////////////////////////////
INLINE CPT(qpGeomVertexFormat) qpGeomMunger::
munge_format(const qpGeomVertexFormat *format) const {
  // We cast away the const pointer, because do_munge_format() needs
  // to update caches and stuff, but we trust it not to change any
  // user-definable parameters.
  return ((qpGeomMunger *)this)->do_munge_format(format);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::get_registry
//       Access: Private
//  Description: Returns the global registry object.
////////////////////////////////////////////////////////////////////
INLINE qpGeomMunger::Registry *qpGeomMunger::
get_registry() {
  if (_registry == (Registry *)NULL) {
    make_registry();
  }
  return _registry;
}
