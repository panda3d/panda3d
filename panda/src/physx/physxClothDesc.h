/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxClothDesc.h
 * @author enn0x
 * @date 2010-03-30
 */

#ifndef PHYSXCLOTHDESC_H
#define PHYSXCLOTHDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physxEnums.h"
#include "physx_includes.h"

class PhysxClothMesh;

/**
 * Descriptor for PhysxCloth.
 */
class EXPCL_PANDAPHYSX PhysxClothDesc : public PhysxEnums {

PUBLISHED:
  INLINE PhysxClothDesc();
  INLINE ~PhysxClothDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_cloth_mesh(PhysxClothMesh *mesh);

  void set_name(const char *name);
  void set_global_pos(const LPoint3f &pos);
  void set_global_mat(const LMatrix4f &mat);
  void set_global_hpr(float h, float p, float r);
  void set_thickness(float thickness);
  void set_density(float density);
  void set_bending_stiffness(float stiffness);
  void set_stretching_stiffness(float stiffness);
  void set_damping_coefficient(float damping);
  void set_friction(float friction);
  void set_pressure(float pressure);
  void set_tear_factor(float tearFactor);
  void set_solver_iterations(unsigned int interations);
  void set_flag(PhysxClothFlag flag, bool value);

  const char *get_name() const;
  LPoint3f get_global_pos() const;
  LMatrix4f get_global_mat() const;
  float get_thickness() const;
  float get_density() const;
  float get_bending_stiffness() const;
  float get_stretching_stiffness() const;
  float get_damping_coefficient() const;
  float get_friction() const;
  float get_pressure() const;
  float get_tear_factor() const;
  unsigned int get_solver_iterations() const;
  bool get_flag(PhysxClothFlag flag) const;

public:
  NxClothDesc _desc;

private:
  std::string _name;
};

#include "physxClothDesc.I"

#endif // PHYSXCLOTHDESC_H
