// Filename: physMeshHash.h
// Created by: enn0x (13Sep10)
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

#ifndef PHYSXMESHHASH_H
#define PHYSXMESHHASH_H

#include "config_physx.h"

#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxMeshHash
// Description : Utility class used in building links between a
//               tetrahedron mesh (soft body) and a triangle mesh
//               used for rendering the soft body.
////////////////////////////////////////////////////////////////////
class PhysxMeshHash {

public:
  PhysxMeshHash();
  ~PhysxMeshHash();

  void reset();

  void set_grid_spacing(float spacing);
  INLINE NxF32 get_grid_spacing() const;

  void add(const NxBounds3 &bounds, int itemIndex);
  void add(const NxVec3 &pos, int itemIndex);

  void query(const NxBounds3 &bounds, pvector<int> &itemIndices, int maxIndices=-1);
  void query_unique(const NxBounds3 &bounds, pvector<int> &itemIndices, int maxIndices=-1);

  void query(const NxVec3 &pos, pvector<int> &itemIndices, int maxIndices=-1);
  void query_unique(const NxVec3 &pos, pvector<int> &itemIndices, int maxIndices=-1);

private:
  struct MeshHashRoot {
    int first;
    int timeStamp;
  };

  struct MeshHashEntry {
    int itemIndex;
    int next;
  };

  NxI32 _time;
  NxF32 _spacing;
  NxF32 _invSpacing;

  static const int _hashIndexSize = 17011;
  MeshHashRoot _hashIndex[_hashIndexSize];

  pvector<MeshHashEntry> _entries;

  INLINE int hash_function(int xi, int yi, int zi) const;
  INLINE void cell_coord_of(const NxVec3 &v, int &xi, int &yi, int &zi) const;

  void compress_indices(pvector<int> &itemIndices);
  void quick_sort(pvector<int> &itemIndices, int l, int r);

};

#include "physxMeshHash.I"

#endif // PHYSXMESHHASH_H
