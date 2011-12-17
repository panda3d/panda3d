// Filename: physxConvexMeshDesc.h
// Created by:  enn0x (11Oct09)
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

#ifndef PHYSXCONVEXMESHDESC_H
#define PHYSXCONVEXMESHDESC_H

#include "pandabase.h"
#include "luse.h"
#include "nodePath.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxConvexMeshDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxConvexMeshDesc {

PUBLISHED:
  INLINE PhysxConvexMeshDesc();
  INLINE ~PhysxConvexMeshDesc();

  INLINE bool is_valid() const;

  void set_num_vertices(unsigned int n);
  void set_vertex(unsigned int idx, const LPoint3f &vert);
  void set_from_node_path(const NodePath &np);

public:
  const NxConvexMeshDesc &get_desc() const;

private:
  NxVec3 *_vertices;
  NxConvexMeshDesc _desc;
};

#include "physxConvexMeshDesc.I"

#endif // PHYSXCONVEXMESHDESC_H
