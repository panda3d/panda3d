/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionGeom.cxx
 * @author drose
 * @date 2006-03-01
 */

#include "collisionGeom.h"

PStatCollector CollisionGeom::_volume_pcollector("Collision Volumes:CollisionGeom");
PStatCollector CollisionGeom::_test_pcollector("Collision Tests:CollisionGeom");
TypeHandle CollisionGeom::_type_handle;

/**
 *
 */
CollisionSolid *CollisionGeom::
make_copy() {
  return new CollisionGeom(*this);
}

/**
 * Returns a PStatCollector that is used to count the number of bounding
 * volume tests made against a solid of this type in a given frame.
 */
PStatCollector &CollisionGeom::
get_volume_pcollector() {
  return _volume_pcollector;
}

/**
 * Returns a PStatCollector that is used to count the number of intersection
 * tests made against a solid of this type in a given frame.
 */
PStatCollector &CollisionGeom::
get_test_pcollector() {
  return _test_pcollector;
}

/**
 *
 */
void CollisionGeom::
output(std::ostream &out) const {
  out << "cgeom";
}
