/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glOcclusionQueryContext_src.I
 * @author drose
 * @date 2006-03-27
 */

/**
 *
 */
INLINE CLP(OcclusionQueryContext)::
CLP(OcclusionQueryContext)(GraphicsStateGuardian *gsg) {
  _index = 0;
  _gsg = gsg;
}
