// Filename: drawCullHandler.cxx
// Created by:  drose (25Feb02)
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

#include "drawCullHandler.h"
#include "cullableObject.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"
#include "graphicsStateGuardianBase.h"
#include "config_pgraph.h"
#include "cullTraverser.h"

////////////////////////////////////////////////////////////////////
//     Function: DrawCullHandler::record_object
//       Access: Public, Virtual
//  Description: This callback function is intended to be overridden
//               by a derived class.  This is called as each Geom is
//               discovered by the CullTraverser.
////////////////////////////////////////////////////////////////////
void DrawCullHandler::
record_object(CullableObject *object, const CullTraverser *traverser) {
  // Munge vertices as needed for the GSG's requirements, and the
  // object's current state.
  bool force = !_gsg->get_effective_incomplete_render();
  Thread *current_thread = traverser->get_current_thread();

  if (object->munge_geom(_gsg, _gsg->get_geom_munger(object->_state, current_thread), traverser, force)) {
    // Now we can immediately draw the object.
    draw(object, _gsg, force, current_thread);
  }

  // Dispense with the object.
  delete object;
}
