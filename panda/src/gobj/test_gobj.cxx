/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_gobj.cxx
 * @author shochet
 * @date 2000-02-02
 */

#include "geom.h"
#include "perspectiveProjection.h"

int main() {
  nout << "running test_gobj" << std::endl;
  PT(GeomTri) triangle = new GeomTri;
  Frustumf frust;
  PT(PerspectiveProjection) proj = new PerspectiveProjection(frust);
  LMatrix4f mat = proj->get_projection_mat();
  nout << "default proj matrix: " << mat;
  return 0;
}
