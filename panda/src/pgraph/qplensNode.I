// Filename: qplensNode.I
// Created by:  drose (26Feb02)
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


////////////////////////////////////////////////////////////////////
//     Function: copy_lens
//       Access: Public
//  Description: Sets up the qpLensNode using a copy of the
//               indicated Lens.  If the original Lens is
//               changed or destroyed, this qpLensNode is not
//               affected.
////////////////////////////////////////////////////////////////////
INLINE void qpLensNode::
copy_lens(const Lens &lens) {
  _lens = lens.make_copy();
}

////////////////////////////////////////////////////////////////////
//     Function: set_lens
//       Access: Public
//  Description: Sets up the qpLensNode using this particular Lens
//               pointer.  If the lens is subsequently modified, the
//               qpLensNode properties immediately reflect the change.
////////////////////////////////////////////////////////////////////
INLINE void qpLensNode::
set_lens(Lens *lens) {
  _lens = lens;
}

////////////////////////////////////////////////////////////////////
//     Function: get_lens
//       Access: Public
//  Description: Returns a pointer to the particular Lens
//               associated with this qpLensNode, or NULL if there is
//               not yet a Lens associated.
////////////////////////////////////////////////////////////////////
INLINE Lens *qpLensNode::
get_lens() const {
  return _lens;
}
