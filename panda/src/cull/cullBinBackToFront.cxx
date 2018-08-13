/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinBackToFront.cxx
 * @author drose
 * @date 2002-02-28
 */

#include "cullBinBackToFront.h"
#include "graphicsStateGuardianBase.h"
#include "geometricBoundingVolume.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "pStatTimer.h"

#include <algorithm>


TypeHandle CullBinBackToFront::_type_handle;

/**
 *
 */
CullBinBackToFront::
~CullBinBackToFront() {
  Objects::iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;
    delete object;
  }
}

/**
 * Factory constructor for passing to the CullBinManager.
 */
CullBin *CullBinBackToFront::
make_bin(const std::string &name, GraphicsStateGuardianBase *gsg,
         const PStatCollector &draw_region_pcollector) {
  return new CullBinBackToFront(name, gsg, draw_region_pcollector);
}

/**
 * Adds a geom, along with its associated state, to the bin for rendering.
 */
void CullBinBackToFront::
add_object(CullableObject *object, Thread *current_thread) {
  // Determine the center of the bounding volume.
  CPT(BoundingVolume) volume = object->_geom->get_bounds(current_thread);
  if (volume->is_empty()) {
    delete object;
    return;
  }

  const GeometricBoundingVolume *gbv = volume->as_geometric_bounding_volume();
  nassertv(gbv != nullptr);

  LPoint3 center = gbv->get_approx_center();
  nassertv(object->_internal_transform != nullptr);
  center = center * object->_internal_transform->get_mat();

  PN_stdfloat distance = _gsg->compute_distance_to(center);
  _objects.push_back(ObjectData(object, distance));
}

/**
 * Called after all the geoms have been added, this indicates that the cull
 * process is finished for this frame and gives the bins a chance to do any
 * post-processing (like sorting) before moving on to draw.
 */
void CullBinBackToFront::
finish_cull(SceneSetup *, Thread *current_thread) {
  PStatTimer timer(_cull_this_pcollector, current_thread);
  sort(_objects.begin(), _objects.end());
}

/**
 * Draws all the geoms in the bin, in the appropriate order.
 */
void CullBinBackToFront::
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
void CullBinBackToFront::
fill_result_graph(CullBin::ResultGraphBuilder &builder) {
  Objects::const_iterator oi;
  for (oi = _objects.begin(); oi != _objects.end(); ++oi) {
    CullableObject *object = (*oi)._object;
    builder.add_object(object);
  }
}
