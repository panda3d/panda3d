// Filename: collisionGeom.cxx
// Created by:  drose (01Mar06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
PStatCollector CollisionGeom::
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
PStatCollector CollisionGeom::
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
