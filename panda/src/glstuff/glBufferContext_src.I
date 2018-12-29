/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glBufferContext_src.I
 * @author rdb
 * @date 2016-12-12
 */

/**
 *
 */
INLINE CLP(BufferContext)::
CLP(BufferContext)(CLP(GraphicsStateGuardian) *glgsg,
                   PreparedGraphicsObjects *pgo,
                   TypedWritableReferenceCount *object) :
  BufferContext(&pgo->_sbuffer_residency, object),
  AdaptiveLruPage(0),
  _glgsg(glgsg)
{
  _index = 0;
}
