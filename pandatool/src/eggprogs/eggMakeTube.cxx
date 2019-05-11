/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMakeTube.cxx
 * @author drose
 * @date 2003-10-01
 */

#include "eggMakeTube.h"
#include "eggGroup.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggPolygon.h"
#include "pointerTo.h"
#include "look_at.h"

/**
 *
 */
EggMakeTube::
EggMakeTube() {

  set_program_brief("generate a tube or sphere from geometry in an .egg file");
  set_program_description
    ("egg-make-tube generates an egg file representing a \"tube\" model, "
     "a cylinder capped on both ends by hemispheres.  This is similar "
     "in shape to the CollisionCapsule object within Panda.\n\n"
     "This program can also generate spheres if you omit -b; in this "
     "case, you are generating a degenerate tube of length 0.");

  add_option
    ("a", "x,y,z", 0,
     "Specify the first endpoint of the tube.",
     &EggWriter::dispatch_double_triple, nullptr, _point_a);

  add_option
    ("b", "x,y,z", 0,
     "Specify the second endpoint of the tube.",
     &EggWriter::dispatch_double_triple, &_got_point_b, _point_b);

  add_option
    ("r", "radius", 0,
     "Specify the radius of the tube.  The tube will extend beyond "
     "the endpoints in each direction by the amount of radius.",
     &EggWriter::dispatch_double, nullptr, &_radius);

  add_option
    ("slices", "count", 0,
     "Specify the number of slices appearing radially around the tube.",
     &EggWriter::dispatch_int, nullptr, &_num_slices);

  add_option
    ("crings", "count", 0,
     "Specify the number of rings appearing in each endcap of the tube.",
     &EggWriter::dispatch_int, nullptr, &_num_crings);

  add_option
    ("trings", "count", 0,
     "Specify the number of rings appearing in the cylindrical body "
     "of the tube.",
     &EggWriter::dispatch_int, nullptr, &_num_trings);

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

/**
 *
 */
void EggMakeTube::
run() {
  if (!_got_point_b) {
    _point_b[0] = _point_a[0];
    _point_b[1] = _point_a[1];
    _point_b[2] = _point_a[2];
  }

  // We will generate the vertices in the canonical space (along the y axis),
  // then transform it to the desired point.
  LVector3d direction(_point_b[0] - _point_a[0],
                      _point_b[1] - _point_a[1],
                      _point_b[2] - _point_a[2]);
  _length = direction.length();

  // First, create an enclosing group and a vertex pool.
  _group = new EggGroup("tube");
  _data->add_child(_group);

  _vpool = new EggVertexPool("tube");
  _group->add_child(_vpool);

  // Generate the first endcap.
  int ri, si;
  EggVertex *vtx_1;
  EggVertex *vtx_2;

  for (ri = 0; ri < _num_crings; ri++) {
    vtx_1 = nullptr;
    vtx_2 = nullptr;
    for (si = 0; si <= _num_slices; si++) {
      EggVertex *vtx_3 = calc_sphere1_vertex(ri, si);
      EggVertex *vtx_4 = calc_sphere1_vertex(ri + 1, si);
      add_polygon(vtx_1, vtx_2, vtx_4, vtx_3);
      vtx_1 = vtx_3;
      vtx_2 = vtx_4;
    }
  }

  // Now the cylinder sides.
  if (_length != 0.0) {
    for (ri = 0; ri < _num_trings; ri++) {
      vtx_1 = nullptr;
      vtx_2 = nullptr;
      for (si = 0; si <= _num_slices; si++) {
        EggVertex *vtx_3 = calc_tube_vertex(ri, si);
        EggVertex *vtx_4 = calc_tube_vertex(ri + 1, si);
        add_polygon(vtx_1, vtx_2, vtx_4, vtx_3);
        vtx_1 = vtx_3;
        vtx_2 = vtx_4;
      }
    }
  }

  // And the second endcap.
  for (ri = _num_crings - 1; ri >= 0; ri--) {
    vtx_1 = nullptr;
    vtx_2 = nullptr;
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

/**
 * Calculates a particular vertex on the surface of the first endcap
 * hemisphere.
 */
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

/**
 * Calculates a vertex on the side of the cylindrical body of the tube.
 */
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

/**
 * Calculates a particular vertex on the surface of the second endcap
 * hemisphere.
 */
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

/**
 * Adds the polygon defined by the indicated four vertices to the group.  If
 * the first vertex is NULL, does nothing.
 */
void EggMakeTube::
add_polygon(EggVertex *a, EggVertex *b, EggVertex *c, EggVertex *d) {
  if (a == nullptr) {
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
