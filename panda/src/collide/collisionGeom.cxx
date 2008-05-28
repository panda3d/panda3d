// Filename: collisionGeom.cxx
// Created by:  drose (01Mar06)
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

#include "collisionGeom.h"

PStatCollector CollisionGeom::_volume_pcollector("Collision Volumes:CollisionGeom");
PStatCollector CollisionGeom::_test_pcollector("Collision Tests:CollisionGeom");
TypeHandle CollisionGeom::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionGeom::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid *CollisionGeom::
make_copy() {
  return new CollisionGeom(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionGeom::get_volume_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of bounding volume tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionGeom::
get_volume_pcollector() {
  return _volume_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionGeom::get_test_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of intersection tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionGeom::
get_test_pcollector() {
  return _test_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionGeom::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionGeom::
output(ostream &out) const {
  out << "cgeom";
}
