/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletMinkowskiSumShape.cxx
 * @author enn0x
 * @date 2013-08-15
 */

#include "bulletMinkowskiSumShape.h"

#include "bulletWorld.h"

TypeHandle BulletMinkowskiSumShape::_type_handle;

/**
 *
 */
BulletMinkowskiSumShape::
BulletMinkowskiSumShape(const BulletShape *shape_a, const BulletShape *shape_b) :
  _shape_a(shape_a),
  _shape_b(shape_b) {

  nassertv(shape_a->is_convex());
  nassertv(shape_b->is_convex());

  const btConvexShape *ptr_a = (const btConvexShape *)shape_a->ptr();
  const btConvexShape *ptr_b = (const btConvexShape *)shape_b->ptr();

  _shape = new btMinkowskiSumShape(ptr_a, ptr_b);
  _shape->setUserPointer(this);
}

/**
 *
 */
BulletMinkowskiSumShape::
BulletMinkowskiSumShape(const BulletMinkowskiSumShape &copy) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _shape_a = copy._shape_a;
  _shape_b = copy._shape_b;
  
  const btConvexShape *ptr_a = (const btConvexShape *)_shape_a->ptr();
  const btConvexShape *ptr_b = (const btConvexShape *)_shape_b->ptr();

  _shape = new btMinkowskiSumShape(ptr_a, ptr_b);
  _shape->setUserPointer(this);
}

/**
 *
 */
btCollisionShape *BulletMinkowskiSumShape::
ptr() const {

  return _shape;
}

/**
 *
 */
void BulletMinkowskiSumShape::
set_transform_a(const TransformState *ts) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(ts);
  _shape->setTransformA(TransformState_to_btTrans(ts));
}

/**
 *
 */
void BulletMinkowskiSumShape::
set_transform_b(const TransformState *ts) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(ts);
  _shape->setTransformB(TransformState_to_btTrans(ts));
}

/**
 *
 */
CPT(TransformState) BulletMinkowskiSumShape::
get_transform_a() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btTrans_to_TransformState(_shape->getTransformA());
}

/**
 *
 */
CPT(TransformState) BulletMinkowskiSumShape::
get_transform_b() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btTrans_to_TransformState(_shape->GetTransformB());
}

/**
 * Tells the BamReader how to create objects of type BulletShape.
 */
void BulletMinkowskiSumShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletMinkowskiSumShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  BulletShape::write_datagram(manager, dg);
  dg.add_stdfloat(get_margin());

  // parameters to serialize: _shape_a, _shape_b, _transform_a, _transform_b
  manager->write_pointer(dg, _shape_a);
  manager->write_pointer(dg, _shape_b);
  manager->write_pointer(dg, get_transform_a());
  manager->write_pointer(dg, get_transform_b());
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int BulletMinkowskiSumShape::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = BulletShape::complete_pointers(p_list, manager);

  _shape_a = DCAST(BulletShape, p_list[pi++]);
  _shape_b = DCAST(BulletShape, p_list[pi++]);

  const TransformState *transform_a = DCAST(TransformState, p_list[pi++]);
  const TransformState *transform_b = DCAST(TransformState, p_list[pi++]);

  const btConvexShape *ptr_a = (const btConvexShape *)_shape_a->ptr();
  const btConvexShape *ptr_b = (const btConvexShape *)_shape_b->ptr();

  _shape = new btMinkowskiSumShape(ptr_a, ptr_b);
  _shape->setUserPointer(this);
  _shape->setMargin(_margin);

  set_transform_a(transform_a);
  set_transform_b(transform_b);

  return pi;
}

/**
 * Some objects require all of their nested pointers to have been completed
 * before the objects themselves can be completed.  If this is the case,
 * override this method to return true, and be careful with circular
 * references (which would make the object unreadable from a bam file).
 */
bool BulletMinkowskiSumShape::
require_fully_complete() const {
  // We require the shape pointers to be complete before we add them.
  return true;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *BulletMinkowskiSumShape::
make_from_bam(const FactoryParams &params) {
  // create a default BulletMinkowskiSumShape
  BulletMinkowskiSumShape *param = new BulletMinkowskiSumShape;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new BulletShape.
 */
void BulletMinkowskiSumShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  BulletShape::fillin(scan, manager);
  nassertv(_shape == nullptr);

  _margin = scan.get_stdfloat();

  // parameters to serialize: _shape_a, _shape_b, _transform_a, _transform_b
  manager->read_pointer(scan);
  manager->read_pointer(scan);
  manager->read_pointer(scan);
  manager->read_pointer(scan);
}
