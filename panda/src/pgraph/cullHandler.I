// Filename: cullHandler.I
// Created by:  drose (04Mar02)
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
//     Function: CullHandler::draw
//       Access: Public, Static
//  Description: Draws the indicated CullableObject, with full support
//               for decals if they are attached to the object.  The
//               appropriate state is set on the GSG before drawing
//               the object.
////////////////////////////////////////////////////////////////////
INLINE void CullHandler::
draw(CullableObject *object, GraphicsStateGuardianBase *gsg) {
  if (object->_next != (CullableObject *)NULL) {
    draw_with_decals(object, gsg);
  } else {
    gsg->set_state_and_transform(object->_state, object->_transform);
    object->_geom->draw(gsg);
  }
}
