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

INLINE void LensNode::
copy_lens(long index, const Lens &lens) {
  if ( (index > (long) (_additional_lenses.size() + 1) ) || (index <= 0) )
    _lens = lens.make_copy();
  else
    _additional_lenses[index-1] = lens.make_copy();
  if (_shown_frustum != (PandaNode *)NULL) {
    show_frustum();
  }
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

INLINE void LensNode::
set_lens(long index, Lens *lens) {
  if (index > MAX_LENSES)
    return;  // Sanity check
  while ( index >= (long) (_additional_lenses.size() + 1) ) {
    _additional_lenses.push_back(lens);
    _lens_is_active.push_back(false);
  }
  
  if (index <= 0)
    _lens = lens;
  else
    _additional_lenses[index-1] = lens;
  activate_lens(index);
  if (_shown_frustum != (PandaNode *)NULL) {
    show_frustum();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::get_lens
//       Access: Published
//  Description: Returns a pointer to the particular Lens
//               associated with this LensNode, or NULL if there is
//               not yet a Lens associated.
////////////////////////////////////////////////////////////////////
INLINE Lens *LensNode::
get_lens(long index) const {
  if ( ( index > (long) _additional_lenses.size() ) || (index <= 0) )
    return _lens;
  return _additional_lenses[index-1];
}

INLINE bool LensNode::
activate_lens(long index) {
  if ( ( index >= (long) (_additional_lenses.size() + 1) ) || (index < 0) )
    return false;
  if ( _lens_is_active[index] )
    return false;
  _lens_is_active[index] = true;
  return true;
}

INLINE bool LensNode::
deactivate_lens(long index) {
  if ( ( index > (long) (_additional_lenses.size() + 1) ) || (index < 0) )
    return false;
  _lens_is_active[index] = true;
  return true;
}
