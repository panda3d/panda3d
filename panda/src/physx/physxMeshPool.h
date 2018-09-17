/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxMeshPool.h
 * @author enn0x
 * @date 2009-10-14
 */

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
class PhysxClothMesh;
class PhysxSoftBodyMesh;

/**
 * This class unifies all references to the same filename, so that multiple
 * attempts to load the same mesh will return the same pointer.  The mesh
 * filename is automatically resolved before an attempt to load the mesh is
 * made.
 */
class EXPCL_PANDAPHYSX PhysxMeshPool {

PUBLISHED:
  INLINE PhysxMeshPool();
  INLINE ~PhysxMeshPool();

  static PhysxConvexMesh *load_convex_mesh(const Filename &filename);
  static PhysxTriangleMesh *load_triangle_mesh(const Filename &filename);
  static PhysxClothMesh *load_cloth_mesh(const Filename &filename);
  static PhysxSoftBodyMesh *load_soft_body_mesh(const Filename &filename);

  static bool release_convex_mesh(PhysxConvexMesh *mesh);
  static bool release_triangle_mesh(PhysxTriangleMesh *mesh);
  static bool release_cloth_mesh(PhysxClothMesh *mesh);
  static bool release_soft_body_mesh(PhysxSoftBodyMesh *mesh);

  static void list_contents();
  static void list_contents(std::ostream &out);

private:
  static bool check_filename(const Filename &fn);

  typedef pmap<Filename, PT(PhysxConvexMesh)> ConvexMeshes;
  static ConvexMeshes _convex_meshes;

  typedef pmap<Filename, PT(PhysxTriangleMesh)> TriangleMeshes;
  static TriangleMeshes _triangle_meshes;

  typedef pmap<Filename, PT(PhysxClothMesh)> ClothMeshes;
  static ClothMeshes _cloth_meshes;

  typedef pmap<Filename, PT(PhysxSoftBodyMesh)> SoftbodyMeshes;
  static SoftbodyMeshes _softbody_meshes;
};

#include "physxMeshPool.I"

#endif // PHYSXMESHPOOL_H
