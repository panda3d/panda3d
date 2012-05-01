// Filename: physxCcdSkeletonDesc.h
// Created by:  enn0x (01May12)
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

#ifndef PHYSXCCDSKELETONDESC_H
#define PHYSXCCDSKELETONDESC_H

#include "pandabase.h"
#include "nodePath.h"
#include "luse.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxCcdSkeletonDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxCcdSkeletonDesc {

PUBLISHED:
  INLINE PhysxCcdSkeletonDesc();
  INLINE ~PhysxCcdSkeletonDesc();

  INLINE bool is_valid() const;

  void set_num_vertices(unsigned int n);
  void set_vertex(unsigned int idx, const LPoint3f &vert);

  void set_num_triangles(unsigned int n);
  void set_triangle(unsigned int idx,
                    unsigned int i1, unsigned int i2, unsigned int i3);

  void set_from_node_path(const NodePath &np);

public:
  const NxSimpleTriangleMesh &get_desc() const;

private:
  NxVec3 *_vertices;
  NxU32 *_triangles;
  NxSimpleTriangleMesh _desc;
};

#include "physxCcdSkeletonDesc.I"

#endif // PHYSXCCDSKELETONDESC_H
