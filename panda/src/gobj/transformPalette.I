// Filename: transformPalette.I
// Created by:  drose (23Mar05)
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
//     Function: TransformPalette::is_registered
//       Access: Published
//  Description: Returns true if this palette has been registered.
//               Once it has been registered, the set of transforms in
//               a TransformPalette may not be further modified; but
//               it must be registered before it can be assigned to a
//               Geom.
////////////////////////////////////////////////////////////////////
INLINE bool TransformPalette::
is_registered() const {
  return _is_registered;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::register_palette
//       Access: Published, Static
//  Description: Registers a TransformPalette for use.  This is
//               similar to GeomVertexFormat::register_format().  Once
//               registered, a TransformPalette may no longer be
//               modified (although the individual VertexTransform
//               objects may modify their reported transforms).
//
//               This must be called before a palette may be used in a
//               Geom.  After this call, you should discard the
//               original pointer you passed in (which may or may not
//               now be invalid) and let its reference count decrement
//               normally; you should use only the returned value from
//               this point on.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformPalette) TransformPalette::
register_palette(TransformPalette *palette) {
  // We don't actually bother adding the palette object to a registry.
  // This means there may be multiple copies of identical registered
  // TransformPalettes.  Big deal.  We can always go back and make a
  // registry later if we really need it.
  if (palette->is_registered()) {
    return palette;
  }

  palette->do_register();
  return palette;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::get_num_transforms
//       Access: Published
//  Description: Returns the number of transforms in the palette.
////////////////////////////////////////////////////////////////////
INLINE int TransformPalette::
get_num_transforms() const {
  return _transforms.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::get_transform
//       Access: Published
//  Description: Returns the nth transform in the palette.
////////////////////////////////////////////////////////////////////
INLINE const VertexTransform *TransformPalette::
get_transform(int n) const {
  nassertr(n >= 0 && n < (int)_transforms.size(), NULL);
  return _transforms[n];
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::get_modified
//       Access: Published
//  Description: Returns a sequence number that's guaranteed to change
//               at least when any VertexTransforms in the palette
//               change.  (However, this is only true for a registered
//               palette.  An unregistered palette may or may not
//               reflect an update here when a VertexTransform
//               changes.)
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq TransformPalette::
get_modified() const {
  CDReader cdata(_cycler);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::update_modified
//       Access: Private
//  Description: Called internally whenever a nested VertexTransform
//               reports that it has been modified.
////////////////////////////////////////////////////////////////////
INLINE void TransformPalette::
update_modified(UpdateSeq) {
  CDWriter cdata(_cycler);
  ++(cdata->_modified);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TransformPalette::CData::
CData() {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TransformPalette::CData::
CData(const TransformPalette::CData &copy) :
  _modified(copy._modified)
{
}
