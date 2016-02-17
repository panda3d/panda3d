/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTriangleMesh.I
 * @author enn0x
 * @date 2010-02-09
 */

/**
 *
 */
INLINE BulletTriangleMesh::
~BulletTriangleMesh() {

  delete _mesh;
}

/**
 *
 */
btTriangleMesh *BulletTriangleMesh::
ptr() const {

  return _mesh;
}

/**
 *
 */
INLINE ostream &
operator << (ostream &out, const BulletTriangleMesh &obj) {

  obj.output(out);
  return out;
}
