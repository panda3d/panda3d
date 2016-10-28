/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTriangleMeshShape.cxx
 * @author enn0x
 * @date 2010-02-09
 */

#include "bulletTriangleMeshShape.h"
#include "bulletTriangleMesh.h"

#include "nodePathCollection.h"
#include "geomNode.h"
#include "geomVertexReader.h"

TypeHandle BulletTriangleMeshShape::_type_handle;

/**
 * Only used by make_from_bam.
 */
BulletTriangleMeshShape::
BulletTriangleMeshShape() :
  _mesh(NULL),
  _gimpact_shape(NULL),
  _bvh_shape(NULL),
  _dynamic(false),
  _compress(false),
  _bvh(false) {
}

/**
 * The parameters 'compress' and 'bvh' are only used if 'dynamic' is set to
 * FALSE.
 */
BulletTriangleMeshShape::
BulletTriangleMeshShape(BulletTriangleMesh *mesh, bool dynamic, bool compress, bool bvh) :
  _dynamic(dynamic),
  _compress(compress),
  _bvh(bvh) {

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

/**
 *
 */
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

/**
 *
 */
void BulletTriangleMeshShape::
refit_tree(const LPoint3 &aabb_min, const LPoint3 &aabb_max) {

  nassertv(!aabb_max.is_nan());
  nassertv(!aabb_max.is_nan());

  nassertv(this->is_static());

  _bvh_shape->refitTree(LVecBase3_to_btVector3(aabb_min),
                        LVecBase3_to_btVector3(aabb_max));
}

/**
 * Tells the BamReader how to create objects of type BulletTriangleMeshShape.
 */
void BulletTriangleMeshShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletTriangleMeshShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  BulletShape::write_datagram(manager, dg);

  dg.add_stdfloat(get_margin());

  manager->write_pointer(dg, _mesh);

  dg.add_bool(_dynamic);
  if (!_dynamic) {
    dg.add_bool(_compress);
    dg.add_bool(_bvh);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int BulletTriangleMeshShape::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = BulletShape::complete_pointers(p_list, manager);

  _mesh = DCAST(BulletTriangleMesh, p_list[pi++]);

  btStridingMeshInterface *mesh_ptr = _mesh->ptr();
  nassertr(mesh_ptr != NULL, pi);

  if (_dynamic) {
    _gimpact_shape = new btGImpactMeshShape(mesh_ptr);
    _gimpact_shape->updateBound();
    _gimpact_shape->setUserPointer(this);
    _gimpact_shape->setMargin(_margin);
  } else {
    _bvh_shape = new btBvhTriangleMeshShape(mesh_ptr, _compress, _bvh);
    _bvh_shape->setUserPointer(this);
    _bvh_shape->setMargin(_margin);
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *BulletTriangleMeshShape::
make_from_bam(const FactoryParams &params) {
  BulletTriangleMeshShape *param = new BulletTriangleMeshShape;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new BulletTriangleMeshShape.
 */
void BulletTriangleMeshShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  BulletShape::fillin(scan, manager);

  _margin = scan.get_stdfloat();

  manager->read_pointer(scan);

  _dynamic = scan.get_bool();
  if (!_dynamic) {
    _compress = scan.get_bool();
    _bvh = scan.get_bool();
  }
}
