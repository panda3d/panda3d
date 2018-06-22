/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_math.cxx
 * @author drose
 * @date 1999-01-14
 */

#include "luse.h"
#include "lmatrix.h"
#include "compose_matrix.h"
#include "pstrtod.h"

#include "pnotify.h"
#include <stdlib.h>

using std::cerr;
using std::cout;
using std::endl;

void test() {
  LMatrix4f x = LMatrix4f::ident_mat();
  LMatrix4f y = LMatrix4f::ident_mat();

  LMatrix4f z = x * y;
}

int main(int argc, char *argv[]) {
  test();

  {
    LOrientationf orientation(LMatrix4f::rotate_mat(-45.0f, LVector3f(0, 0, 1)));
    LRotationf rotation(LMatrix4f::rotate_mat(45.0f, LVector3f(0, 0, 1)));

    nout << "Orientation: " << orientation << endl;
    nout << "Rotation: " << rotation << endl;

    LQuaternionf composition = orientation * rotation;

    nout << "Composition (o * r): " << composition << endl;
    composition.normalize();
    nout << "Composition (after normalize): " << composition << endl;

    LPoint3f p(1, 0, 0);
    LMatrix4f m = LMatrix4f::ident_mat();

    composition.extract_to_matrix(m);
    cout << "Rotation => Matrix: " << m << endl;
    cout << "Point: " << p << endl;
    cout << "Point * Matrix: " << p * m << endl;

  }
  {

    LMatrix4d x = LMatrix4d::ident_mat();
    LMatrix4d y = LMatrix4d::rotate_mat(90.0, LVector3d::up());
    LMatrix4d a = LMatrix4d::translate_mat(10.0, 10.0, 0.0f);

    nout << "x is " << x << "\ny is " << y << "\n"
         << "x * y is " << x * y << "\n"
         << "y * x is " << y * x << "\n"
         << "invert(x) is " << invert(x) << "\n"
         << "invert(y) is " << invert(y) << "\n"
         << "y * a is " << y * a << "\n"
         << "invert(y * a) is " << invert(y * a) << "\n"
         << "invert(invert(y * a)) is " << invert(invert(y * a)) << "\n"
         << "(y * a) * invert(y * a) is " << (y * a) * invert(y * a) << "\n"
         << "a * y is " << a * y << "\n"
         << "invert(a * y) is " << invert(a * y) << "\n"
         << "invert(invert(a * y)) is " << invert(invert(a * y)) << "\n";

    nout << "a is " << a << "\n"
         << "a * y is " << a * y << "\n"
         << "y * a is " << y * a << "\n";

    LVector3d r = LVector3d::right();
    nout << "r is " << r << "\n"
         << "r * x is " << r * x << "\n"
         << "r * y is " << r * y << "\n"
         << "r * invert(y) is " << r * invert(y) << "\n"
         << "r * a is " << r * a << "\n";

    LPoint3d p(0.0f, 1.0f, 1.0f);
    nout << "p is " << p << "\n"
         << "p * x is " << p * x << "\n"
         << "p * y is " << p * y << "\n"
         << "p * invert(y) is " << p * invert(y) << "\n"
         << "p * a is " << p * a << "\n";

    LVecBase4d q(0.0f, 1.0f, 1.0f, 1.0f);
    nout << "q is " << q << "\n"
         << "q * x is " << q * x << "\n"
         << "q * y is " << q * y << "\n"
         << "q * invert(y) is " << q * invert(y) << "\n"
         << "q * a is " << q * a << "\n";

    LNormald v1(0,0,1), v2(1,1,1);
    LVertexd p1(1,0,1), p2(1,2,3);
    LColorf c1(1,1,1,1), c2(0,0,0,0);

    p2 = p2 - v1;

    nout << "v1 = " << v1
         << "\nv2 = " << v2
         << "\np1 = " << p1
         << "\np2 = " << p2
         << "\nc1 = " << c1
         << "\n(c1 == c2) = " << (c1 == c2)
         << "\n";

    {
      LVecBase3f hpr(0.0f, 0.0f, 0.0f);
      LVecBase3f scale(1.0f, 1.0f, 1.0f);

      if (argc > 3) {
        hpr.set(patof(argv[1]), patof(argv[2]), patof(argv[3]));
      }
      if (argc > 6) {
        scale.set(patof(argv[4]), patof(argv[5]), patof(argv[6]));
      }

      cerr << "< hpr = " << hpr << " scale = " << scale << "\n";
      LMatrix3f mat;
      compose_matrix(mat, scale, hpr);

      if (decompose_matrix(mat, scale, hpr)) {
        nout << "> hpr = " << hpr << " scale = " << scale << "\n";
      } else {
        nout << "Cannot decompose\n";
      }
    }

  }
  {
    for (int p = -90; p < 90; p += 10) {
      for (int x = -10; x < 10; x += 5) {
        LVecBase3f hpr(0, p, 0);
        LVecBase3f xyz(x, x, x);
        LVecBase3f scale(1, 1, 1);
        nout << "\n< hpr = " << hpr << " xyz = " << xyz << "\n";
        LMatrix4f mat;
        compose_matrix(mat, scale, hpr, xyz);
        if (decompose_matrix(mat, scale, hpr, xyz)) {
          nout << "> hpr = " << hpr << " xyz = " << xyz << "\n";
        } else {
          nout << "Cannot decompose\n";
        }
      }
    }
  }

  {
    LMatrix3f mat(1, 2, 3,
                  4, 5, 6,
                  7, 8, 9);
    nout << "Matrix:";
    for (LMatrix3f::iterator mi = mat.begin(); mi != mat.end(); ++mi) {
      nout << " " << *mi;
    }
    nout << "\n";
  }

  return(0);
}
