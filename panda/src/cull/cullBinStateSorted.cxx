/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinStateSorted.cxx
 * @author drose
 * @date 2005-03-22
 */

#include "cullBinStateSorted.h"
#include "graphicsStateGuardianBase.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "pStatTimer.h"

#include <algorithm>


TypeHandle CullBinStateSorted::_type_handle;

/**
 * Factory constructor for passing to the CullBinManager.
 */
CullBin *CullBinStateSorted::
make_bin(const std::string &name, GraphicsStateGuardianBase *gsg,
         const PStatCollector &draw_region_pcollector) {
  return new CullBinStateSorted(name, gsg, draw_region_pcollector);
}

/**
 * Adds a geom, along with its associated state, to the bin for rendering.
 */
void CullBinStateSorted::
add_object(CullableObject *object, Thread *current_thread) {
  _objects.push_back(ObjectData(object));
}

/**
 * Called after all the geoms have been added, this indicates that the cull
 * process is finished for this frame and gives the bins a chance to do any
 * post-processing (like sorting) before moving on to draw.
 */
void CullBinStateSorted::
finish_cull(SceneSetup *, Thread *current_thread) {
  PStatTimer timer(_cull_this_pcollector, current_thread);
  sort(_objects.begin(), _objects.end());
}


/**
 * Draws all the geoms in the bin, in the appropriate order.
 */
void CullBinStateSorted::
draw(bool force, Thread *current_thread) {
  PStatTimer timer(_draw_this_pcollector, current_thread);

  for (const ObjectData &data : _objects) {
    data._object->draw(_gsg, force, current_thread);
  }
}

/**
 * Called by CullBin::make_result_graph() to add all the geoms to the special
 * cull result scene graph.
 */
void CullBinStateSorted::
fill_result_graph(CullBin::ResultGraphBuilder &builder) {
  Objects::const_iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;
    builder.add_object(object);
  }
}
