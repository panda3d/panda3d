// Filename: physxTriangleMeshShape.h
// Created by:  enn0x (14Oct09)
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

#ifndef PHYSXTRIANGLEMESHSHAPE_H
#define PHYSXTRIANGLEMESHSHAPE_H

#include "pandabase.h"

#include "physxShape.h"
#include "physx_includes.h"

class PhysxTriangleMeshShapeDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxTriangleMeshShape
// Description : A shapes which is used to represent an instance of
//               an convex mesh.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxTriangleMeshShape : public PhysxShape {

PUBLISHED:
  INLINE PhysxTriangleMeshShape();
  INLINE ~PhysxTriangleMeshShape();

  void save_to_desc(PhysxTriangleMeshShapeDesc &shapeDesc) const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxShape *ptr() const { return (NxShape *)_ptr; };

  void link(NxShape *shapePtr);
  void unlink();

private:
  NxTriangleMeshShape *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxShape::init_type();
    register_type(_type_handle, "PhysxTriangleMeshShape", 
                  PhysxShape::get_class_type());
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

#include "physxTriangleMeshShape.I"

#endif // PHYSXTRIANGLEMESHSHAPE_H
