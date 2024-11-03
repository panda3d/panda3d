/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipeOcclusionCullTraverser.I
 * @author drose
 * @date 2007-05-29
 */

/**
 *
 */
INLINE GraphicsOutput *PipeOcclusionCullTraverser::
get_buffer() const {
  return _buffer;
}

/**
 * Specifies the DrawMask that should be set on occlusion polygons for this
 * scene.  This identifies the polygons that are to be treated as occluders.
 * Polygons that do not have this draw mask set will not be considered
 * occluders.
 */
INLINE void PipeOcclusionCullTraverser::
set_occlusion_mask(const DrawMask &occlusion_mask) {
  _occlusion_mask = occlusion_mask;
}

/**
 * Returns the DrawMask for occlusion polygons.  See set_occlusion_mask().
 */
INLINE const DrawMask &PipeOcclusionCullTraverser::
get_occlusion_mask() const {
  return _occlusion_mask;
}

/**
 *
 */
INLINE PipeOcclusionCullTraverser::PendingObject::
PendingObject(CullableObject &&object) :
  _object(std::move(object))
{
}

/**
 *
 */
INLINE PipeOcclusionCullTraverser::PendingObject::
~PendingObject() {
  // Tempting as it is, we shouldn't delete the CullableObject in the
  // destructor, since it has already been deleted.  delete _object;
}
