/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxCcdSkeleton.h
 * @author enn0x
 * @date 2012-05-01
 */

#ifndef PHYSXCCDSKELETON_H
#define PHYSXCCDSKELETON_H

#include "pandabase.h"

#include "physxObject.h"
#include "physx_includes.h"

/**
 * A Convex Mesh.  Internally represented as a list of convex polygons.  The
 * number of polygons is limited to 256.
 */
class EXPCL_PANDAPHYSX PhysxCcdSkeleton : public PhysxObject {

PUBLISHED:
  unsigned int get_reference_count() const;

PUBLISHED:
  void release();

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

public:
  INLINE PhysxCcdSkeleton();
  INLINE ~PhysxCcdSkeleton();

  INLINE NxCCDSkeleton *ptr() const { return _ptr; };

  void link(NxCCDSkeleton *meshPtr);
  void unlink();

private:
  NxCCDSkeleton *_ptr;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxCcdSkeleton",
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

#include "physxCcdSkeleton.I"

#endif // PHYSXCCDSKELETON_H
