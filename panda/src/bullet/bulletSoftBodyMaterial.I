/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyMaterial.I
 * @author enn0x
 * @date 2011-03-19
 */

/**
 *
 */
INLINE BulletSoftBodyMaterial::
~BulletSoftBodyMaterial() {

}

/**
 * Named constructor intended to be used for asserts which have to return a
 * concrete value.
 */
INLINE BulletSoftBodyMaterial BulletSoftBodyMaterial::
empty() {

  static btSoftBody::Material material;

  return BulletSoftBodyMaterial(material);
}

/**
 *
 */
INLINE btSoftBody::Material &BulletSoftBodyMaterial::
get_material() const {

  return _material;
}
