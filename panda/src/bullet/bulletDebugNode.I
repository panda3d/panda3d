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
 *
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
 *
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
 *
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
 *
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