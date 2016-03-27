/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyMaterial.cxx
 * @author enn0x
 * @date 2011-03-19
 */

#include "bulletSoftBodyMaterial.h"

/**
 *
 */
BulletSoftBodyMaterial::
BulletSoftBodyMaterial(btSoftBody::Material &material) : _material(material) {

}
