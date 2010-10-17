// Filename: physxKitchen.h
// Created by:  enn0x (12Oct09)
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

#ifndef PHYSXKITCHEN_H
#define PHYSXKITCHEN_H

#include "pandabase.h"
#include "filename.h"

#include "physx_includes.h"

class PhysxConvexMesh;
class PhysxConvexMeshDesc;
class PhysxTriangleMesh;
class PhysxTriangleMeshDesc;
class PhysxClothMesh;
class PhysxClothMeshDesc;
class PhysxSoftBodyMesh;
class PhysxSoftBodyMeshDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxKitchen
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxKitchen {

PUBLISHED:
  INLINE PhysxKitchen();
  INLINE ~PhysxKitchen();

  void set_cooking_params(float skinWidth, bool hintCollisionSpeed);

  bool cook_convex_mesh(const PhysxConvexMeshDesc &meshDesc, const Filename &filename);
  bool cook_triangle_mesh(const PhysxTriangleMeshDesc &meshDesc, const Filename &filename);
  bool cook_cloth_mesh(const PhysxClothMeshDesc &meshDesc, const Filename &filename);
  bool cook_soft_body_mesh(const PhysxSoftBodyMeshDesc &meshDesc, const Filename &filename);
  bool cook_texcoords(const PhysxClothMeshDesc &meshDesc, const Filename &filename);

  PhysxConvexMesh *cook_convex_mesh(const PhysxConvexMeshDesc &meshDesc);
  PhysxTriangleMesh *cook_triangle_mesh(const PhysxTriangleMeshDesc &meshDesc);
  PhysxClothMesh *cook_cloth_mesh(const PhysxClothMeshDesc &meshDesc);
  PhysxSoftBodyMesh *cook_soft_body_mesh(const PhysxSoftBodyMeshDesc &meshDesc);

private:
  NxCookingInterface *_cooking;
};

#include "physxKitchen.I"

#endif // PHYSXKITCHEN_H
