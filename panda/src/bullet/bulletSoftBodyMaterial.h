/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletSoftBodyMaterial.h
 * @author enn0x
 * @date 2011-03-19
 */

#ifndef __BULLET_SOFT_BODY_MATERIAL_H__
#define __BULLET_SOFT_BODY_MATERIAL_H__

#include "pandabase.h"

#include "bullet_includes.h"

#include "numeric_types.h"

/**
 *
 */
class EXPCL_PANDABULLET BulletSoftBodyMaterial {

PUBLISHED:
  INLINE ~BulletSoftBodyMaterial();
  INLINE static BulletSoftBodyMaterial empty();

  PN_stdfloat get_linear_stiffness() const;
  void set_linear_stiffness(PN_stdfloat value);
  
  PN_stdfloat get_angular_stiffness() const;
  void set_angular_stiffness(PN_stdfloat value);

  PN_stdfloat get_volume_preservation() const;
  void set_volume_preservation(PN_stdfloat value);
  
  MAKE_PROPERTY(linear_stiffness, get_linear_stiffness, set_linear_stiffness);
  MAKE_PROPERTY(angular_stiffness, get_angular_stiffness, set_angular_stiffness);
  MAKE_PROPERTY(volume_preservation, get_volume_preservation, set_volume_preservation);

public:
  BulletSoftBodyMaterial(btSoftBody::Material &material);

  INLINE btSoftBody::Material &get_material() const;

private:
  btSoftBody::Material &_material;
};

#include "bulletSoftBodyMaterial.I"

#endif // __BULLET_SOFT_BODY_MATERIAL_H__
