// Filename: physxMeshPool.h
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

#ifndef PHYSXMESHPOOL_H
#define PHYSXMESHPOOL_H

#include "pandabase.h"
#include "pointerTo.h"
#include "pnotify.h"
#include "pmap.h"
#include "filename.h"

#include "physx_includes.h"

class PhysxConvexMesh;
class PhysxTriangleMesh;

////////////////////////////////////////////////////////////////////
//       Class : PhysxMeshPool
// Description : This class unifies all references to the same
//               filename, so that multiple attempts to load the
//               same mesh will return the same pointer.
//               The mesh filename is automatically resolved before
//               an attempt to load the mesh is made.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxMeshPool {

PUBLISHED:
  INLINE PhysxMeshPool();
  INLINE ~PhysxMeshPool();

  static PhysxConvexMesh *load_convex_mesh(const Filename &filename);
  static PhysxTriangleMesh *load_triangle_mesh(const Filename &filename);

  static bool release_convex_mesh(PhysxConvexMesh *mesh);
  static bool release_triangle_mesh(PhysxTriangleMesh *mesh);

  static void list_contents();
  static void list_contents(ostream &out);

private:
  static bool prepare_filename(Filename &fn);

  typedef pmap<Filename, PT(PhysxConvexMesh)> ConvexMeshes;
  typedef pmap<Filename, PT(PhysxTriangleMesh)> TriangleMeshes;
  //typedef pmap<Filename, PT(PhysxClothMesh)> ClothMeshes;
  //typedef pmap<Filename, PT(PhysxSoftBodyMesh)> SoftbodyMeshes;

  static ConvexMeshes _convex_meshes;
  static TriangleMeshes _triangle_meshes;
  //static ClothMeshes _cloth_meshes;
  //static SoftbodyMeshes _softbody_meshes;
};

#include "physxMeshPool.I"

#endif // PHYSXMESHPOOL_H
