// Filename: nurbsSurfaceResult.I
// Created by:  drose (10Oct03)
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


////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NurbsSurfaceResult::
~NurbsSurfaceResult() {
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::get_start_u
//       Access: Public
//  Description: Returns the first legal value of u on the surface.
//               Usually this is 0.0.
////////////////////////////////////////////////////////////////////
INLINE float NurbsSurfaceResult::
get_start_u() const {
  return _u_basis.get_start_t();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::get_end_u
//       Access: Public
//  Description: Returns the last legal value of u on the surface.
////////////////////////////////////////////////////////////////////
INLINE float NurbsSurfaceResult::
get_end_u() const {
  return _u_basis.get_end_t();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::get_start_v
//       Access: Public
//  Description: Returns the first legal value of v on the surface.
//               Usually this is 0.0.
////////////////////////////////////////////////////////////////////
INLINE float NurbsSurfaceResult::
get_start_v() const {
  return _v_basis.get_start_t();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::get_end_v
//       Access: Public
//  Description: Returns the last legal value of v on the surface.
////////////////////////////////////////////////////////////////////
INLINE float NurbsSurfaceResult::
get_end_v() const {
  return _v_basis.get_end_t();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::eval_point
//       Access: Published
//  Description: Computes the point on the surface corresponding to the
//               indicated value in parametric time.  Returns true if
//               the t value is value, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NurbsSurfaceResult::
eval_point(float u, float v, LVecBase3f &point) {
  int ui = find_u_segment(u);
  int vi = find_v_segment(v);
  if (ui == -1 || vi == -1) {
    return false;
  }

  eval_segment_point(ui, vi, _u_basis.scale_t(ui, u), _v_basis.scale_t(vi, v),
                     point);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::eval_normal
//       Access: Published
//  Description: Computes the normal to the surface at the indicated
//               point in parametric time.  This normal vector will
//               not necessarily be normalized, and could be zero.
//               See also eval_point().
////////////////////////////////////////////////////////////////////
INLINE bool NurbsSurfaceResult::
eval_normal(float u, float v, LVecBase3f &normal) {
  int ui = find_u_segment(u);
  int vi = find_v_segment(v);
  if (ui == -1 || vi == -1) {
    return false;
  }

  eval_segment_normal(ui, vi, _u_basis.scale_t(ui, u), _v_basis.scale_t(vi, v),
                      normal);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::eval_extended_point
//       Access: Published
//  Description: Evaluates the surface in n-dimensional space according
//               to the extended vertices associated with the surface in
//               the indicated dimension.
////////////////////////////////////////////////////////////////////
INLINE float NurbsSurfaceResult::
eval_extended_point(float u, float v, int d) {
  int ui = find_u_segment(u);
  int vi = find_v_segment(v);
  if (ui == -1 || vi == -1) {
    return 0.0f;
  }

  return eval_segment_extended_point(ui, vi, _u_basis.scale_t(ui, u), 
                                     _v_basis.scale_t(vi, v), d);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::get_num_u_segments
//       Access: Public
//  Description: Returns the number of piecewise continuous segments
//               within the surface in the U direction.  This number
//               is usually not important unless you plan to call
//               eval_segment_point().
////////////////////////////////////////////////////////////////////
INLINE int NurbsSurfaceResult::
get_num_u_segments() const {
  return _u_basis.get_num_segments();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::get_num_v_segments
//       Access: Public
//  Description: Returns the number of piecewise continuous segments
//               within the surface in the V direction.  This number
//               is usually not important unless you plan to call
//               eval_segment_point().
////////////////////////////////////////////////////////////////////
INLINE int NurbsSurfaceResult::
get_num_v_segments() const {
  return _v_basis.get_num_segments();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::get_segment_u
//       Access: Public
//  Description: Accepts a u value in the range [0, 1], and assumed to
//               be relative to the indicated segment (as in
//               eval_segment_point()), and returns the corresponding
//               u value in the entire surface (as in eval_point()).
////////////////////////////////////////////////////////////////////
INLINE float NurbsSurfaceResult::
get_segment_u(int ui, float u) const {
  return u * (_u_basis.get_to(ui) - _u_basis.get_from(ui)) + _u_basis.get_from(ui);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::get_segment_v
//       Access: Public
//  Description: Accepts a v value in the range [0, 1], and assumed to
//               be relative to the indicated segment (as in
//               eval_segment_point()), and returns the corresponding
//               v value in the entire surface (as in eval_point()).
////////////////////////////////////////////////////////////////////
INLINE float NurbsSurfaceResult::
get_segment_v(int vi, float v) const {
  return v * (_v_basis.get_to(vi) - _v_basis.get_from(vi)) + _v_basis.get_from(vi);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::verti
//       Access: Private
//  Description: An internal function to dereference a 2-d vertex
//               coordinate pair into a linear list of vertices.  This
//               returns the linear index corresponding to the 2-d
//               pair.
////////////////////////////////////////////////////////////////////
INLINE int NurbsSurfaceResult::
verti(int ui, int vi) const {
  return ui * _num_v_vertices + vi;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsSurfaceResult::segi
//       Access: Private
//  Description: An internal function to dereference a 2-d segment
//               coordinate pair into a linear list of segments.  This
//               returns the linear index corresponding to the 2-d
//               pair.
////////////////////////////////////////////////////////////////////
INLINE int NurbsSurfaceResult::
segi(int ui, int vi) const {
  return ui * _v_basis.get_num_segments() + vi;
}
