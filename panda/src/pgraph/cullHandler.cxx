// Filename: cullHandler.cxx
// Created by:  drose (23Feb02)
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

#include "cullHandler.h"
#include "cullableObject.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"
#include "pnotify.h"

////////////////////////////////////////////////////////////////////
//     Function: CullHandler::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CullHandler::
CullHandler() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullHandler::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CullHandler::
~CullHandler() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullHandler::record_object
//       Access: Public, Virtual
//  Description: This callback function is intended to be overridden
//               by a derived class.  This is called as each Geom is
//               discovered by the CullTraverser.
//
//               The CullHandler becomes the owner of the
//               CullableObject pointer and is expected to delete it
//               later.
////////////////////////////////////////////////////////////////////
void CullHandler::
record_object(CullableObject *object, const CullTraverser *traverser) {
  nout << *object->_geom << " " << *object->_modelview_transform << " " 
       << *object->_state << "\n";
  delete object;
}

////////////////////////////////////////////////////////////////////
//     Function: CullHandler::end_traverse
//       Access: Public, Virtual
//  Description: This callback function is intended to be overridden
//               by a derived class.  This is called at the end of the
//               traversal.
////////////////////////////////////////////////////////////////////
void CullHandler::
end_traverse() {
}

////////////////////////////////////////////////////////////////////
//     Function: CullHandler::draw_with_decals
//       Access: Public, Static
//  Description: Draws the indicated CullableObject, assuming it has
//               attached decals.
////////////////////////////////////////////////////////////////////
void CullHandler::
draw_with_decals(CullableObject *object, GraphicsStateGuardianBase *gsg,
                 bool force, Thread *current_thread) {
  // We draw with a three-step process.

  // First, render all of the base geometry for the first pass.
  CPT(RenderState) state = gsg->begin_decal_base_first();

  CullableObject *base = object;
  while (base != (CullableObject *)NULL && base->_geom != (Geom *)NULL) {
    gsg->set_state_and_transform(base->_state->compose(state), base->_internal_transform);
    base->draw(gsg, force, current_thread);
    
    base = base->_next;
  }

  if (base != (CullableObject *)NULL) {
    // Now, draw all the decals.
    state = gsg->begin_decal_nested();

    CullableObject *decal = base->_next;
    while (decal != (CullableObject *)NULL) {
      gsg->set_state_and_transform(decal->_state->compose(state), decal->_internal_transform);
      decal->draw(gsg, force, current_thread);
      decal = decal->_next;
    }
  }

  // And now, re-draw the base geometry, if required.
  state = gsg->begin_decal_base_second();
  if (state != (const RenderState *)NULL) {
    base = object;
    while (base != (CullableObject *)NULL && base->_geom != (Geom *)NULL) {
      gsg->set_state_and_transform(base->_state->compose(state), base->_internal_transform);
      base->draw(gsg, force, current_thread);
      
      base = base->_next;
    }
  }
}

