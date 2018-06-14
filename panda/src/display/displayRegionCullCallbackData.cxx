/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file displayRegionCullCallbackData.cxx
 * @author drose
 * @date 2009-03-14
 */

#include "displayRegionCullCallbackData.h"
#include "cullHandler.h"
#include "sceneSetup.h"
#include "graphicsEngine.h"

TypeHandle DisplayRegionCullCallbackData::_type_handle;


/**
 *
 */
DisplayRegionCullCallbackData::
DisplayRegionCullCallbackData(CullHandler *cull_handler, SceneSetup *scene_setup) :
  _cull_handler(cull_handler),
  _scene_setup(scene_setup)
{
}

/**
 *
 */
void DisplayRegionCullCallbackData::
output(std::ostream &out) const {
  out << get_type() << "(" << (void *)_cull_handler << ", "
      << (void *)_scene_setup << ")";
}

/**
 * You should make this call during the callback if you want to continue the
 * normal rendering function that would have been done in the absence of a
 * callback.
 *
 * Specifically, this method will perform the cull traversal for the
 * DisplayRegion's scene graph, and add all renderable objects to its
 * CullResult.
 */
void DisplayRegionCullCallbackData::
upcall() {
  Thread *current_thread = Thread::get_current_thread();
  DisplayRegion *dr = _scene_setup->get_display_region();
  GraphicsStateGuardian *gsg = dr->get_window()->get_gsg();

  dr->do_cull(_cull_handler, _scene_setup, gsg, current_thread);
}
