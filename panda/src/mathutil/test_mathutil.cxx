// Filename: test_mathutil.cxx
// Created by:  drose (16Mar00)
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

#include "luse.h"
#include "rotate_to.h"
#include "boundingLine.h"
#include "boundingSphere.h"
#include "plane.h"


int
main() {
  Planef p1(LVector3f(1, 0, 0), LPoint3f(1, 0, 0));
  Planef p2(LVector3f(0, 0, 1), LPoint3f(1, 0, 0));

  LPoint3f from;
  LVector3f delta;

  if (p1.intersects_plane(from, delta, p2)) {
    nout << "intersects, " << from << " to " << delta << "\n";
  } else {
    nout << "no intersect\n";
  }

  /*
  LVector3d a(1.0f, 0.0f, 0.0f);
  LVector3d b = normalize(LVector3d(0.5, 0.5, 0.0f));

  LMatrix3d rot;
  rotate_to(rot, a, b);

  nout << "a = " << a << "\n"
       << "b = " << b << "\n"
       << "rot =\n";
  rot.write(nout, 2);
  nout << "a * rot = " << a * rot << " length " << length(a * rot) << "\n"
       << "a * invert(rot) = " << a * invert(rot)
       << " length " << length(a * invert(rot)) << "\n"
       << "b * rot = " << b * rot
       << " length " << length(b * rot) << "\n"
       << "b * invert(rot) = " << b * invert(rot)
       << " length " << length(a * invert(rot)) << "\n";
  */

  /*
  BoundingLine line(LPoint3f(0, 0, 1), LPoint3f(0, 0, 0));

  BoundingSphere s1(LPoint3f(0, 0, 10), 1);
  BoundingSphere s2(LPoint3f(10, 0, 10), 1);
  BoundingSphere s3(LPoint3f(1, 0, 0), 1);
  BoundingSphere s4(LPoint3f(-1, -1, -1), 1);

  line.contains(&s1);
  line.contains(&s2);
  line.contains(&s3);
  line.contains(&s4);
  */

  /*
  s1.contains(LPoint3f(0, 0, 1), LPoint3f(0, 0, 0));
  s2.contains(LPoint3f(0, 0, 1), LPoint3f(0, 0, 0));
  s3.contains(LPoint3f(0, 0, 1), LPoint3f(0, 0, 0));
  s4.contains(LPoint3f(0, 0, 1), LPoint3f(0, 0, 0));
  */

  return (0);
}
