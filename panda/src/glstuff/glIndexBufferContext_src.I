/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glIndexBufferContext_src.I
 * @author drose
 * @date 2005-03-17
 */

/**
 *
 */
INLINE CLP(IndexBufferContext)::
CLP(IndexBufferContext)(CLP(GraphicsStateGuardian) *glgsg,
                        PreparedGraphicsObjects *pgo,
                        GeomPrimitive *data) :
  IndexBufferContext(pgo, data),
  _glgsg(glgsg)
{
  _index = 0;
}
