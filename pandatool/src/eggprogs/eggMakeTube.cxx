// Filename: eggMakeTube.cxx
// Created by:  drose (01Oct03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "eggMakeTube.h"
#include "eggGroup.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggPolygon.h"
#include "pointerTo.h"
#include "look_at.h"

////////////////////////////////////////////////////////////////////
//     Function: EggMakeTube::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggMakeTube::
EggMakeTube() {
  add_option
    ("a", "x,y,z", 0, 
     "Specify the first endpoint of the tube.",
     &EggWriter::dispatch_double_triple, NULL, _point_a);

  add_option
    ("b", "x,y,z", 0, 
     "Specify the second endpoint of the tube.",
     &EggWriter::dispatch_double_triple, NULL, _point_a);

  add_option
    ("r", "radius", 0, 
     "Specify the radius of the tube.  The tube will extend beyond "
     "the endpoints in each direction by the amount of radius.",
     &EggWriter::dispatch_double, NULL, &_radius);

  add_option
    ("slices", "count", 0, 
     "Specify the number of slices appearing radially around the tube.",
     &EggWriter::dispatch_int, NULL, &_num_slices);

  add_option
    ("crings", "count", 0, 
     "Specify the number of rings appearing in each endcap of the tube.",
     &EggWriter::dispatch_int, NULL, &_num_crings);

  add_option
    ("trings", "count", 0, 
     "Specify the number of rings appearing in the cylindrical body "
     "of the tube.",
     &EggWriter::dispatch_int, NULL, &_num_trings);

  _point_a[0] = 0.0;
  _point_a[1] = 0.0;
  _point_a[2] = 0.0;

  _point_b[0] = 0.0;
  _point_b[1] = 0.0;
  _point_b[2] = 0.0;

  _radius = 1.0;

  _num_slices = 8;
  _num_crings = 4;
  _num_trings = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeTube::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggMakeTube::
run() {
  // We will generate the vertices in the canonical space (along the y
  // axis), then transform it to the desired point.
  LVector3d direction(_point_b[0] - _point_a[0],
                      _point_b[1] - _point_a[1],
                      _point_b[2] - _point_a[2]);
  _length = direction.length();

  // First, create an enclosing group and a vertex pool.
  _group = new EggGroup("tube");
  _data.add_child(_group);

  _vpool = new EggVertexPool("tube");
  _group->add_child(_vpool);

  // Generate the first endcap.
  int ri, si;
  EggVertex *vtx_1;
  EggVertex *vtx_2;

  for (ri = 0; ri < _num_crings; ri++) {
    vtx_1 = NULL;
    vtx_2 = NULL;
    for (si = 0; si <= _num_slices; si++) {
      EggVertex *vtx_3 = calc_sphere1_vertex(ri, si);
      EggVertex *vtx_4 = calc_sphere1_vertex(ri + 1, si);
      add_polygon(vtx_1, vtx_2, vtx_4, vtx_3);
      vtx_1 = vtx_3;
      vtx_2 = vtx_4;
    }
  }

  // Now the cylinder sides.
  for (ri = 0; ri < _num_trings; ri++) {
    vtx_1 = NULL;
    vtx_2 = NULL;
    for (si = 0; si <= _num_slices; si++) {
      EggVertex *vtx_3 = calc_tube_vertex(ri, si);
      EggVertex *vtx_4 = calc_tube_vertex(ri + 1, si);
      add_polygon(vtx_1, vtx_2, vtx_4, vtx_3);
      vtx_1 = vtx_3;
      vtx_2 = vtx_4;
    }
  }

  // And the second endcap.
  for (ri = _num_crings - 1; ri >= 0; ri--) {
    vtx_1 = NULL;
    vtx_2 = NULL;
    for (si = 0; si <= _num_slices; si++) {
      EggVertex *vtx_3 = calc_sphere2_vertex(ri + 1, si);
      EggVertex *vtx_4 = calc_sphere2_vertex(ri, si);
      add_polygon(vtx_1, vtx_2, vtx_4, vtx_3);
      vtx_1 = vtx_3;
      vtx_2 = vtx_4;
    }
  }

  // Now transform the vertices out of the canonical position.
  LMatrix4d mat;
  look_at(mat, direction, LVector3d(0.0, 0.0, 1.0), CS_zup_right);
  mat.set_row(3, LPoint3d(_point_a[0], _point_a[1], _point_a[2]));
  _group->transform(mat);

  write_egg_file();
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeTube::calc_sphere1_vertex
//       Access: Private
//  Description: Calculates a particular vertex on the surface of the
//               first endcap hemisphere.
////////////////////////////////////////////////////////////////////
EggVertex *EggMakeTube::
calc_sphere1_vertex(int ri, int si) {
  double r = (double)ri / (double)_num_crings;
  double s = (double)si / (double)_num_slices;

  // Find the point on the rim, based on the slice.
  double theta = s * 2.0 * MathNumbers::pi;
  double x_rim = cos(theta);
  double z_rim = sin(theta);

  // Now pull that point in towards the pole, based on the ring.
  double phi = r * 0.5 * MathNumbers::pi;
  double to_pole = sin(phi);

  double x = _radius * x_rim * to_pole;
  double y = -_radius * cos(phi);
  double z = _radius * z_rim * to_pole;

  EggVertex vert;
  vert.set_pos(LPoint3d(x, y, z));

  return _vpool->create_unique_vertex(vert);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeTube::calc_tube_vertex
//       Access: Private
//  Description: Calculates a vertex on the side of the cylindrical
//               body of the tube.
////////////////////////////////////////////////////////////////////
EggVertex *EggMakeTube::
calc_tube_vertex(int ri, int si) {
  double r = (double)ri / (double)_num_trings;
  double s = (double)si / (double)_num_slices;

  // Find the point on the rim, based on the slice.
  double theta = s * 2.0 * MathNumbers::pi;
  double x_rim = cos(theta);
  double z_rim = sin(theta);

  double x = _radius * x_rim;
  double y = _length * r;
  double z = _radius * z_rim;

  EggVertex vert;
  vert.set_pos(LPoint3d(x, y, z));

  return _vpool->create_unique_vertex(vert);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeTube::calc_sphere2_vertex
//       Access: Private
//  Description: Calculates a particular vertex on the surface of the
//               second endcap hemisphere.
////////////////////////////////////////////////////////////////////
EggVertex *EggMakeTube::
calc_sphere2_vertex(int ri, int si) {
  double r = (double)ri / (double)_num_crings;
  double s = (double)si / (double)_num_slices;

  // Find the point on the rim, based on the slice.
  double theta = s * 2.0 * MathNumbers::pi;
  double x_rim = cos(theta);
  double z_rim = sin(theta);

  // Now pull that point in towards the pole, based on the ring.
  double phi = r * 0.5 * MathNumbers::pi;
  double to_pole = sin(phi);

  double x = _radius * x_rim * to_pole;
  double y = _length + _radius * cos(phi);
  double z = _radius * z_rim * to_pole;

  EggVertex vert;
  vert.set_pos(LPoint3d(x, y, z));

  return _vpool->create_unique_vertex(vert);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeTube::add_polygon
//       Access: Private
//  Description: Adds the polygon defined by the indicated four
//               vertices to the group.  If the first vertex is
//               NULL, does nothing.
////////////////////////////////////////////////////////////////////
void EggMakeTube::
add_polygon(EggVertex *a, EggVertex *b, EggVertex *c, EggVertex *d) {
  if (a == (EggVertex *)NULL) {
    return;
  }

  PT(EggPolygon) poly = new EggPolygon;
  poly->add_vertex(a);
  if (a != b) {
    poly->add_vertex(b);
  }
  poly->add_vertex(c);
  if (c != d) {
    poly->add_vertex(d);
  }

  _group->add_child(poly.p());
}


int main(int argc, char *argv[]) {
  EggMakeTube prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
