// Filename: cullBinBackToFront.cxx
// Created by:  drose (28Feb02)
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

#include "cullBinBackToFront.h"
#include "graphicsStateGuardianBase.h"
#include "geometricBoundingVolume.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "pStatTimer.h"

#include <algorithm>


TypeHandle CullBinBackToFront::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CullBinBackToFront::
~CullBinBackToFront() {
  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;
    delete object;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::make_bin
//       Access: Public, Static
//  Description: Factory constructor for passing to the CullBinManager.
////////////////////////////////////////////////////////////////////
CullBin *CullBinBackToFront::
make_bin(const string &name, GraphicsStateGuardianBase *gsg,
         const PStatCollector &draw_region_pcollector) {
  return new CullBinBackToFront(name, gsg, draw_region_pcollector);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::add_object
//       Access: Public, Virtual
//  Description: Adds a geom, along with its associated state, to
//               the bin for rendering.
////////////////////////////////////////////////////////////////////
void CullBinBackToFront::
add_object(CullableObject *object, Thread *current_thread) {
  // Determine the center of the bounding volume.
  CPT(BoundingVolume) volume = object->_geom->get_bounds(current_thread);
  if (volume->is_empty()) {
    delete object;
    return;
  }

  const GeometricBoundingVolume *gbv;
  DCAST_INTO_V(gbv, volume);

  LPoint3 center = gbv->get_approx_center();
  nassertv(object->_internal_transform != (const TransformState *)NULL);
  center = center * object->_internal_transform->get_mat();

  PN_stdfloat distance = _gsg->compute_distance_to(center);
  _objects.push_back(ObjectData(object, distance));
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
finish_cull(SceneSetup *, Thread *current_thread) {
  PStatTimer timer(_cull_this_pcollector, current_thread);
  sort(_objects.begin(), _objects.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::draw
//       Access: Public, Virtual
//  Description: Draws all the geoms in the bin, in the appropriate
//               order.
////////////////////////////////////////////////////////////////////
void CullBinBackToFront::
draw(bool force, Thread *current_thread) {
  PStatTimer timer(_draw_this_pcollector, current_thread);
  Objects::const_iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;
    CullHandler::draw(object, _gsg, force, current_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::fill_result_graph
//       Access: Protected, Virtual
//  Description: Called by CullBin::make_result_graph() to add all the
//               geoms to the special cull result scene graph.
////////////////////////////////////////////////////////////////////
void CullBinBackToFront::
fill_result_graph(CullBin::ResultGraphBuilder &builder) {
  Objects::const_iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;
    builder.add_object(object);
  }
}
