// Filename: bulletSphereShape.cxx
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

#include "bulletSphereShape.h"

TypeHandle BulletSphereShape::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletSphereShape::
BulletSphereShape(PN_stdfloat radius) {

  _shape = new btSphereShape(radius);
  _shape->setUserPointer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::ptr
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
btCollisionShape *BulletSphereShape::
ptr() const {

  return _shape;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::make_from_solid
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
BulletSphereShape *BulletSphereShape::
make_from_solid(const CollisionSphere *solid) {

  return new BulletSphereShape(solid->get_radius());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               BulletShape.
////////////////////////////////////////////////////////////////////
void BulletSphereShape::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void BulletSphereShape::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_stdfloat(get_margin());
  dg.add_stdfloat(get_radius());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type BulletShape is encountered
//               in the Bam file.  It should create the BulletShape
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *BulletSphereShape::
make_from_bam(const FactoryParams &params) {
  BulletSphereShape *param = new BulletSphereShape;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSphereShape::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new BulletShape.
////////////////////////////////////////////////////////////////////
void BulletSphereShape::
fillin(DatagramIterator &scan, BamReader *manager) {
  nassertv(_shape == NULL);

  PN_stdfloat margin = scan.get_stdfloat();

  _shape = new btSphereShape(scan.get_stdfloat());
  _shape->setUserPointer(this);
  _shape->setMargin(margin);
}
