// Filename: bulletTriangleMeshShape.cxx
// Created by:  enn0x (09Feb10)
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

#include "bulletTriangleMeshShape.h"
#include "bulletTriangleMesh.h"

#include "nodePathCollection.h"
#include "geomNode.h"
#include "geomVertexReader.h"

TypeHandle BulletTriangleMeshShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMeshShape::Constructor
//       Access: Published
//  Description: The parameters 'compress' and 'bvh' are only used
//               if 'dynamic' is set to FALSE.
////////////////////////////////////////////////////////////////////
BulletTriangleMeshShape::
BulletTriangleMeshShape(BulletTriangleMesh *mesh, bool dynamic, bool compress, bool bvh) {

  // Assert that mesh is not NULL
  if (!mesh) {
    bullet_cat.warning() << "mesh is NULL! creating new mesh." << endl;
    mesh = new BulletTriangleMesh();
  }

  // Assert that mesh has at least one triangle
  if (mesh->get_num_triangles() == 0) {
    bullet_cat.warning() << "mesh has zero triangles! adding degenerated triangle." << endl;
    mesh->add_triangle(LPoint3::zero(), LPoint3::zero(), LPoint3::zero());
  }

  // Retain a pointer to the mesh, to prevent it from being deleted
  _mesh = mesh;

  // Dynamic will create a GImpact mesh shape
  if (dynamic) {

    _gimpact_shape = new btGImpactMeshShape(mesh->ptr());
    _gimpact_shape->updateBound();
    _gimpact_shape->setUserPointer(this);

    _bvh_shape = NULL;
  }

  // Static will create a Bvh mesh shape
  else {

    _bvh_shape = new btBvhTriangleMeshShape(mesh->ptr(), compress, bvh);
    _bvh_shape->setUserPointer(this);

    _gimpact_shape = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMeshShape::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionShape *BulletTriangleMeshShape::
ptr() const {

  if (_bvh_shape) {
    return _bvh_shape;
  }

  if (_gimpact_shape) {
    return _gimpact_shape;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMeshShape::refit_tree
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletTriangleMeshShape::
refit_tree(const LPoint3 &aabb_min, const LPoint3 &aabb_max) {

  nassertv(!aabb_max.is_nan());
  nassertv(!aabb_max.is_nan());

  nassertv(this->is_static());

  _bvh_shape->refitTree(LVecBase3_to_btVector3(aabb_min),
                        LVecBase3_to_btVector3(aabb_max));
}

