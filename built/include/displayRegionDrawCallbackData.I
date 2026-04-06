/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file displayRegionDrawCallbackData.I
 * @author drose
 * @date 2009-03-13
 */

/**
 * Returns a pointer to the CullResult, the list of CullableObjects that
 * should be drawn in this DisplayRegion.
 */
INLINE CullResult *DisplayRegionDrawCallbackData::
get_cull_result() const {
  return _cull_result;
}

/**
 * Returns a pointer to the SceneSetup object, which contains information
 * about the camera and such.
 */
INLINE SceneSetup *DisplayRegionDrawCallbackData::
get_scene_setup() const {
  return _scene_setup;
}
