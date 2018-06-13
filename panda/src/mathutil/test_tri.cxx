/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_tri.cxx
 * @author drose
 * @date 2007-01-19
 */

#include "pandabase.h"

#include "triangulator.h"

int main(int argc, char *argv[]) {
  Triangulator t;

  t.add_vertex(0, 0);
  t.add_vertex(4, 0);
  t.add_vertex(4, 4);
  t.add_vertex(0, 4);

  t.add_vertex(1, 1);
  t.add_vertex(2, 1);
  t.add_vertex(2, 2);
  t.add_vertex(1, 2);

  t.add_polygon_vertex(0);
  t.add_polygon_vertex(1);
  t.add_polygon_vertex(2);
  t.add_polygon_vertex(3);

  t.begin_hole();
  t.add_hole_vertex(7);
  t.add_hole_vertex(6);
  t.add_hole_vertex(5);
  t.add_hole_vertex(4);

  t.triangulate();

  for (int i = 0; i < t.get_num_triangles(); ++i) {
    std::cerr << "tri: " << t.get_triangle_v0(i) << " "
         << t.get_triangle_v1(i) << " "
         << t.get_triangle_v2(i) << "\n";
  }

  return 0;
}
