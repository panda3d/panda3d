// Filename: physxMaterialDesc.h
// Created by:  pratt (Jul 9, 2006)
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

#ifndef PHYSXMATERIALDESC_H
#define PHYSXMATERIALDESC_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxMaterialDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxMaterialDesc {
PUBLISHED:
  PhysxMaterialDesc();

  INLINE bool is_valid() const;
  INLINE void set_to_default();

  LVecBase3f get_dir_of_anisotropy() const;
  INLINE float get_dynamic_friction() const;
  INLINE float get_dynamic_friction_v() const;
  INLINE unsigned int get_flags() const;
  PhysxCombineMode get_friction_combine_mode() const;
  INLINE float get_restitution() const;
  PhysxCombineMode get_restitution_combine_mode() const;
  INLINE float get_static_friction() const;
  INLINE float get_static_friction_v() const;

  void set_dir_of_anisotropy(LVecBase3f value);
  INLINE void set_dynamic_friction(float value);
  INLINE void set_dynamic_friction_v(float value);
  INLINE void set_flags(unsigned int value);
  void set_friction_combine_mode(PhysxCombineMode value);
  INLINE void set_restitution(float value);
  void set_restitution_combine_mode(PhysxCombineMode value);
  INLINE void set_static_friction(float value);
  INLINE void set_static_friction_v(float value);

public:
  NxMaterialDesc nMaterialDesc;
};

#include "physxMaterialDesc.I"

#endif // HAVE_PHYSX

#endif // PHYSXMATERIALDESC_H
