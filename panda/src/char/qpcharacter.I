// Filename: qpcharacter.I
// Created by:  drose (06Mar02)
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

#include "characterJointBundle.h"

////////////////////////////////////////////////////////////////////
//     Function: qpCharacter::get_bundle
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CharacterJointBundle *qpCharacter::
get_bundle() const {
  return DCAST(CharacterJointBundle, qpPartBundleNode::get_bundle());
}


////////////////////////////////////////////////////////////////////
//     Function: qpCharacter::get_computed_vertices
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ComputedVertices *qpCharacter::
get_computed_vertices() const {
  return _computed_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: qpCharacter::get_num_parts
//       Access: Public
//  Description: Returns the total number of moving parts (e.g. joints
//               and sliders) associated with the qpCharacter.
////////////////////////////////////////////////////////////////////
INLINE int qpCharacter::
get_num_parts() const {
  return _parts.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpCharacter::get_part
//       Access: Public
//  Description: Returns the nth moving part associated with the
//               qpCharacter.
////////////////////////////////////////////////////////////////////
INLINE PartGroup *qpCharacter::
get_part(int n) const {
  nassertr(n >= 0 && n < (int)_parts.size(), NULL);
  return _parts[n];
}

////////////////////////////////////////////////////////////////////
//     Function: qpCharacter::write_parts
//       Access: Public
//  Description: Writes a list of the qpCharacter's joints and sliders,
//               in their hierchical structure, to the indicated
//               output stream.
////////////////////////////////////////////////////////////////////
INLINE void qpCharacter::
write_parts(ostream &out) const {
  get_bundle()->write(out, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: qpCharacter::write_part_values
//       Access: Public
//  Description: Writes a list of the qpCharacter's joints and sliders,
//               along with each current position, in their hierchical
//               structure, to the indicated output stream.
////////////////////////////////////////////////////////////////////
INLINE void qpCharacter::
write_part_values(ostream &out) const {
  get_bundle()->write_with_value(out, 0);
}
