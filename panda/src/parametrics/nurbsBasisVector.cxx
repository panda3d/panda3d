// Filename: nurbsBasisVector.cxx
// Created by:  drose (03Dec02)
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

#include "nurbsBasisVector.h"

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::clear
//       Access: Public
//  Description: Removes all the segments from the curve.
////////////////////////////////////////////////////////////////////
void NurbsBasisVector::
clear(int order) {
  _order = order;
  _segments.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::append_segment
//       Access: Public
//  Description: Computes a NURBS basis for one segment of the curve
//               and appends it to the set of basis matrices.
////////////////////////////////////////////////////////////////////
void NurbsBasisVector::
append_segment(int vertex_index, const PN_stdfloat knots[]) {
  int i;

  // Scale the supplied knots to the range 0..1.
  PN_stdfloat scaled_knots[8];
  PN_stdfloat min_k = knots[_order - 1];
  PN_stdfloat max_k = knots[_order];

  nassertv(min_k != max_k);
  for (i = 0; i < _order + _order; i++) {
    scaled_knots[i] = (knots[i] - min_k) / (max_k - min_k);
  }

  Segment segment;
  segment._vertex_index = vertex_index;
  segment._from = min_k;
  segment._to = max_k;

  for (i = 0; i < _order; i++) {
    LVecBase4 b = nurbs_blending_function(_order, i, _order, scaled_knots);
    segment._basis.set_col(i, b);
  }

  for (i = _order; i < 4; i++) {
    segment._basis.set_col(i, LVecBase4::zero());
  }

  _segments.push_back(segment);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::transpose
//       Access: Public
//  Description: Transposes the basis matrices stored in the vector.
////////////////////////////////////////////////////////////////////
void NurbsBasisVector::
transpose() {
  Segments::iterator si;
  for (si = _segments.begin(); si != _segments.end(); ++si) {
    (*si)._basis.transpose_in_place();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsBasisVector::nurbs_blending_function
//       Access: Private, Static
//  Description: Recursively computes the appropriate blending
//               function for the indicated knot vector.
////////////////////////////////////////////////////////////////////
LVecBase4 NurbsBasisVector::
nurbs_blending_function(int order, int i, int j, const PN_stdfloat knots[]) {
  // This is doubly recursive.  Ick.
  LVecBase4 r;

  if (j == 1) {
    if (i == order - 1 && knots[i] < knots[i + 1]) {
      r.set(0.0f, 0.0f, 0.0f, 1.0f);
    } else {
      r.set(0.0f, 0.0f, 0.0f, 0.0f);
    }

  } else {
    LVecBase4 bi0 = nurbs_blending_function(order, i, j - 1, knots);
    LVecBase4 bi1 = nurbs_blending_function(order, i + 1, j - 1, knots);

    PN_stdfloat d0 = knots[i + j - 1] - knots[i];
    PN_stdfloat d1 = knots[i + j] - knots[i + 1];

    // First term.  Division by zero is defined to equal zero.
    if (d0 != 0.0f) {
      if (d1 != 0.0f) {
        r = bi0 / d0 - bi1 / d1;
      } else {
        r = bi0 / d0;
      }

    } else if (d1 != 0.0f) {
      r = - bi1 / d1;

    } else {
      r.set(0.0f, 0.0f, 0.0f, 0.0f);
    }

    // scale by t.
    r[0] = r[1];
    r[1] = r[2];
    r[2] = r[3];
    r[3] = 0.0f;

    // Second term.
    if (d0 != 0.0f) {
      if (d1 != 0.0f) {
        r += bi0 * (- knots[i] / d0) + bi1 * (knots[i + j] / d1);
      } else {
        r += bi0 * (- knots[i] / d0);
      }

    } else if (d1 != 0.0f) {
      r += bi1 * (knots[i + j] / d1);
    }
  }

  return r;
}
