// Filename: test_gobj.cxx
// Created by:  shochet (02Feb00)
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

#include "geom.h"
#include "perspectiveProjection.h"

int main() {
  nout << "running test_gobj" << endl;
  PT(GeomTri) triangle = new GeomTri;
  Frustumf frust;
  PT(PerspectiveProjection) proj = new PerspectiveProjection(frust);
  LMatrix4f mat = proj->get_projection_mat();
  nout << "default proj matrix: " << mat;
  return 0;
}
