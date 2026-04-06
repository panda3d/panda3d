/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animateVerticesRequest.I
 * @author pratt
 * @date 2007-11-20
 */

/**
 * Create a new AnimateVerticesRequest.
 */
INLINE AnimateVerticesRequest::
AnimateVerticesRequest(GeomVertexData *geom_vertex_data) :
  _geom_vertex_data(geom_vertex_data)
{
}

/**
 * Returns true if this request has completed, false if it is still pending.
 * Equivalent to `req.done() and not req.cancelled()`.
 * @see done()
 */
INLINE bool AnimateVerticesRequest::
is_ready() const {
  return (FutureState)_future_state.load(std::memory_order_relaxed) == FS_finished;
}
