// Filename: cullBinUnsorted.cxx
// Created by:  drose (28Feb02)
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

#include "cullBinUnsorted.h"
#include "cullHandler.h"
#include "graphicsStateGuardianBase.h"
#include "pStatTimer.h"


TypeHandle CullBinUnsorted::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullBinUnsorted::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CullBinUnsorted::
~CullBinUnsorted() {
  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi);
    delete object;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinUnsorted::add_object
//       Access: Public, Virtual
//  Description: Adds a geom, along with its associated state, to
//               the bin for rendering.
////////////////////////////////////////////////////////////////////
void CullBinUnsorted::
add_object(CullableObject *object) {
  _objects.push_back(object);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinUnsorted::draw
//       Access: Public
//  Description: Draws all the objects in the bin, in the appropriate
//               order.
////////////////////////////////////////////////////////////////////
void CullBinUnsorted::
draw() {
  PStatTimer timer(_draw_this_pcollector);
  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi);
    CullHandler::draw(object, _gsg);
  }
}

