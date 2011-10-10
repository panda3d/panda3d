// Filename: nurbsCurveResult.cxx
// Created by:  drose (04Dec02)
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

#include "nurbsCurveResult.h"
#include "nurbsVertex.h"


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::Constructor
//       Access: Public
//  Description: The constructor automatically builds up the result as
//               the product of the indicated set of basis matrices
//               and the indicated table of control vertex positions.
////////////////////////////////////////////////////////////////////
NurbsCurveResult::
NurbsCurveResult(const NurbsBasisVector &basis,
                 const LVecBase4 vecs[], const NurbsVertex *verts,
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
    nassertv(vi >= 0 && vi + order - 1 < num_vertices);

    // Create a geometry matrix from our (up to) four involved vertices.
    LMatrix4 geom;
    int ci = 0;
    while (ci < order) {
      geom.set_row(ci, vecs[vi + ci]);
      ci++;
    }
    while (ci < 4) {
      geom.set_row(ci, LVecBase4::zero());
      ci++;
    }

    // And compose this geometry matrix with the basis matrix to
    // produce a new matrix, which will be used to evaluate the curve.
    LMatrix4 result;
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
eval_segment_point(int segment, PN_stdfloat t, LVecBase3 &point) const {
  PN_stdfloat t2 = t*t;
  LVecBase4 tvec(t*t2, t2, t, 1.0f);

  PN_stdfloat weight = tvec.dot(_composed[segment].get_col(3));

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
eval_segment_tangent(int segment, PN_stdfloat t, LVecBase3 &tangent) const {
  PN_stdfloat t2 = t*t;
  LVecBase4 tvec(3.0 * t2, 2.0 * t, 1.0f, 0.0f);

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
PN_stdfloat NurbsCurveResult::
eval_segment_extended_point(int segment, PN_stdfloat t, int d) const {
  nassertr(segment >= 0 && segment < _basis.get_num_segments(), 0.0f);

  PN_stdfloat t2 = t*t;
  LVecBase4 tvec(t*t2, t2, t, 1.0f);

  PN_stdfloat weight = tvec.dot(_composed[segment].get_col(3));

  // Calculate the composition of the basis matrix and the geometry
  // matrix on-the-fly.
  int order = _basis.get_order();
  int vi = _basis.get_vertex_index(segment);

  LVecBase4 geom;
  int ci = 0;
  while (ci < order) {
    geom[ci] = _verts[vi + ci].get_extended_vertex(d);
    ci++;
  }
  while (ci < 4) {
    geom[ci] = 0.0f;
    ci++;
  }

  const LMatrix4 &basis = _basis.get_basis(segment);

  // Compute matrix * column vector.
  LVecBase4 composed_geom(basis.get_row(0).dot(geom),
                           basis.get_row(1).dot(geom),
                           basis.get_row(2).dot(geom),
                           basis.get_row(3).dot(geom));
  return tvec.dot(composed_geom) / weight;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::eval_segment_extended_points
//       Access: Published
//  Description: Simultaneously performs eval_extended_point on a
//               contiguous sequence of dimensions.  The dimensions
//               evaluated are d through (d + num_values - 1); the
//               results are filled into the num_values elements in
//               the indicated result array.
////////////////////////////////////////////////////////////////////
void NurbsCurveResult::
eval_segment_extended_points(int segment, PN_stdfloat t, int d,
                             PN_stdfloat result[], int num_values) const {
  nassertv(segment >= 0 && segment < _basis.get_num_segments());

  PN_stdfloat t2 = t*t;
  LVecBase4 tvec(t*t2, t2, t, 1.0f);

  PN_stdfloat weight = tvec.dot(_composed[segment].get_col(3));

  // Calculate the composition of the basis matrix and the geometry
  // matrix on-the-fly.
  const LMatrix4 &basis = _basis.get_basis(segment);
  int order = _basis.get_order();
  int vi = _basis.get_vertex_index(segment);

  for (int n = 0; n < num_values; n++) {
    LVecBase4 geom;
    int ci = 0;
    while (ci < order) {
      geom[ci] = _verts[vi + ci].get_extended_vertex(d + n);
      ci++;
    }
    while (ci < 4) {
      geom[ci] = 0.0f;
      ci++;
    }

    // Compute matrix * column vector.
    LVecBase4 composed_geom(basis.get_row(0).dot(geom),
                             basis.get_row(1).dot(geom),
                             basis.get_row(2).dot(geom),
                             basis.get_row(3).dot(geom));
    result[n] = tvec.dot(composed_geom) / weight;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::adaptive_sample
//       Access: Published
//  Description: Determines the set of subdivisions necessary to
//               approximate the curve with a set of linear segments,
//               no point of which is farther than tolerance units
//               from the actual curve.
//
//               After this call, you may walk through the resulting
//               set of samples with get_num_samples(),
//               get_sample_t(), and get_sample_point().
////////////////////////////////////////////////////////////////////
void NurbsCurveResult::
adaptive_sample(PN_stdfloat tolerance) {
  PN_stdfloat tolerance_2 = tolerance * tolerance;
  _adaptive_result.clear();

  LPoint3 p0, p1;

  int num_segments = _basis.get_num_segments();
  for (int segment = 0; segment < num_segments; ++segment) {
    eval_segment_point(segment, 0.0f, p0);
    if (segment == 0 || !p0.almost_equal(p1)) {
      // We explicitly push the first point, and the boundary point
      // anytime the segment boundary is discontinuous.
      _adaptive_result.push_back(AdaptiveSample(_basis.get_from(segment), p0));
    }

    eval_segment_point(segment, 1.0f, p1);

    // Then we recusrively get the remaining points in the segment.
    r_adaptive_sample(segment, 0.0f, p0, 1.0f, p1, tolerance_2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::find_segment
//       Access: Private
//  Description: Returns the index of the segment that contains the
//               indicated value of t, or -1 if no segment contains
//               this value.
////////////////////////////////////////////////////////////////////
int NurbsCurveResult::
find_segment(PN_stdfloat t) {
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
r_find_segment(PN_stdfloat t, int top, int bot) const {
  if (bot < top) {
    // Not found.
    return -1;
  }
  int mid = (top + bot) / 2;
  nassertr(mid >= 0 && mid < _basis.get_num_segments(), -1);

  PN_stdfloat from = _basis.get_from(mid);
  PN_stdfloat to = _basis.get_to(mid);
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

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::r_adaptive_sample
//       Access: Private
//  Description: Recursively subdivides a potential evaluation of the
//               segment until it is found to be within tolerance.
//               This will add everything up to and including t1, but
//               excluding t0.
////////////////////////////////////////////////////////////////////
void NurbsCurveResult::
r_adaptive_sample(int segment, PN_stdfloat t0, const LPoint3 &p0, 
                  PN_stdfloat t1, const LPoint3 &p1, PN_stdfloat tolerance_2) {
  PN_stdfloat tmid = (t0 + t1) * 0.5f;
  LPoint3 pmid;
  eval_segment_point(segment, tmid, pmid);

  if (sqr_dist_to_line(pmid, p0, p1 - p0) > tolerance_2) {
    // The line is still too curved--subdivide.
    r_adaptive_sample(segment, t0, p0, tmid, pmid, tolerance_2);
    r_adaptive_sample(segment, tmid, pmid, t1, p1, tolerance_2);

  } else {
    // The line is sufficiently close.  Stop here.
    _adaptive_result.push_back(AdaptiveSample(_basis.scale_t(segment, t1), p1));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurveResult::sqr_dist_to_line
//       Access: Private, Static
//  Description: A support function for r_adaptive_sample(), this
//               computes the minimum distance from a point to a line,
//               and returns the distance squared.
////////////////////////////////////////////////////////////////////
PN_stdfloat NurbsCurveResult::
sqr_dist_to_line(const LPoint3 &point, const LPoint3 &origin, 
                 const LVector3 &vec) {
  LVector3 norm = vec;
  norm.normalize();
  LVector3 d = point - origin;
  PN_stdfloat hyp_2 = d.length_squared();
  PN_stdfloat leg = d.dot(norm);
  return hyp_2 - leg * leg;
}
