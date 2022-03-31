/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggNurbsSurface.cxx
 * @author drose
 * @date 2000-02-15
 */

#include "eggNurbsSurface.h"

#include "indent.h"

TypeHandle EggNurbsSurface::_type_handle;

/**
 * Makes a copy of this object.
 */
EggNurbsSurface *EggNurbsSurface::
make_copy() const {
  return new EggNurbsSurface(*this);
}

/**
 * Prepares a new surface definition with the indicated order and number of
 * knots in each dimension.  This also implies a particular number of vertices
 * in each dimension as well (the number of knots minus the order), but it is
 * up to the user to add the correct number of vertices to the surface by
 * repeatedly calling push_back().
 */
void EggNurbsSurface::
setup(int u_order, int v_order,
      int num_u_knots, int num_v_knots) {
  _u_order = u_order;
  _v_order = v_order;
  _u_knots.clear();
  _v_knots.clear();

  int i;
  _u_knots.reserve(num_u_knots);
  for (i = 0; i < num_u_knots; i++) {
    _u_knots.push_back((double)i);
  }
  _v_knots.reserve(num_v_knots);
  for (i = 0; i < num_v_knots; i++) {
    _v_knots.push_back((double)i);
  }
}

/**
 * Directly changes the number of knots in the U direction.  This will either
 * add zero-valued knots onto the end, or truncate knot values from the end,
 * depending on whether the list is being increased or decreased.  If
 * possible, it is preferable to use the setup() method instead of directly
 * setting the number of knots, as this may result in an invalid surface.
 */
void EggNurbsSurface::
set_num_u_knots(int num) {
  if ((int)_u_knots.size() >= num) {
    // Truncate knot values at the end.
    _u_knots.erase(_u_knots.begin() + num, _u_knots.end());
  } else {
    // Append knot values to the end.
    _u_knots.reserve(num);
    for (int i = _u_knots.size(); i < num; i++) {
      _u_knots.push_back(0.0);
    }
  }
}

/**
 * Directly changes the number of knots in the V direction.  This will either
 * add zero-valued knots onto the end, or truncate knot values from the end,
 * depending on whether the list is being increased or decreased.  If
 * possible, it is preferable to use the setup() method instead of directly
 * setting the number of knots, as this may result in an invalid surface.
 */
void EggNurbsSurface::
set_num_v_knots(int num) {
  if ((int)_v_knots.size() >= num) {
    // Truncate knot values at the end.
    _v_knots.erase(_v_knots.begin() + num, _v_knots.end());
  } else {
    // Append knot values to the end.
    _v_knots.reserve(num);
    for (int i = _v_knots.size(); i < num; i++) {
      _v_knots.push_back(0.0);
    }
  }
}

/**
 * Returns true if the NURBS parameters are all internally consistent (e.g.
 * it has the right number of vertices to match its number of knots and order
 * in each dimension), or false otherwise.
 */
bool EggNurbsSurface::
is_valid() const {
  if (_u_order < 1 || _u_order > 4 || _v_order < 1 || _v_order > 4) {
    // Invalid order.
    return false;
  }

  if (get_num_cvs() != (int)size()) {
    // Wrong number of CV's.
    return false;
  }

  // Do all the knot values monotonically increase?
  int i;
  for (i = 1; i < get_num_u_knots(); i++) {
    if (get_u_knot(i) < get_u_knot(i - 1)) {
      return false;
    }
  }
  for (i = 1; i < get_num_v_knots(); i++) {
    if (get_v_knot(i) < get_v_knot(i - 1)) {
      return false;
    }
  }

  // Everything's looking good!
  return true;
}

/**
 * Returns true if the surface appears to be closed in the U direction.  Since
 * the Egg syntax does not provide a means for explicit indication of closure,
 * this has to be guessed at by examining the surface itself.
 */
bool EggNurbsSurface::
is_closed_u() const {
  // Technically, the surface is closed if the CV's at the end are repeated
  // from the beginning.  We'll do a cheesy test for expediency's sake: the
  // surface is closed if the first n knots are not repeated.  I think this
  // will catch all the normal surfaces we're likely to see.

  int i;
  for (i = 1; i < get_u_order(); i++) {
    if (get_u_knot(i) != get_u_knot(i-1)) {
      return true;
    }
  }
  return false;
}

/**
 * Returns true if the surface appears to be closed in the V direction.  Since
 * the Egg syntax does not provide a means for explicit indication of closure,
 * this has to be guessed at by examining the surface itself.
 */
bool EggNurbsSurface::
is_closed_v() const {
  int i;
  for (i = 1; i < get_v_order(); i++) {
    if (get_v_knot(i) != get_v_knot(i-1)) {
      return true;
    }
  }
  return false;
}

/**
 * Writes the nurbsSurface to the indicated output stream in Egg format.
 */
void EggNurbsSurface::
write(std::ostream &out, int indent_level) const {
  write_header(out, indent_level, "<NurbsSurface>");

  Trims::const_iterator ti;
  for (ti = _trims.begin(); ti != _trims.end(); ++ti) {
    indent(out, indent_level + 2) << "<Trim> {\n";
    Loops::const_iterator li;
    for (li = (*ti).begin(); li != (*ti).end(); ++li) {
      indent(out, indent_level + 4) << "<Loop> {\n";
      Curves::const_iterator ci;
      for (ci = (*li).begin(); ci != (*li).end(); ++ci) {
        (*ci)->write(out, indent_level + 6);
      }
      indent(out, indent_level + 4) << "}\n";
    }
    indent(out, indent_level + 2) << "}\n";
  }

  if (get_u_subdiv() != 0) {
    indent(out, indent_level + 2)
      << "<Scalar> U-subdiv { " << get_u_subdiv() << " }\n";
  }
  if (get_v_subdiv() != 0) {
    indent(out, indent_level + 2)
      << "<Scalar> V-subdiv { " << get_v_subdiv() << " }\n";
  }
  indent(out, indent_level + 2)
    << "<Order> { " << get_u_order() << " " << get_v_order() << " }\n";
  indent(out, indent_level + 2)
    << "<U-Knots> {\n";
  write_long_list(out, indent_level+4, _u_knots.begin(), _u_knots.end(),
        "", "", 72);
  indent(out, indent_level + 2)
    << "}\n";
  indent(out, indent_level + 2)
    << "<V-Knots> {\n";
  write_long_list(out, indent_level+4, _v_knots.begin(), _v_knots.end(),
        "", "", 72);
  indent(out, indent_level + 2)
    << "}\n";

  write_body(out, indent_level+2);

  Curves::const_iterator ci;
  for (ci = _curves_on_surface.begin(); ci != _curves_on_surface.end(); ++ci) {
    (*ci)->write(out, indent_level + 2);
  }

  indent(out, indent_level) << "}\n";
}

/**
 * The recursive implementation of apply_texmats().
 */
void EggNurbsSurface::
r_apply_texmats(EggTextureCollection &textures) {
  // A NURBS cannot safely apply texture matrices, so we leave it alone.
}
