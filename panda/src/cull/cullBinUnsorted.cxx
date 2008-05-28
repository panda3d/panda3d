// Filename: cullBinUnsorted.cxx
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
//     Function: CullBinUnsorted::make_bin
//       Access: Public, Static
//  Description: Factory constructor for passing to the CullBinManager.
////////////////////////////////////////////////////////////////////
CullBin *CullBinUnsorted::
make_bin(const string &name, GraphicsStateGuardianBase *gsg,
         const PStatCollector &draw_region_pcollector) {
  return new CullBinUnsorted(name, gsg, draw_region_pcollector);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinUnsorted::add_object
//       Access: Public, Virtual
//  Description: Adds a geom, along with its associated state, to
//               the bin for rendering.
////////////////////////////////////////////////////////////////////
void CullBinUnsorted::
add_object(CullableObject *object, Thread *current_thread) {
  _objects.push_back(object);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinUnsorted::draw
//       Access: Public, Virtual
//  Description: Draws all the objects in the bin, in the appropriate
//               order.
////////////////////////////////////////////////////////////////////
void CullBinUnsorted::
draw(bool force, Thread *current_thread) {
  PStatTimer timer(_draw_this_pcollector, current_thread);
  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi);
    CullHandler::draw(object, _gsg, force, current_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinUnsorted::fill_result_graph
//       Access: Protected, Virtual
//  Description: Called by CullBin::make_result_graph() to add all the
//               geoms to the special cull result scene graph.
////////////////////////////////////////////////////////////////////
void CullBinUnsorted::
fill_result_graph(CullBin::ResultGraphBuilder &builder) {
  Objects::const_iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi);
    builder.add_object(object);
  }
}
