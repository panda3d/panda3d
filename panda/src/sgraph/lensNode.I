// Filename: lensNode.I
// Created by:  drose (23May00)
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
//     Function: LensNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LensNode::
LensNode(const string &name) :
  NamedNode(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LensNode::
LensNode(const LensNode &copy) :
  NamedNode(copy),
  _lens(copy._lens)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LensNode::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LensNode::
operator = (const LensNode &copy) {
  NamedNode::operator = (copy);
  _lens = copy._lens;
}

////////////////////////////////////////////////////////////////////
//     Function: copy_lens
//       Access: Public
//  Description: Sets up the LensNode using a copy of the
//               indicated Lens.  If the original Lens is
//               changed or destroyed, this LensNode is not
//               affected.
////////////////////////////////////////////////////////////////////
INLINE void LensNode::
copy_lens(const Lens &lens) {
  _lens = lens.make_copy();
}

////////////////////////////////////////////////////////////////////
//     Function: set_lens
//       Access: Public
//  Description: Sets up the LensNode using this particular Lens
//               pointer.  If the lens is subsequently modified, the
//               LensNode properties immediately reflect the change.
////////////////////////////////////////////////////////////////////
INLINE void LensNode::
set_lens(Lens *lens) {
  _lens = lens;
}

////////////////////////////////////////////////////////////////////
//     Function: get_lens
//       Access: Public
//  Description: Returns a pointer to the particular Lens
//               associated with this LensNode, or NULL if there is
//               not yet a Lens associated.
////////////////////////////////////////////////////////////////////
INLINE Lens *LensNode::
get_lens() {
  return _lens;
}
