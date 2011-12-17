// Filename: physxSoftBodyMeshDesc.h
// Created by:  enn0x (12Sep10)
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

#ifndef PHYSXSOFTBODYMESHDESC_H
#define PHYSXSOFTBODYMESHDESC_H

#include "pandabase.h"
#include "luse.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxSoftBodyMeshDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSoftBodyMeshDesc {

PUBLISHED:
  INLINE PhysxSoftBodyMeshDesc();
  INLINE ~PhysxSoftBodyMeshDesc();

  INLINE bool is_valid() const;

  void set_num_vertices(unsigned int n);
  void set_vertex(unsigned int idx,
                  const LPoint3f &vert);

  void set_num_tetrahedra(unsigned int n);
  void set_tetrahedron(unsigned int idx,
                       unsigned int i1,
                       unsigned int i2,
                       unsigned int i3,
                       unsigned int i4);

public:
  INLINE const NxSoftBodyMeshDesc &get_desc() const;

private:
  NxSoftBodyMeshDesc _desc;
  NxVec3 *_vertices;
  NxU32 *_tetrahedra;
};

#include "physxSoftBodyMeshDesc.I"

#endif // PHYSXSOFTBODYMESHDESC_H
