// Filename: nurbsCurveResult.cxx
// Created by:  drose (04Dec02)
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

#include "nurbsCurveResult.h"


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::Constructor
//       Access: Public
//  Description: The constructor automatically builds up the result as
//               the product of the indicated set of basis matrices
//               and the indicated table of control vertex positions.
////////////////////////////////////////////////////////////////////
NurbsCurveResult::
NurbsCurveResult(const NurbsMatrixVector &basis,
                 const LVecBase4f vecs[], const NurbsVertex *verts,
                 int num_vertices) :
  _basis(basis),
  _verts(verts)
{
  _last_segment = -1;
  int order = _basis.get_order();
  int num_segments = _basis.get_num_segments();

  _composed.reserve(num_segments);
  for (int i = 0; i < num_segments; i++) {
    int vi = _basis.get_vertex_index(i);
    nassertv(vi >= 0 && vi < num_vertices);

    // Create a geometry matrix from our (up to) four involved vertices.
    LMatrix4f geom;
    int ci = 0;
    while (ci < order) {
      geom.set_row(ci, vecs[vi + ci]);
      ci++;
    }
    while (ci < 4) {
      geom.set_row(ci, LVecBase4f::zero());
      ci++;
    }

    // And compose this geometry matrix with the basis matrix to
    // produce a new matrix, which will be used to evaluate the curve.
    LMatrix4f result;
    result.multiply(_basis.get_basis(i), geom);
    _composed.push_back(result);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::eval_segment_point
//       Access: Published
//  Description: Evaluates the point on the curve corresponding to the
//               indicated value in parametric time within the
//               indicated curve segment.  t should be in the range
//               [0, 1].
//
//               The curve is internally represented as a number of
//               connected (or possibly unconnected) piecewise
//               continuous segments.  The exact number of segments
//               for a particular curve depends on the knot vector,
//               and is returned by get_num_segments().  Normally,
//               eval_point() is used to evaluate a point along the
//               continuous curve, but when you care more about local
//               continuity, you can use eval_segment_point() to
//               evaluate the points along each segment.
////////////////////////////////////////////////////////////////////
void NurbsCurveResult::
eval_segment_point(int segment, float t, LVecBase3f &point) const {
  float t2 = t*t;
  LVecBase4f tvec(t*t2, t2, t, 1.0f);

  float weight = tvec.dot(_composed[segment].get_col(3));
  point.set(tvec.dot(_composed[segment].get_col(0)) / weight,
            tvec.dot(_composed[segment].get_col(1)) / weight,
            tvec.dot(_composed[segment].get_col(2)) / weight);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::eval_segment_tangent
//       Access: Published
//  Description: As eval_segment_point, but computes the tangent to
//               the curve at the indicated point.  The tangent vector
//               will not necessarily be normalized, and could be
//               zero, particularly at the endpoints.
////////////////////////////////////////////////////////////////////
void NurbsCurveResult::
eval_segment_tangent(int segment, float t, LVecBase3f &tangent) const {
  float t2 = t*t;
  LVecBase4f tvec(t2, t, 1.0f, 0.0f);

  tangent.set(tvec.dot(_composed[segment].get_col(0)),
              tvec.dot(_composed[segment].get_col(1)),
              tvec.dot(_composed[segment].get_col(2)));
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::eval_segment_extended_point
//       Access: Published
//  Description: Evaluates the curve in n-dimensional space according
//               to the extended vertices associated with the curve in
//               the indicated dimension.
////////////////////////////////////////////////////////////////////
float NurbsCurveResult::
eval_segment_extended_point(int segment, float t, int d) const {
  nassertr(segment >= 0 && segment < _basis.get_num_segments(), 0.0f);

  int order = _basis.get_order();
  int vi = _basis.get_vertex_index(segment);

  LVecBase4f geom;
  int ci = 0;
  while (ci < order) {
    geom[ci] = _verts[vi + ci].get_extended_vertex(d);
    ci++;
  }
  while (ci < 4) {
    geom[ci] = 0.0f;
    ci++;
  }

  const LMatrix4f &basis = _basis.get_basis(segment);

  // Compute matrix * column vector.
  LVecBase4f composed_geom(basis.get_row(0).dot(geom),
                           basis.get_row(1).dot(geom),
                           basis.get_row(2).dot(geom),
                           basis.get_row(3).dot(geom));

  float t2 = t*t;
  LVecBase4f tvec(t*t2, t2, t, 1.0f);

  float weight = tvec.dot(_composed[segment].get_col(3));

  float result = tvec.dot(composed_geom) / weight;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::find_segment
//       Access: Private
//  Description: Returns the index of the segment that contains the
//               indicated value of t, or -1 if no segment contains
//               this value.
////////////////////////////////////////////////////////////////////
int NurbsCurveResult::
find_segment(float t) {
  // Trivially check the endpoints of the curve.
  if (t >= get_end_t()) {
    return _basis.get_num_segments() - 1;
  } else if (t <= get_start_t()) {
    return 0;
  }

  // Check the last segment we searched for.  Often, two consecutive
  // requests are for the same segment.
  if (_last_segment != -1 && (t >= _last_from && t < _last_to)) {
    return _last_segment;
  }

  // Look for the segment the hard way.
  int segment = r_find_segment(t, 0, _basis.get_num_segments() - 1);
  if (segment != -1) {
    _last_segment = segment;
    _last_from = _basis.get_from(segment);
    _last_to = _basis.get_to(segment);
  }
  return segment;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::r_find_segment
//       Access: Private
//  Description: Recursively searches for the segment that contains
//               the indicated value of t by performing a binary
//               search.  This assumes the segments are stored in
//               increasing order of t, and they don't overlap.
////////////////////////////////////////////////////////////////////
int NurbsCurveResult::
r_find_segment(float t, int top, int bot) const {
  if (bot < top) {
    // Not found.
    return -1;
  }
  int mid = (top + bot) / 2;
  nassertr(mid >= 0 && mid < _basis.get_num_segments(), -1);

  float from = _basis.get_from(mid);
  float to = _basis.get_to(mid);
  if (from > t) {
    // Too high, try lower.
    return r_find_segment(t, top, mid - 1);

  } else if (to <= t) {
    // Too low, try higher.
    return r_find_segment(t, mid + 1, bot);

  } else {
    // Here we are!
    return mid;
  }
}

