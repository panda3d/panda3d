// Filename: test_mathutil.cxx
// Created by:  drose (16Mar00)
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

#include "luse.h"
#include "rotate_to.h"
#include "boundingLine.h"
#include "boundingSphere.h"
#include "plane.h"


int
main() {
  LPlane p1(LVector3(1, 0, 0), LPoint3(1, 0, 0));
  LPlane p2(LVector3(0, 0, 1), LPoint3(1, 0, 0));

  LPoint3 from;
  LVector3 delta;

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
  BoundingLine line(LPoint3(0, 0, 1), LPoint3(0, 0, 0));

  BoundingSphere s1(LPoint3(0, 0, 10), 1);
  BoundingSphere s2(LPoint3(10, 0, 10), 1);
  BoundingSphere s3(LPoint3(1, 0, 0), 1);
  BoundingSphere s4(LPoint3(-1, -1, -1), 1);

  line.contains(&s1);
  line.contains(&s2);
  line.contains(&s3);
  line.contains(&s4);
  */

  /*
  s1.contains(LPoint3(0, 0, 1), LPoint3(0, 0, 0));
  s2.contains(LPoint3(0, 0, 1), LPoint3(0, 0, 0));
  s3.contains(LPoint3(0, 0, 1), LPoint3(0, 0, 0));
  s4.contains(LPoint3(0, 0, 1), LPoint3(0, 0, 0));
  */

  return (0);
}
