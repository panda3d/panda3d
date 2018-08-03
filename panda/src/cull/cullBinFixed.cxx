/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinFixed.cxx
 * @author drose
 * @date 2002-05-29
 */

#include "cullBinFixed.h"
#include "graphicsStateGuardianBase.h"
#include "geometricBoundingVolume.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "pStatTimer.h"

#include <algorithm>


TypeHandle CullBinFixed::_type_handle;

/**
 *
 */
CullBinFixed::
~CullBinFixed() {
  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;
    delete object;
  }
}

/**
 * Factory constructor for passing to the CullBinManager.
 */
CullBin *CullBinFixed::
make_bin(const std::string &name, GraphicsStateGuardianBase *gsg,
         const PStatCollector &draw_region_pcollector) {
  return new CullBinFixed(name, gsg, draw_region_pcollector);
}

/**
 * Adds a geom, along with its associated state, to the bin for rendering.
 */
void CullBinFixed::
add_object(CullableObject *object, Thread *current_thread) {
  int draw_order = object->_state->get_draw_order();
  _objects.push_back(ObjectData(object, draw_order));
}

/**
 * Called after all the geoms have been added, this indicates that the cull
 * process is finished for this frame and gives the bins a chance to do any
 * post-processing (like sorting) before moving on to draw.
 */
void CullBinFixed::
finish_cull(SceneSetup *, Thread *current_thread) {
  PStatTimer timer(_cull_this_pcollector, current_thread);
  std::stable_sort(_objects.begin(), _objects.end());
}

/**
 * Draws all the geoms in the bin, in the appropriate order.
 */
void CullBinFixed::
draw(bool force, Thread *current_thread) {
  PStatTimer timer(_draw_this_pcollector, current_thread);

  Objects::const_iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;

    if (object->_draw_callback == nullptr) {
      nassertd(object->_geom != nullptr) continue;

      _gsg->set_state_and_transform(object->_state, object->_internal_transform);

      GeomPipelineReader geom_reader(object->_geom, current_thread);
      GeomVertexDataPipelineReader data_reader(object->_munged_data, current_thread);
      data_reader.check_array_readers();
      geom_reader.draw(_gsg, &data_reader, force);
    } else {
      // It has a callback associated.
      object->draw_callback(_gsg, force, current_thread);
      // Now the callback has taken care of drawing.
    }
  }
}

/**
 * Called by CullBin::make_result_graph() to add all the geoms to the special
 * cull result scene graph.
 */
void CullBinFixed::
fill_result_graph(CullBin::ResultGraphBuilder &builder) {
  Objects::const_iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;
    builder.add_object(object);
  }
}
