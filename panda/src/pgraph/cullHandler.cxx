// Filename: cullHandler.cxx
// Created by:  drose (23Feb02)
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

#include "cullHandler.h"
#include "cullableObject.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"
#include "notify.h"

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
  nout << *object->_geom << " " << *object->_transform << " " 
       << *object->_state << "\n";
  delete object;
}

////////////////////////////////////////////////////////////////////
//     Function: CullHandler::draw_with_decals
//       Access: Public, Static
//  Description: Draws the indicated CullableObject, assuming it has
//               attached decals.
////////////////////////////////////////////////////////////////////
void CullHandler::
draw_with_decals(CullableObject *object, GraphicsStateGuardianBase *gsg) {
  // We draw with a three-step process.

  // First, render all of the base geometry for the first pass.
  CPT(RenderState) state = gsg->begin_decal_base_first();

  CullableObject *base = object;
  while (base != (CullableObject *)NULL && base->_geom != (Geom *)NULL) {
    gsg->set_state_and_transform(base->_state->compose(state), base->_transform);
    base->draw(gsg);
    
    base = base->_next;
  }

  if (base != (CullableObject *)NULL) {
    // Now, draw all the decals.
    state = gsg->begin_decal_nested();

    CullableObject *decal = base->_next;
    while (decal != (CullableObject *)NULL) {
      gsg->set_state_and_transform(decal->_state->compose(state), decal->_transform);
      decal->draw(gsg);
      decal = decal->_next;
    }
  }

  // And now, re-draw the base geometry, if required.
  state = gsg->begin_decal_base_second();
  if (state != (const RenderState *)NULL) {
    base = object;
    while (base != (CullableObject *)NULL && base->_geom != (Geom *)NULL) {
      gsg->set_state_and_transform(base->_state->compose(state), base->_transform);
      base->draw(gsg);
      
      base = base->_next;
    }
  }
}

