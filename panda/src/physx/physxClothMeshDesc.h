// Filename: physxClothMeshDesc.h
// Created by:  enn0x (28Mar10)
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

#ifndef PHYSXCLOTHMESHDESC_H
#define PHYSXCLOTHMESHDESC_H

#include "pandabase.h"
#include "luse.h"
#include "nodePath.h"
#include "plist.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxClothMeshDesc
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxClothMeshDesc {

PUBLISHED:
  INLINE PhysxClothMeshDesc();
  INLINE ~PhysxClothMeshDesc();

  INLINE bool is_valid() const;

  void set_num_vertices(unsigned int n);
  void set_vertex(unsigned int idx,
                  const LPoint3f &vert, const LPoint2f &texcoord);

  void set_num_triangles(unsigned int n);
  void set_triangle(unsigned int idx,
                    unsigned int i1, unsigned int i2, unsigned int i3);

  void set_from_node_path(const NodePath &np);

public:
  INLINE const NxClothMeshDesc &get_desc() const;
  INLINE const plist<LPoint2f> get_texcoords() const;

private:
  NxClothMeshDesc _desc;
  NxVec3 *_points;
  NxU32 *_triangles;

  LPoint2f *_texcoords;
};

#include "physxClothMeshDesc.I"

#endif // PHYSXCLOTHMESHDESC_H
