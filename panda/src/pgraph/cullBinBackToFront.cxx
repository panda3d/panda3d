// Filename: cullBinBackToFront.cxx
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

#include "cullBinBackToFront.h"
#include "graphicsStateGuardianBase.h"
#include "geometricBoundingVolume.h"

#include <algorithm>


TypeHandle CullBinBackToFront::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::add_geom
//       Access: Public, Virtual
//  Description: Adds the geom, along with its associated state, to
//               the bin for rendering.
////////////////////////////////////////////////////////////////////
void CullBinBackToFront::
add_geom(Geom *geom, const TransformState *transform,
         const RenderState *state) {
  // Determine the center of the bounding volume.
  const BoundingVolume &volume = geom->get_bound();

  if (!volume.is_empty() &&
      volume.is_of_type(GeometricBoundingVolume::get_class_type())) {
    const GeometricBoundingVolume *gbv;
    DCAST_INTO_V(gbv, &volume);
    
    LPoint3f center = gbv->get_approx_center();
    center = center * transform->get_mat();
    
    // Oops!  Don't have compute_distance_to() here yet!
    //    float distance = gsg->compute_distance_to(center);
    float distance = -center[2];
    _geoms.push_back(GeomData(geom, transform, state, distance));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::finish_cull
//       Access: Public
//  Description: Called after all the geoms have been added, this
//               indicates that the cull process is finished for this
//               frame and gives the bins a chance to do any
//               post-processing (like sorting) before moving on to
//               draw.
////////////////////////////////////////////////////////////////////
void CullBinBackToFront::
finish_cull() {
  sort(_geoms.begin(), _geoms.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::draw
//       Access: Public
//  Description: Draws all the geoms in the bin, in the appropriate
//               order.
////////////////////////////////////////////////////////////////////
void CullBinBackToFront::
draw() {
  Geoms::iterator gi;
  for (gi = _geoms.begin(); gi != _geoms.end(); ++gi) {
    GeomData &geom_data = (*gi);
    _gsg->set_state_and_transform(geom_data._state, geom_data._transform);
    geom_data._geom->draw(_gsg);
  }
}

