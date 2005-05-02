// Filename: lensNode.I
// Created by:  drose (26Feb02)
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
//     Function: LensNode::copy_lens
//       Access: Published
//  Description: Sets up the LensNode using a copy of the
//               indicated Lens.  If the original Lens is
//               changed or destroyed, this LensNode is not
//               affected.
////////////////////////////////////////////////////////////////////
INLINE void LensNode::
copy_lens(const Lens &lens) {
  _lens = lens.make_copy();
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
  _lens = lens;
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
get_lens() const {
  return _lens;
}
