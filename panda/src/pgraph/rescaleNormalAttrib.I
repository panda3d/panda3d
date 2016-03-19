/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rescaleNormalAttrib.I
 * @author drose
 * @date 2004-12-30
 */

/**
 * Use RescaleNormalAttrib::make() to construct a new RescaleNormalAttrib
 * object.
 */
INLINE RescaleNormalAttrib::
RescaleNormalAttrib(RescaleNormalAttrib::Mode mode) :
  _mode(mode)
{
}

/**
 * Constructs a RescaleNormalAttrib object that's suitable for putting at the
 * top of a scene graph.  This will contain whatever attrib was suggested by
 * the user's rescale-normals Config variable.
 */
INLINE CPT(RenderAttrib) RescaleNormalAttrib::
make_default() {
  return RenderAttribRegistry::quick_get_global_ptr()->get_slot_default(_attrib_slot);
}

/**
 * Returns the render mode.
 */
INLINE RescaleNormalAttrib::Mode RescaleNormalAttrib::
get_mode() const {
  return _mode;
}
