// Filename: cullBinFixed.cxx
// Created by:  drose (29May02)
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

#include "cullBinFixed.h"
#include "graphicsStateGuardianBase.h"
#include "geometricBoundingVolume.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "pStatTimer.h"

#include <algorithm>


TypeHandle CullBinFixed::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullBinFixed::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CullBinFixed::
~CullBinFixed() {
  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;
    delete object;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinFixed::add_object
//       Access: Public, Virtual
//  Description: Adds a geom, along with its associated state, to
//               the bin for rendering.
////////////////////////////////////////////////////////////////////
void CullBinFixed::
add_object(CullableObject *object) {
  int draw_order = object->_state->get_draw_order();
  _objects.push_back(ObjectData(object, draw_order));
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinFixed::finish_cull
//       Access: Public
//  Description: Called after all the geoms have been added, this
//               indicates that the cull process is finished for this
//               frame and gives the bins a chance to do any
//               post-processing (like sorting) before moving on to
//               draw.
////////////////////////////////////////////////////////////////////
void CullBinFixed::
finish_cull() {
  PStatTimer timer(_cull_this_pcollector);
  stable_sort(_objects.begin(), _objects.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinFixed::draw
//       Access: Public
//  Description: Draws all the geoms in the bin, in the appropriate
//               order.
////////////////////////////////////////////////////////////////////
void CullBinFixed::
draw() {
  PStatTimer timer(_draw_this_pcollector);
  Objects::const_iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;
    CullHandler::draw(object, _gsg);
  }
}

