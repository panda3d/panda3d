// Filename: test_mathutil.cxx
// Created by:  drose (16Mar00)
// 
////////////////////////////////////////////////////////////////////

#include <luse.h>
#include "rotate_to.h"
#include "boundingLine.h"
#include "boundingSphere.h"

int
main() {
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

  BoundingLine line(LPoint3f(0, 0, 1), LPoint3f(0, 0, 0));
  
  BoundingSphere s1(LPoint3f(0, 0, 10), 1);
  BoundingSphere s2(LPoint3f(10, 0, 10), 1);
  BoundingSphere s3(LPoint3f(1, 0, 0), 1);
  BoundingSphere s4(LPoint3f(-1, -1, -1), 1);

  line.contains(&s1);
  line.contains(&s2);
  line.contains(&s3);
  line.contains(&s4);

  /*
  s1.contains(LPoint3f(0, 0, 1), LPoint3f(0, 0, 0));
  s2.contains(LPoint3f(0, 0, 1), LPoint3f(0, 0, 0));
  s3.contains(LPoint3f(0, 0, 1), LPoint3f(0, 0, 0));
  s4.contains(LPoint3f(0, 0, 1), LPoint3f(0, 0, 0));
  */

  return (0);
}
