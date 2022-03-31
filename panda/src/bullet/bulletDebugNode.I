/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletDebugNode.I
 * @author enn0x
 * @date 2010-01-23
 */

/**
 *
 */
INLINE BulletDebugNode::
~BulletDebugNode() {

}

/**
 * If true, displays collision shapes in wireframe mode.
 */
INLINE void BulletDebugNode::
show_wireframe(bool show) {

  _wireframe = show;
  draw_mask_changed();
}

/**
 *
 */
INLINE bool BulletDebugNode::
get_show_wireframe() const {

  return _wireframe;
}

/**
 * If true, display limits defined for constraints, e.g. a pivot axis or maximum
 * amplitude.
 */
INLINE void BulletDebugNode::
show_constraints(bool show) {

  _constraints = show;
  draw_mask_changed();
}

/**
 *
 */
INLINE bool BulletDebugNode::
get_show_constraints() const {

  return _constraints;
}

/**
 * If true, displays axis aligned bounding boxes for objects.
 */
INLINE void BulletDebugNode::
show_bounding_boxes(bool show) {

  _bounds = show;
  draw_mask_changed();
}

/**
 *
 */
INLINE bool BulletDebugNode::
get_show_bounding_boxes() const {

  return _bounds;
}

/**
 * If true, displays normal vectors for triangle mesh and heightfield faces.
 */
INLINE void BulletDebugNode::
show_normals(bool show) {

  _drawer._normals = show;
}

/**
 *
 */
INLINE bool BulletDebugNode::
get_show_normals() const {

  return _drawer._normals;
}
