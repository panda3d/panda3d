/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsSurfaceResult.cxx
 * @author drose
 * @date 2003-10-10
 */

#include "nurbsSurfaceResult.h"
#include "nurbsVertex.h"


/**
 * The constructor automatically builds up the result as the product of the
 * indicated set of basis matrices and the indicated table of control vertex
 * positions.
 */
NurbsSurfaceResult::
NurbsSurfaceResult(const NurbsBasisVector &u_basis,
                   const NurbsBasisVector &v_basis,
                   const LVecBase4 vecs[], const NurbsVertex *verts,
                   int num_u_vertices, int num_v_vertices) :
  _u_basis(u_basis),
  _v_basis(v_basis),
  _verts(verts),
  _num_u_vertices(num_u_vertices),
  _num_v_vertices(num_v_vertices)
{
  // The V basis matrices will always be transposed.
  _v_basis.transpose();

  _last_u_segment = -1;
  _last_v_segment = -1;
  int u_order = _u_basis.get_order();
  int v_order = _v_basis.get_order();
  int num_u_segments = _u_basis.get_num_segments();
  int num_v_segments = _v_basis.get_num_segments();
  int num_segments = num_u_segments * num_v_segments;

  _composed.reserve(num_segments);
  for (int i = 0; i < num_segments; i++) {
    _composed.push_back(ComposedMats());
  }

  for (int vi = 0; vi < num_v_segments; vi++) {
    const LMatrix4 &v_basis_transpose = _v_basis.get_basis(vi);

    int vn = _v_basis.get_vertex_index(vi);
    nassertv(vn >= 0 && vn + v_order - 1 < _num_v_vertices);

    for (int ui = 0; ui < num_u_segments; ui++) {
      const LMatrix4 &u_basis_mat = _u_basis.get_basis(ui);

      int un = _u_basis.get_vertex_index(ui);
      nassertv(un >= 0 && un + u_order - 1 < _num_u_vertices);

      // Create four geometry matrices from our (up to) sixteen involved
      // vertices.
      LMatrix4 geom_x, geom_y, geom_z, geom_w;
      geom_x.fill(0);
      geom_y.fill(0);
      geom_z.fill(0);
      geom_w.fill(0);

      for (int uni = 0; uni < 4; uni++) {
        for (int vni = 0; vni < 4; vni++) {
          if (uni < u_order && vni < v_order) {
            const LVecBase4 &vec = vecs[verti(un + uni, vn + vni)];
            geom_x(uni, vni) = vec[0];
            geom_y(uni, vni) = vec[1];
            geom_z(uni, vni) = vec[2];
            geom_w(uni, vni) = vec[3];
          }
        }
      }

      // And compose these geometry matrices with the basis matrices to
      // produce a new set of matrices, which will be used to evaluate the
      // surface.
      int i = segi(ui, vi);
      nassertv(i >= 0 && i < (int)_composed.size());
      ComposedMats &result = _composed[i];
      result._x = u_basis_mat * geom_x * v_basis_transpose;
      result._y = u_basis_mat * geom_y * v_basis_transpose;
      result._z = u_basis_mat * geom_z * v_basis_transpose;
      result._w = u_basis_mat * geom_w * v_basis_transpose;
    }
  }
}

/**
 * Evaluates the point on the surface corresponding to the indicated value in
 * parametric time within the indicated surface segment.  u and v should be in
 * the range [0, 1].
 *
 * The surface is internally represented as a number of connected (or possibly
 * unconnected) piecewise continuous segments.  The exact number of segments
 * for a particular surface depends on the knot vector, and is returned by
 * get_num_segments().  Normally, eval_point() is used to evaluate a point
 * along the continuous surface, but when you care more about local
 * continuity, you can use eval_segment_point() to evaluate the points along
 * each segment.
 */
void NurbsSurfaceResult::
eval_segment_point(int ui, int vi, PN_stdfloat u, PN_stdfloat v, LVecBase3 &point) const {
  int i = segi(ui, vi);
  nassertv(i >= 0 && i < (int)_composed.size());

  PN_stdfloat u2 = u*u;
  LVecBase4 uvec(u*u2, u2, u, 1.0f);
  PN_stdfloat v2 = v*v;
  LVecBase4 vvec(v*v2, v2, v, 1.0f);

  PN_stdfloat weight = vvec.dot(uvec * _composed[i]._w);

  point.set(vvec.dot(uvec * _composed[i]._x) / weight,
            vvec.dot(uvec * _composed[i]._y) / weight,
            vvec.dot(uvec * _composed[i]._z) / weight);
}

/**
 * As eval_segment_point, but computes the normal to the surface at the
 * indicated point.  The normal vector will not necessarily be normalized, and
 * could be zero.
 */
void NurbsSurfaceResult::
eval_segment_normal(int ui, int vi, PN_stdfloat u, PN_stdfloat v, LVecBase3 &normal) const {
  int i = segi(ui, vi);
  nassertv(i >= 0 && i < (int)_composed.size());

  PN_stdfloat u2 = u*u;
  LVecBase4 uvec(u*u2, u2, u, 1.0f);
  LVecBase4 duvec(3.0f * u2, 2.0f * u, 1.0f, 0.0f);
  PN_stdfloat v2 = v*v;
  LVecBase4 vvec(v*v2, v2, v, 1.0f);
  LVecBase4 dvvec(3.0f * v2, 2.0f * v, 1.0f, 0.0f);

  LVector3 utan(vvec.dot(duvec * _composed[i]._x),
                 vvec.dot(duvec * _composed[i]._y),
                 vvec.dot(duvec * _composed[i]._z));

  LVector3 vtan(dvvec.dot(uvec * _composed[i]._x),
                 dvvec.dot(uvec * _composed[i]._y),
                 dvvec.dot(uvec * _composed[i]._z));

  normal = utan.cross(vtan);
}

/**
 * Evaluates the surface in n-dimensional space according to the extended
 * vertices associated with the surface in the indicated dimension.
 */
PN_stdfloat NurbsSurfaceResult::
eval_segment_extended_point(int ui, int vi, PN_stdfloat u, PN_stdfloat v, int d) const {
  int i = segi(ui, vi);
  nassertr(i >= 0 && i < (int)_composed.size(), 0.0f);

  PN_stdfloat u2 = u*u;
  LVecBase4 uvec(u*u2, u2, u, 1.0f);
  PN_stdfloat v2 = v*v;
  LVecBase4 vvec(v*v2, v2, v, 1.0f);

  PN_stdfloat weight = vvec.dot(uvec * _composed[i]._w);

  // Calculate the composition of the basis matrices and the geometry matrix
  // on-the-fly.
  const LMatrix4 &v_basis_transpose = _v_basis.get_basis(vi);
  const LMatrix4 &u_basis_mat = _u_basis.get_basis(ui);
  int u_order = _u_basis.get_order();
  int v_order = _v_basis.get_order();

  int un = _u_basis.get_vertex_index(ui);
  int vn = _v_basis.get_vertex_index(vi);

  LMatrix4 geom;
  geom.fill(0);

  for (int uni = 0; uni < 4; uni++) {
    for (int vni = 0; vni < 4; vni++) {
      if (uni < u_order && vni < v_order) {
        geom(uni, vni) = _verts[verti(un + uni, vn + vni)].get_extended_vertex(d);
      }
    }
  }

  LMatrix4 composed = u_basis_mat * geom * v_basis_transpose;
  return vvec.dot(uvec * composed) / weight;
}

/**
 * Simultaneously performs eval_extended_point on a contiguous sequence of
 * dimensions.  The dimensions evaluated are d through (d + num_values - 1);
 * the results are filled into the num_values elements in the indicated result
 * array.
 */
void NurbsSurfaceResult::
eval_segment_extended_points(int ui, int vi, PN_stdfloat u, PN_stdfloat v, int d,
                             PN_stdfloat result[], int num_values) const {
  int i = segi(ui, vi);
  nassertv(i >= 0 && i < (int)_composed.size());

  PN_stdfloat u2 = u*u;
  LVecBase4 uvec(u*u2, u2, u, 1.0f);
  PN_stdfloat v2 = v*v;
  LVecBase4 vvec(v*v2, v2, v, 1.0f);

  PN_stdfloat weight = vvec.dot(uvec * _composed[i]._w);

  // Calculate the composition of the basis matrices and the geometry matrix
  // on-the-fly.
  const LMatrix4 &v_basis_transpose = _v_basis.get_basis(vi);
  const LMatrix4 &u_basis_mat = _u_basis.get_basis(ui);
  int u_order = _u_basis.get_order();
  int v_order = _v_basis.get_order();

  int un = _u_basis.get_vertex_index(ui);
  int vn = _v_basis.get_vertex_index(vi);

  for (int n = 0; n < num_values; n++) {
    LMatrix4 geom;
    geom.fill(0);

    for (int uni = 0; uni < 4; uni++) {
      for (int vni = 0; vni < 4; vni++) {
        if (uni < u_order && vni < v_order) {
          geom(uni, vni) =
            _verts[verti(un + uni, vn + vni)].get_extended_vertex(d + n);
        }
      }
    }

    LMatrix4 composed = u_basis_mat * geom * v_basis_transpose;
    result[n] = vvec.dot(uvec * composed) / weight;
  }
}

/**
 * Returns the index of the segment that contains the indicated value of t, or
 * -1 if no segment contains this value.
 */
int NurbsSurfaceResult::
find_u_segment(PN_stdfloat u) {
  // Trivially check the endpoints of the surface.
  if (u >= get_end_u()) {
    return _u_basis.get_num_segments() - 1;
  } else if (u <= get_start_u()) {
    return 0;
  }

  // Check the last segment we searched for.  Often, two consecutive requests
  // are for the same segment.
  if (_last_u_segment != -1 && (u >= _last_u_from && u < _last_u_to)) {
    return _last_u_segment;
  }

  // Look for the segment the hard way.
  int segment = r_find_u_segment(u, 0, _u_basis.get_num_segments() - 1);
  if (segment != -1) {
    _last_u_segment = segment;
    _last_u_from = _u_basis.get_from(segment);
    _last_u_to = _u_basis.get_to(segment);
  }
  return segment;
}

/**
 * Recursively searches for the segment that contains the indicated value of t
 * by performing a binary search.  This assumes the segments are stored in
 * increasing order of t, and they don't overlap.
 */
int NurbsSurfaceResult::
r_find_u_segment(PN_stdfloat u, int top, int bot) const {
  if (bot < top) {
    // Not found.
    return -1;
  }
  int mid = (top + bot) / 2;
  nassertr(mid >= 0 && mid < _u_basis.get_num_segments(), -1);

  PN_stdfloat from = _u_basis.get_from(mid);
  PN_stdfloat to = _u_basis.get_to(mid);
  if (from > u) {
    // Too high, try lower.
    return r_find_u_segment(u, top, mid - 1);

  } else if (to <= u) {
    // Too low, try higher.
    return r_find_u_segment(u, mid + 1, bot);

  } else {
    // Here we are!
    return mid;
  }
}


/**
 * Returns the index of the segment that contains the indicated value of t, or
 * -1 if no segment contains this value.
 */
int NurbsSurfaceResult::
find_v_segment(PN_stdfloat v) {
  // Trivially check the endpoints of the surface.
  if (v >= get_end_v()) {
    return _v_basis.get_num_segments() - 1;
  } else if (v <= get_start_v()) {
    return 0;
  }

  // Check the last segment we searched for.  Often, two consecutive requests
  // are for the same segment.
  if (_last_v_segment != -1 && (v >= _last_v_from && v < _last_v_to)) {
    return _last_v_segment;
  }

  // Look for the segment the hard way.
  int segment = r_find_v_segment(v, 0, _v_basis.get_num_segments() - 1);
  if (segment != -1) {
    _last_v_segment = segment;
    _last_v_from = _v_basis.get_from(segment);
    _last_v_to = _v_basis.get_to(segment);
  }
  return segment;
}

/**
 * Recursively searches for the segment that contains the indicated value of t
 * by performing a binary search.  This assumes the segments are stored in
 * increasing order of t, and they don't overlap.
 */
int NurbsSurfaceResult::
r_find_v_segment(PN_stdfloat v, int top, int bot) const {
  if (bot < top) {
    // Not found.
    return -1;
  }
  int mid = (top + bot) / 2;
  nassertr(mid >= 0 && mid < _v_basis.get_num_segments(), -1);

  PN_stdfloat from = _v_basis.get_from(mid);
  PN_stdfloat to = _v_basis.get_to(mid);
  if (from > v) {
    // Too high, try lower.
    return r_find_v_segment(v, top, mid - 1);

  } else if (to <= v) {
    // Too low, try higher.
    return r_find_v_segment(v, mid + 1, bot);

  } else {
    // Here we are!
    return mid;
  }
}
