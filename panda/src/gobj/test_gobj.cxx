// Filename: test_gobj.cxx
// Created by:  shochet (02Feb00)
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
