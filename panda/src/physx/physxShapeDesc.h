/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxShapeDesc.h
 * @author enn0x
 * @date 2009-09-08
 */

#ifndef PHYSXSHAPEDESC_H
#define PHYSXSHAPEDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physxEnums.h"
#include "physx_includes.h"

class PhysxMaterial;

/**
 * Abstract base class for shape descriptors.  Descriptors for all the
 * different shape types are derived from this class.
 */
class EXPCL_PANDAPHYSX PhysxShapeDesc : public PhysxEnums, public ReferenceCount {

PUBLISHED:
  virtual void set_to_default() = 0;
  virtual bool is_valid() const = 0;

  void set_name(const char *name);
  void set_trigger(bool value);
  void set_local_pos(const LPoint3f &pos);
  void set_local_mat(const LMatrix4f &mat);
  void set_local_hpr(float h, float p, float r);
  void set_skin_width(float skinWidth);
  void set_shape_flag(const PhysxShapeFlag flag, bool value);
  void set_mass(float mass);
  void set_density(float density);
  void set_group(unsigned short group);
  void set_material(const PhysxMaterial &material);
  void set_material_index(unsigned short index);

  const char *get_name() const;
  LPoint3f get_local_pos() const;
  LMatrix4f get_local_mat() const;
  float get_skin_width() const;
  bool get_shape_flag(const PhysxShapeFlag flag) const;
  float get_mass() const;
  float get_density() const;
  unsigned short get_group() const;
  unsigned short get_material_index() const;

public:
  virtual NxShapeDesc *ptr() const = 0;

private:
  std::string _name;

protected:
  INLINE PhysxShapeDesc();
  INLINE ~PhysxShapeDesc();
};

#include "physxShapeDesc.I"

#endif // PHYSXSHAPEDESC_H
