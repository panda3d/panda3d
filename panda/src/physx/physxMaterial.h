/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxMaterial.h
 * @author enn0x
 * @date 2009-09-21
 */

#ifndef PHYSXMATERIAL_H
#define PHYSXMATERIAL_H

#include "pandabase.h"
#include "luse.h"

#include "physxObject.h"
#include "physxEnums.h"
#include "physx_includes.h"

class PhysxScene;
class PhysxMaterialDesc;

/**
 * A class for describing a shape's surface properties.
 *
 * You can create a material which has different friction coefficients
 * depending on the direction that a body in contact is trying to move in.
 * This is called anisotropic friction.
 *
 * Anisotropic friction is useful for modeling things like sledges, skis etc
 *
 * When you create an anisotropic material you specify the default friction
 * parameters and also friction parameters for the V axis.  The friction
 * parameters for the V axis are applied to motion along the direction of
 * anisotropy (dirOfAnisotropy).
 *
 * Default material: You can change the properties of the default material by
 * querying for material index 0.
 */
class EXPCL_PANDAPHYSX PhysxMaterial : public PhysxObject, public PhysxEnums {

PUBLISHED:
  INLINE PhysxMaterial();
  INLINE ~PhysxMaterial();

  PhysxScene *get_scene() const;
  unsigned short get_material_index() const;

  void load_from_desc(const PhysxMaterialDesc &materialDesc);
  void save_to_desc(PhysxMaterialDesc &materialDesc) const;

  void set_dynamic_friction(float coef);
  void set_static_friction(float coef);
  void set_restitution(float rest);
  void set_dynamic_friction_v(float coef);
  void set_static_friction_v(float coef);
  void set_dir_of_anisotropy(const LVector3f dir);
  void set_flag(PhysxMaterialFlag flag, bool value);
  void set_friction_combine_mode(PhysxCombineMode mode);
  void set_restitution_combine_mode(PhysxCombineMode mode);

  float get_dynamic_friction() const;
  float get_static_friction() const;
  float get_restitution() const;
  float get_dynamic_friction_v() const;
  float get_static_friction_v() const;
  LVector3f get_dir_of_anisotropy() const;
  bool get_flag(PhysxMaterialFlag flag) const;
  PhysxCombineMode get_friction_combine_mode() const;
  PhysxCombineMode get_restitution_combine_mode() const;

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

PUBLISHED:
  void release();

public:
  INLINE NxMaterial *ptr() const { return _ptr; };

  void link(NxMaterial *ptr);
  void unlink();

private:
  NxMaterial *_ptr;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxMaterial",
                  PhysxObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "physxMaterial.I"

#endif // PHYSXMATERIAL_H
