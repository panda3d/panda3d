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
#include "graphicsStateGuardianBase.h"


TypeHandle CullBinUnsorted::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullBinUnsorted::add_geom
//       Access: Public, Virtual
//  Description: Adds the geom, along with its associated state, to
//               the bin for rendering.
////////////////////////////////////////////////////////////////////
void CullBinUnsorted::
add_geom(Geom *geom, const TransformState *transform,
         const RenderState *state) {
  _geoms.push_back(GeomData(geom, transform, state));
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinUnsorted::draw
//       Access: Public
//  Description: Draws all the geoms in the bin, in the appropriate
//               order.
////////////////////////////////////////////////////////////////////
void CullBinUnsorted::
draw() {
  Geoms::iterator gi;
  for (gi = _geoms.begin(); gi != _geoms.end(); ++gi) {
    GeomData &geom_data = (*gi);
    _gsg->set_state_and_transform(geom_data._state, geom_data._transform);
    geom_data._geom->draw(_gsg);
  }
}

