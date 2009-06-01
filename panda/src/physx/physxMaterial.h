// Filename: physxMaterial.h
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

#ifndef PHYSXMATERIAL_H
#define PHYSXMATERIAL_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

class PhysxMaterialDesc;
class PhysxScene;

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxMaterial
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxMaterial {
PUBLISHED:

  LVecBase3f get_dir_of_anisotropy() const;
  float get_dynamic_friction() const;
  float get_dynamic_friction_v() const;
  unsigned int get_flags() const;
  PhysxCombineMode get_friction_combine_mode() const;
  unsigned short get_material_index();
  float get_restitution() const;
  PhysxCombineMode get_restitution_combine_mode() const;
  PhysxScene & get_scene() const;
  float get_static_friction() const;
  float get_static_friction_v() const;
  void load_from_desc(const PhysxMaterialDesc & desc);
  void save_to_desc(PhysxMaterialDesc & desc) const;
  void set_dir_of_anisotropy(const LVecBase3f & vec);
  void set_dynamic_friction(float coef);
  void set_dynamic_friction_v(float coef);
  void set_flags(unsigned int flags);
  void set_friction_combine_mode(PhysxCombineMode comb_mode);
  void set_restitution(float rest);
  void set_restitution_combine_mode(PhysxCombineMode comb_mode);
  void set_static_friction(float coef);
  void set_static_friction_v(float coef);


public:
  NxMaterial *nMaterial;
};

#include "physxMaterial.I"

#endif // HAVE_PHYSX

#endif // PHYSXMATERIAL_H
