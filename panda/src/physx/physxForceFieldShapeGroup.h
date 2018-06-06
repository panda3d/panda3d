/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxForceFieldShapeGroup.h
 * @author enn0x
 * @date 2009-11-11
 */

#ifndef PHYSXFORCEFIELDSHAPEGROUP_H
#define PHYSXFORCEFIELDSHAPEGROUP_H

#include "pandabase.h"

#include "physxObject.h"
#include "physxEnums.h"

#include "physx_includes.h"

class PhysxScene;
class PhysxForceField;
class PhysxForceFieldShape;
class PhysxForceFieldShapeDesc;
class PhysxForceFieldShapeGroupDesc;

/**
 *
 */
class EXPCL_PANDAPHYSX PhysxForceFieldShapeGroup : public PhysxObject, public PhysxEnums {

PUBLISHED:
  INLINE PhysxForceFieldShapeGroup();
  INLINE ~PhysxForceFieldShapeGroup();

  void save_to_desc(PhysxForceFieldShapeGroupDesc &groupDesc) const;

  PhysxScene *get_scene() const;
  PhysxForceField *get_force_field() const;
  const char *get_name() const;

  void set_name(const char *name);

  // Shapes
  unsigned int get_num_shapes() const;
  PhysxForceFieldShape *create_shape(PhysxForceFieldShapeDesc &desc);
  PhysxForceFieldShape *get_shape(unsigned int idx) const;
  MAKE_SEQ(get_shapes, get_num_shapes, get_shape);

PUBLISHED:
  void release();

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

public:
  INLINE NxForceFieldShapeGroup *ptr() const { return _ptr; };

  void link(NxForceFieldShapeGroup *ptr);
  void unlink();

  PhysxObjectCollection<PhysxForceFieldShape> _shapes;

private:
  NxForceFieldShapeGroup *_ptr;
  std::string _name;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxForceFieldShapeGroup",
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

#include "physxForceFieldShapeGroup.I"

#endif // PHYSXFORCEFIELDSHAPEGROUP_H
