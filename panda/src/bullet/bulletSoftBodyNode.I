/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyNode.I
 * @author enn0x
 * @date 2010-12-27
 */

/**
 *
 */
INLINE BulletSoftBodyNode::
~BulletSoftBodyNode() {

  delete _soft;
}

/**
 *
 */
INLINE BulletSoftBodyNodeElement::
~BulletSoftBodyNodeElement() {

}

/**
 * Named constructor intended to be used for asserts with have to return a
 * concrete value.
 */
INLINE BulletSoftBodyNodeElement BulletSoftBodyNodeElement::
empty() {

  static btSoftBody::Node node;

  return BulletSoftBodyNodeElement(node);
}

