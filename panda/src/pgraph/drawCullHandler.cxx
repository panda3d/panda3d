// Filename: drawCullHandler.cxx
// Created by:  drose (25Feb02)
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

#include "drawCullHandler.h"
#include "cullableObject.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"
#include "graphicsStateGuardianBase.h"


////////////////////////////////////////////////////////////////////
//     Function: DrawCullHandler::record_object
//       Access: Public, Virtual
//  Description: This callback function is intended to be overridden
//               by a derived class.  This is called as each Geom is
//               discovered by the CullTraverser.
////////////////////////////////////////////////////////////////////
void DrawCullHandler::
record_object(CullableObject *object) {
  // Munge vertices as needed for the GSG's requirements, and the
  // object's current state.
  object->munge_vertices(_gsg);

  // And draw the object, then dispense with it.
  draw(object, _gsg);
  delete object;
}
