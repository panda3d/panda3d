// Filename: bulletPlaneShape.cxx
// Created by:  enn0x (23Jan10)
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

#include "bulletPlaneShape.h"

TypeHandle BulletPlaneShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletPlaneShape::
BulletPlaneShape(const LVector3 &normal, PN_stdfloat constant) {

  btVector3 btNormal = LVecBase3_to_btVector3(normal);

  _shape = new btStaticPlaneShape(btNormal, constant);
  _shape->setUserPointer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionShape *BulletPlaneShape::
ptr() const {

  return _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::make_from_solid
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
BulletPlaneShape *BulletPlaneShape::
make_from_solid(const CollisionPlane *solid) {

  LVector3 normal = solid->get_normal();
  PN_stdfloat constant = solid->dist_to_plane(LPoint3(0, 0, 0));

  return new BulletPlaneShape(normal, constant);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               BulletShape.
////////////////////////////////////////////////////////////////////
void BulletPlaneShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void BulletPlaneShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_stdfloat(get_margin());
  get_plane_normal().write_datagram(dg);
  dg.add_stdfloat(get_plane_constant());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type BulletShape is encountered
//               in the Bam file.  It should create the BulletShape
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *BulletPlaneShape::
make_from_bam(const FactoryParams &params) {
  BulletPlaneShape *param = new BulletPlaneShape;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletPlaneShape::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new BulletShape.
////////////////////////////////////////////////////////////////////
void BulletPlaneShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  nassertv(_shape == NULL);

  PN_stdfloat margin = scan.get_stdfloat();

  LVector3 normal;
  normal.read_datagram(scan);

  PN_stdfloat constant = scan.get_stdfloat();

  _shape = new btStaticPlaneShape(LVecBase3_to_btVector3(normal), constant);
  _shape->setUserPointer(this);
  _shape->setMargin(margin);
}

