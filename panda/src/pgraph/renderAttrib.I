// Filename: renderAttrib.I
// Created by:  drose (21Feb02)
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
//     Function: RenderAttrib::compose
//       Access: Public
//  Description: Returns a new RenderAttrib object that represents the
//               composition of this attrib with the other attrib.  In
//               most cases, this is the same as the other attrib; a
//               compose b produces b.  Some kinds of attributes, like
//               a TextureTransform, for instance, might produce a new
//               result: a compose b produces c.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) RenderAttrib::
compose(const RenderAttrib *other) const {
  return compose_impl(other);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::invert_compose
//       Access: Public
//  Description: Returns a new RenderAttrib object that represents the
//               composition of the inverse of this attrib with the
//               other attrib.  In most cases, this is the same as the
//               other attrib; !a compose b produces b.  Some kinds of
//               attributes, like a TextureTransform, for instance,
//               might produce a new result: !a compose b produces c.
//
//               This is similar to compose() except that the source
//               attrib is inverted first.  This is used to compute
//               the relative attribute for one node as viewed from
//               some other node, which is especially useful for
//               transform-type attributes.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) RenderAttrib::
invert_compose(const RenderAttrib *other) const {
  return invert_compose_impl(other);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::make_default
//       Access: Public
//  Description: Returns a different (or possibly the same)
//               RenderAttrib pointer of the same type as this one
//               that corresponds to whatever the standard default
//               properties for render attributes of this type ought
//               to be.
////////////////////////////////////////////////////////////////////
INLINE CPT(RenderAttrib) RenderAttrib::
make_default() const {
  return return_new(make_default_impl());
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::compare_to
//       Access: Public
//  Description: Provides an arbitrary ordering among all unique
//               RenderAttribs, so we can store the essentially
//               different ones in a big set and throw away the rest.
//
//               This method is not needed outside of the RenderAttrib
//               class because all equivalent RenderAttrib objects are
//               guaranteed to share the same pointer; thus, a pointer
//               comparison is always sufficient.
////////////////////////////////////////////////////////////////////
INLINE int RenderAttrib::
compare_to(const RenderAttrib &other) const {
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
//     Function: RenderAttrib::always_reissue
//       Access: Public
//  Description: Returns true if the RenderAttrib should be reissued
//               to the GSG with every state change, even if it is the
//               same pointer as it was before; or false for the
//               normal case, to reissue only when the RenderAttrib
//               pointer changes.
////////////////////////////////////////////////////////////////////
INLINE bool RenderAttrib::
always_reissue() const {
  return _always_reissue;
}

