// Filename: triangulator3.h
// Created by:  drose (03Jan13)
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

#ifndef TRIANGULATOR3_H
#define TRIANGULATOR3_H

#include "pandabase.h"
#include "triangulator.h"
#include "plane.h"


////////////////////////////////////////////////////////////////////
//       Class : Triangulator3
// Description : This is an extension of Triangulator to handle
//               polygons with three-dimensional points.  It assumes
//               all of the points lie in a single plane, and
//               internally projects the supplied points into 2-D for
//               passing to the underlying Triangulator object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MATHUTIL Triangulator3 : public Triangulator {
PUBLISHED:
  Triangulator3();

  void clear();
  int add_vertex(const LPoint3d &point);
  INLINE int add_vertex(double x, double y, double z);

  INLINE int get_num_vertices() const;
  INLINE const LPoint3d &get_vertex(int n) const;
  MAKE_SEQ(get_vertices, get_num_vertices, get_vertex);

  void triangulate();
  INLINE const LPlaned &get_plane() const;

private:
  typedef pvector<LPoint3d> Vertices3;
  Vertices3 _vertices3;

  LPlaned _plane;
};

#include "triangulator3.I"

#endif
