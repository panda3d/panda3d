// Filename: cullHandler.I
// Created by:  drose (04Mar02)
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
//     Function: CullHandler::draw
//       Access: Public, Static
//  Description: Draws the indicated CullableObject, with full support
//               for decals if they are attached to the object.  The
//               appropriate state is set on the GSG before drawing
//               the object.
////////////////////////////////////////////////////////////////////
INLINE void CullHandler::
draw(CullableObject *object, GraphicsStateGuardianBase *gsg,
     bool force, Thread *current_thread) {
  object->draw(gsg, force, current_thread);
}
