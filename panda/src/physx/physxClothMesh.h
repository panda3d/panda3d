/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxClothMesh.h
 * @author enn0x
 * @date 2010-03-28
 */

#ifndef PHYSXCLOTHMESH_H
#define PHYSXCLOTHMESH_H

#include "pandabase.h"

#include "physxObject.h"
#include "physx_includes.h"

/**
 *
 */
class EXPCL_PANDAPHYSX PhysxClothMesh : public PhysxObject {

PUBLISHED:
  unsigned int get_reference_count() const;

PUBLISHED:
  void release();

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

public:
  INLINE PhysxClothMesh();
  INLINE ~PhysxClothMesh();

  INLINE NxClothMesh *ptr() const { return _ptr; };

  void link(NxClothMesh *meshPtr);
  void unlink();

private:
  NxClothMesh *_ptr;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxClothMesh",
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

#include "physxClothMesh.I"

#endif // PHYSXCLOTHMESH_H
