/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggNurbsSurface.I
 * @author drose
 * @date 2000-02-15
 */

/**
 *
 */
INLINE EggNurbsSurface::
EggNurbsSurface(const std::string &name) : EggSurface(name) {
  _u_order = 0;
  _v_order = 0;
}

/**
 *
 */
INLINE EggNurbsSurface::
EggNurbsSurface(const EggNurbsSurface &copy) :
  EggSurface(copy),
  _u_knots(copy._u_knots),
  _v_knots(copy._v_knots),
  _u_order(copy._u_order),
  _v_order(copy._v_order)
{
}

/**
 *
 */
INLINE EggNurbsSurface &EggNurbsSurface::
operator = (const EggNurbsSurface &copy) {
  EggSurface::operator = (copy);
  _u_knots = copy._u_knots;
  _v_knots = copy._v_knots;
  _u_order = copy._u_order;
  _v_order = copy._v_order;
  return *this;
}

/**
 * Directly changes the order in the U direction to the indicated value (which
 * must be an integer in the range 1 <= u_order <= 4).  If possible, it is
 * preferable to use the setup() method instead of this method, since changing
 * the order directly may result in an invalid surface.
 */
INLINE void EggNurbsSurface::
set_u_order(int u_order) {
  nassertv(u_order >= 1 && u_order <= 4);
  _u_order = u_order;
}

/**
 * Directly changes the order in the V direction to the indicated value (which
 * must be an integer in the range 1 <= v_order <= 4).  If possible, it is
 * preferable to use the setup() method instead of this method, since changing
 * the order directly may result in an invalid surface.
 */
INLINE void EggNurbsSurface::
set_v_order(int v_order) {
  nassertv(v_order >= 1 && v_order <= 4);
  _v_order = v_order;
}

/**
 * Resets the value of the indicated knot as indicated.  k must be in the
 * range 0 <= k < get_num_u_knots(), and the value must be in the range
 * get_u_knot(k - 1) <= value <= get_u_knot(k + 1).
 */
INLINE void EggNurbsSurface::
set_u_knot(int k, double value) {
  nassertv(k >= 0 && k < (int)_u_knots.size());
  _u_knots[k] = value;
}

/**
 * Resets the value of the indicated knot as indicated.  k must be in the
 * range 0 <= k < get_num_v_knots(), and the value must be in the range
 * get_v_knot(k - 1) <= value <= get_v_knot(k + 1).
 */
INLINE void EggNurbsSurface::
set_v_knot(int k, double value) {
  nassertv(k >= 0 && k < (int)_v_knots.size());
  _v_knots[k] = value;
}

/**
 * Redefines the control vertex associated with a particular u, v coordinate
 * pair.  This is just a shorthand to access the EggPrimitive's normal vertex
 * assignment for a 2-d control vertex.
 */
INLINE void EggNurbsSurface::
set_cv(int ui, int vi, EggVertex *vertex) {
  int vertex_index = get_vertex_index(ui, vi);
  set_vertex(vertex_index, vertex);
}

/**
 * Returns the order of the surface in the U direction.  The order is the
 * degree of the NURBS equation plus 1; for a typical NURBS, the order is 4.
 * With this implementation of NURBS, the order must be in the range [1, 4].
 */
INLINE int EggNurbsSurface::
get_u_order() const {
  return _u_order;
}

/**
 * Returns the order of the surface in the V direction.  The order is the
 * degree of the NURBS equation plus 1; for a typical NURBS, the order is 4.
 * With this implementation of NURBS, the order must be in the range [1, 4].
 */
INLINE int EggNurbsSurface::
get_v_order() const {
  return _v_order;
}

/**
 * Returns the degree of the surface in the U direction.  For a typical NURBS,
 * the degree is 3.
 */
INLINE int EggNurbsSurface::
get_u_degree() const {
  return _u_order - 1;
}

/**
 * Returns the degree of the surface in the V direction.  for a typical NURBS,
 * the degree is 3.
 */
INLINE int EggNurbsSurface::
get_v_degree() const {
  return _v_order - 1;
}

/**
 * Returns the number of knots in the U direction.
 */
INLINE int EggNurbsSurface::
get_num_u_knots() const {
  return _u_knots.size();
}

/**
 * Returns the number of knots in the V direction.
 */
INLINE int EggNurbsSurface::
get_num_v_knots() const {
  return _v_knots.size();
}

/**
 * Returns the number of control vertices that should be present in the U
 * direction.  This is determined by the number of knots and the order; it
 * does not necessarily reflect the number of vertices that have actually been
 * added to the surface.  (However, if the number of vertices in the surface
 * are wrong, the surface is invalid.)
 */
INLINE int EggNurbsSurface::
get_num_u_cvs() const {
  return get_num_u_knots() - get_u_order();
}

/**
 * Returns the number of control vertices that should be present in the V
 * direction.  This is determined by the number of knots and the order; it
 * does not necessarily reflect the number of vertices that have actually been
 * added to the surface.  (However, if the number of vertices in the surface
 * are wrong, the surface is invalid.)
 */
INLINE int EggNurbsSurface::
get_num_v_cvs() const {
  return get_num_v_knots() - get_v_order();
}

/**
 * Returns the total number of control vertices that *should* be defined for
 * the surface.  This is determined by the number of knots and the order, in
 * each direction; it does not necessarily reflect the number of vertices that
 * have actually been added to the surface.  (However, if the number of
 * vertices in the surface are wrong, the surface is invalid.)
 */
INLINE int EggNurbsSurface::
get_num_cvs() const {
  return get_num_u_cvs() * get_num_v_cvs();
}

/**
 * Returns the U index number of the given vertex within the EggPrimitive's
 * linear list of vertices.  An EggNurbsSurface maps a linear list of vertices
 * to its 2-d mesh; this returns the U index number that corresponds to the
 * nth vertex in the list.
 */
INLINE int EggNurbsSurface::
get_u_index(int vertex_index) const {
  nassertr(vertex_index >= 0 && vertex_index < get_num_cvs(), 0);
  return vertex_index % get_num_u_cvs();
}

/**
 * Returns the V index number of the given vertex within the EggPrimitive's
 * linear list of vertices.  An EggNurbsSurface maps a linear list of vertices
 * to its 2-d mesh; this returns the V index number that corresponds to the
 * nth vertex in the list.
 */
INLINE int EggNurbsSurface::
get_v_index(int vertex_index) const {
  nassertr(vertex_index >= 0 && vertex_index < get_num_cvs(), 0);
  return vertex_index / get_num_u_cvs();
}

/**
 * Returns the index number within the EggPrimitive's list of the control
 * vertex at position ui, vi.
 */
INLINE int EggNurbsSurface::
get_vertex_index(int ui, int vi) const {
  nassertr(ui >= 0 && ui < get_num_u_cvs(), 0);
  nassertr(vi >= 0 && vi < get_num_v_cvs(), 0);
  return vi * get_num_u_cvs() + ui;
}

/**
 * Returns the nth knot value defined in the U direction.
 */
INLINE double EggNurbsSurface::
get_u_knot(int k) const {
  nassertr(k >= 0 && k < (int)_u_knots.size(), 0.0);
  return _u_knots[k];
}

/**
 * Returns the nth knot value defined in the V direction.
 */
INLINE double EggNurbsSurface::
get_v_knot(int k) const {
  nassertr(k >= 0 && k < (int)_v_knots.size(), 0.0);
  return _v_knots[k];
}

/**
 * Returns the control vertex at the indicate U, V position.
 */
INLINE EggVertex *EggNurbsSurface::
get_cv(int ui, int vi) const {
  int vertex_index = get_vertex_index(ui, vi);
  return get_vertex(vertex_index);
}
