// Filename: lensNode.I
// Created by:  drose (26Feb02)
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
//     Function: LensNode::copy_lens
//       Access: Published
//  Description: Sets up the LensNode using a copy of the
//               indicated Lens.  If the original Lens is
//               changed or destroyed, this LensNode is not
//               affected.
////////////////////////////////////////////////////////////////////
INLINE void LensNode::
copy_lens(const Lens &lens) {
  return copy_lens(0, lens);
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::copy_lens
//       Access: Published
//  Description: Copies the indicated lens into the specified slot.
////////////////////////////////////////////////////////////////////
INLINE void LensNode::
copy_lens(int index, const Lens &lens) {
  set_lens(index, lens.make_copy());
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::set_lens
//       Access: Published
//  Description: Sets up the LensNode using this particular Lens
//               pointer.  If the lens is subsequently modified, the
//               LensNode properties immediately reflect the change.
////////////////////////////////////////////////////////////////////
INLINE void LensNode::
set_lens(Lens *lens) {
  return set_lens(0, lens);
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::get_lens
//       Access: Published
//  Description: Returns a pointer to the particular Lens
//               associated with this LensNode, or NULL if there is
//               not yet a Lens associated.  If an index number is
//               specified, returns the nth lens.
////////////////////////////////////////////////////////////////////
INLINE Lens *LensNode::
get_lens(int index) const {
  nassertr(index >= 0 && index < max_lenses, NULL); // Sanity check

  if (index < (int)_lenses.size()) {
    return _lenses[index]._lens;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::get_lens_active
//       Access: Published
//  Description: Returns the active flag for the nth lens.
////////////////////////////////////////////////////////////////////
INLINE bool LensNode::
get_lens_active(int index) const {
  nassertr(index >= 0 && index < max_lenses, false);

  if (index < (int)_lenses.size()) {
    return _lenses[index]._is_active;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::activate_lens
//       Access: Published
//  Description: An alternate way to call set_lens_active(index,
//               true).
////////////////////////////////////////////////////////////////////
INLINE bool LensNode::
activate_lens(int index) {
  return set_lens_active(index, true);
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::deactivate_lens
//       Access: Published
//  Description: An alternate way to call set_lens_active(index,
//               false).
////////////////////////////////////////////////////////////////////
INLINE bool LensNode::
deactivate_lens(int index) {
  return set_lens_active(index, false);
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::is_in_view
//       Access: Published
//  Description: Returns true if the given point is within the bounds
//               of the lens of the LensNode (i.e. if the camera can
//               see the point).
////////////////////////////////////////////////////////////////////
INLINE bool LensNode::
is_in_view(const LPoint3 &pos) {
  return is_in_view(0, pos);
}

