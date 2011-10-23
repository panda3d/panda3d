// Filename: bulletSoftBodyMaterial.h
// Created by:  enn0x (19Mar11)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef __BULLET_SOFT_BODY_MATERIAL_H__
#define __BULLET_SOFT_BODY_MATERIAL_H__

#include "pandabase.h"

#include "bullet_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletSoftBodyMaterial
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletSoftBodyMaterial {

PUBLISHED:
  INLINE ~BulletSoftBodyMaterial();
  INLINE static BulletSoftBodyMaterial empty();

  INLINE void setLinearStiffness(PN_stdfloat value);
  INLINE void setAngularStiffness(PN_stdfloat value);
  INLINE void setVolumePreservation(PN_stdfloat value);

  INLINE PN_stdfloat getLinearStiffness() const;
  INLINE PN_stdfloat getAngularStiffness() const;
  INLINE PN_stdfloat getVolumePreservation() const;

public:
  BulletSoftBodyMaterial(btSoftBody::Material &material);

  INLINE btSoftBody::Material &get_material() const;

private:
  btSoftBody::Material &_material;
};

#include "bulletSoftBodyMaterial.I"

#endif // __BULLET_SOFT_BODY_MATERIAL_H__
