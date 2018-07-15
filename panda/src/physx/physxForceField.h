/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxForceField.h
 * @author enn0x
 * @date 2009-11-06
 */

#ifndef PHYSXFORCEFIELD_H
#define PHYSXFORCEFIELD_H

#include "pandabase.h"
#include "luse.h"

#include "physxObject.h"
#include "physx_includes.h"

class PhysxScene;
class PhysxForceFieldDesc;
class PhysxForceFieldShapeGroup;

/**
 * A force field effector.  Instances of this object automate the application
 * of forces onto rigid bodies, fluid, soft bodies and cloth.  Force fields
 * allow you to implement for example gusts of wind, dust devils, vacuum
 * cleaners or anti-gravity zones.
 */
class EXPCL_PANDAPHYSX PhysxForceField : public PhysxObject {

PUBLISHED:
  INLINE PhysxForceField();
  INLINE ~PhysxForceField();

  // void load_from_desc(const PhysxForceFieldDesc &materialDesc); void
  // save_to_desc(PhysxForceFieldDesc &materialDesc) const;

  void set_name(const char *name);

  const char *get_name() const;
  PhysxScene *get_scene() const;
  PhysxForceFieldShapeGroup *get_include_shape_group() const;

  unsigned int get_num_shape_groups() const;
  PhysxForceFieldShapeGroup *get_shape_group(unsigned int idx) const;
  MAKE_SEQ(get_shape_groups, get_num_shape_groups, get_shape_group);

PUBLISHED:
  void release();

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

public:
  INLINE NxForceField *ptr() const { return _ptr; };

  void link(NxForceField *ptr);
  void unlink();

private:
  NxForceField *_ptr;
  std::string _name;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxForceField",
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

#include "physxForceField.I"

#endif // PHYSXFORCEFIELD_H
