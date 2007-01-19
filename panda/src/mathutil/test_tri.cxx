// Filename: test_tri.cxx
// Created by:  drose (19Jan07)
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

  return 0;
}
