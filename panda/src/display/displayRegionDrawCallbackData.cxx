/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file displayRegionDrawCallbackData.cxx
 * @author drose
 * @date 2009-03-13
 */

#include "displayRegionDrawCallbackData.h"

#include "config_display.h"
#include "cullResult.h"
#include "displayRegion.h"
#include "graphicsOutput.h"
#include "graphicsStateGuardian.h"
#include "sceneSetup.h"

TypeHandle DisplayRegionDrawCallbackData::_type_handle;


/**
 *
 */
DisplayRegionDrawCallbackData::
DisplayRegionDrawCallbackData(CullResult *cull_result, SceneSetup *scene_setup) :
  _cull_result(cull_result),
  _scene_setup(scene_setup)
{
}

/**
 *
 */
void DisplayRegionDrawCallbackData::
output(std::ostream &out) const {
  out << get_type() << "(" << (void *)_cull_result << ", "
      << (void *)_scene_setup << ")";
}

/**
 * You should make this call during the callback if you want to continue the
 * normal rendering function that would have been done in the absence of a
 * callback.
 *
 * Specifically, this method will draw all of the objects in the CullResult
 * list that have been built up for the DisplayRegion during the cull
 * traversal.
 */
void DisplayRegionDrawCallbackData::
upcall() {
  Thread *current_thread = Thread::get_current_thread();
  DisplayRegion *dr = _scene_setup->get_display_region();
  GraphicsStateGuardian *gsg = dr->get_window()->get_gsg();

  if (_cull_result == nullptr || _scene_setup == nullptr) {
    // Nothing to see here.

  } else if (dr->is_stereo()) {
    // We don't actually draw the stereo DisplayRegions.  These are just
    // placeholders; we draw the individual left and right eyes instead.  (We
    // might still clear the stereo DisplayRegions, though, since it's
    // probably faster to clear right and left channels in one pass, than to
    // clear them in two separate passes.)

  } else if (!gsg->set_scene(_scene_setup)) {
    // The scene or lens is inappropriate somehow.
    display_cat.error()
      << gsg->get_type() << " cannot render scene with specified lens.\n";

  } else {
    // Tell the GSG to forget its state.
    gsg->clear_state_and_transform();

    if (gsg->begin_scene()) {
      _cull_result->draw(current_thread);
      gsg->end_scene();
    }
  }
}
