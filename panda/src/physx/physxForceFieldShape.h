// Filename: physxForceFieldShape.h
// Created by:  enn0x (15Nov09)
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

#ifndef PHYSXFORCEFIELDSHAPE_H
#define PHYSXFORCEFIELDSHAPE_H

#include "pandabase.h"
#include "pointerTo.h"
#include "luse.h"

#include "physxObject.h"
#include "physxEnums.h"
#include "physx_includes.h"

class PhysxForceField;
class PhysxForceFieldShapeGroup;

////////////////////////////////////////////////////////////////////
//       Class : PhysxForceFieldShape
// Description : Abstract base class for force field shapes.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxForceFieldShape : public PhysxObject, public PhysxEnums {

PUBLISHED:
  void release();

  PhysxForceField *get_force_field() const;
  PhysxForceFieldShapeGroup *get_shape_group() const;

  void set_name(const char *name);
  void set_mat(const LMatrix4f &mat);
  void set_pos(const LPoint3f &pos);

  const char *get_name() const;
  LMatrix4f get_mat() const;
  LPoint3f get_pos() const;

  INLINE void ls() const;
  INLINE void ls(ostream &out, int indent_level=0) const;

public:
  static PhysxForceFieldShape *factory(NxShapeType shapeType);

  virtual NxForceFieldShape *ptr() const = 0;

  virtual void link(NxForceFieldShape *shapePtr) = 0;
  virtual void unlink() = 0;

protected:
  INLINE PhysxForceFieldShape();

private:
  string _name;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxForceFieldShape", 
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

#include "physxForceFieldShape.I"

#endif // PHYSXFORCEFIELDSHAPE_H
