// Filename: cullBinFixed.cxx
// Created by:  drose (29May02)
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
//     Function: CullBinFixed::make_bin
//       Access: Public, Static
//  Description: Factory constructor for passing to the CullBinManager.
////////////////////////////////////////////////////////////////////
CullBin *CullBinFixed::
make_bin(const string &name, GraphicsStateGuardianBase *gsg,
         const PStatCollector &draw_region_pcollector) {
  return new CullBinFixed(name, gsg, draw_region_pcollector);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinFixed::add_object
//       Access: Public, Virtual
//  Description: Adds a geom, along with its associated state, to
//               the bin for rendering.
////////////////////////////////////////////////////////////////////
void CullBinFixed::
add_object(CullableObject *object, Thread *current_thread) {
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
finish_cull(SceneSetup *, Thread *current_thread) {
  PStatTimer timer(_cull_this_pcollector, current_thread);
  stable_sort(_objects.begin(), _objects.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinFixed::draw
//       Access: Public, Virtual
//  Description: Draws all the geoms in the bin, in the appropriate
//               order.
////////////////////////////////////////////////////////////////////
void CullBinFixed::
draw(bool force, Thread *current_thread) {
  PStatTimer timer(_draw_this_pcollector, current_thread);
  Objects::const_iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;
    CullHandler::draw(object, _gsg, force, current_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinFixed::fill_result_graph
//       Access: Protected, Virtual
//  Description: Called by CullBin::make_result_graph() to add all the
//               geoms to the special cull result scene graph.
////////////////////////////////////////////////////////////////////
void CullBinFixed::
fill_result_graph(CullBin::ResultGraphBuilder &builder) {
  Objects::const_iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;
    builder.add_object(object);
  }
}
