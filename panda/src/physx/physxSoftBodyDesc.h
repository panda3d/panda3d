/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSoftBodyDesc.h
 * @author enn0x
 * @date 2010-09-12
 */

#ifndef PHYSXSOFTBODYDESC_H
#define PHYSXSOFTBODYDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physxEnums.h"
#include "physx_includes.h"

class PhysxSoftBodyMesh;

/**
 * Descriptor for PhysxSoftBody.
 */
class EXPCL_PANDAPHYSX PhysxSoftBodyDesc : public PhysxEnums {

PUBLISHED:
  INLINE PhysxSoftBodyDesc();
  INLINE ~PhysxSoftBodyDesc();

  INLINE void set_to_default();
  INLINE bool is_valid() const;

  void set_soft_body_mesh(PhysxSoftBodyMesh *mesh);

  void set_name(const char *name);
  void set_global_pos(const LPoint3f &pos);
  void set_global_mat(const LMatrix4f &mat);
  void set_global_hpr(float h, float p, float r);
  void set_density(float density);
  void set_volume_stiffness(float stiffness);
  void set_stretching_stiffness(float stiffness);
  void set_damping_coefficient(float damping);
  void set_friction(float friction);
  void set_tear_factor(float tearFactor);
  void set_particle_radius(float radius);
  void set_relative_grid_spacing(float spacing);
  void set_collision_response_coefficient(float coef);
  void set_attachment_response_coefficient(float coef);
  void set_solver_iterations(unsigned int interations);
  void set_flag(PhysxSoftBodyFlag flag, bool value);

  const char *get_name() const;
  LPoint3f get_global_pos() const;
  LMatrix4f get_global_mat() const;
  float get_density() const;
  float get_volume_stiffness() const;
  float get_stretching_stiffness() const;
  float get_damping_coefficient() const;
  float get_friction() const;
  float get_tear_factor() const;
  float get_particle_radius() const;
  float get_relative_grid_spacing() const;
  float get_collision_response_coefficient() const;
  float get_attachment_response_coefficient() const;
  unsigned int get_solver_iterations() const;
  bool get_flag(PhysxSoftBodyFlag flag) const;

public:
  NxSoftBodyDesc _desc;

private:
  std::string _name;
};

#include "physxSoftBodyDesc.I"

#endif // PHYSXSOFTBODYDESC_H
