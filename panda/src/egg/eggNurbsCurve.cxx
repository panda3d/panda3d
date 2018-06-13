/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggNurbsCurve.cxx
 * @author drose
 * @date 2000-02-15
 */

#include "eggNurbsCurve.h"

#include "indent.h"

TypeHandle EggNurbsCurve::_type_handle;

/**
 * Makes a copy of this object.
 */
EggNurbsCurve *EggNurbsCurve::
make_copy() const {
  return new EggNurbsCurve(*this);
}

/**
 * Prepares a new curve definition with the indicated order and number of
 * knots.  This also implies a particular number of vertices as well (the
 * number of knots minus the order), but it is up to the user to add the
 * correct number of vertices to the curve by repeatedly calling push_back().
 */
void EggNurbsCurve::
setup(int order, int num_knots) {
  _order = order;
  _knots.clear();

  int i;
  _knots.reserve(num_knots);
  for (i = 0; i < num_knots; i++) {
    _knots.push_back((double)i);
  }
}

/**
 * Directly changes the number of knots.  This will either add zero-valued
 * knots onto the end, or truncate knot values from the end, depending on
 * whether the list is being increased or decreased.  If possible, it is
 * preferable to use the setup() method instead of directly setting the number
 * of knots, as this may result in an invalid curve.
 */
void EggNurbsCurve::
set_num_knots(int num) {
  if ((int)_knots.size() >= num) {
    // Truncate knot values at the end.
    _knots.erase(_knots.begin() + num, _knots.end());
  } else {
    // Append knot values to the end.
    _knots.reserve(num);
    for (int i = _knots.size(); i < num; i++) {
      _knots.push_back(0.0);
    }
  }
}

/**
 * Returns true if the NURBS parameters are all internally consistent (e.g.
 * it has the right number of vertices to match its number of knots and order
 * in each dimension), or false otherwise.
 */
bool EggNurbsCurve::
is_valid() const {
  if (_order < 1 || _order > 4) {
    // Invalid order.
    return false;
  }

  if (get_num_cvs() != (int)size()) {
    // Wrong number of CV's.
    return false;
  }

  // Do all the knot values monotonically increase?
  int i;
  for (i = 1; i < get_num_knots(); i++) {
    if (get_knot(i) < get_knot(i - 1)) {
      return false;
    }
  }

  // Everything's looking good!
  return true;
}

/**
 * Returns true if the curve appears to be closed.  Since the Egg syntax does
 * not provide a means for explicit indication of closure, this has to be
 * guessed at by examining the curve itself.
 */
bool EggNurbsCurve::
is_closed() const {
  // Technically, the curve is closed if the CV's at the end are repeated from
  // the beginning.  We'll do a cheesy test for expediency's sake: the curve
  // is closed if the first n knots are not repeated.  I think this will catch
  // all the normal curves we're likely to see.

  int i;
  for (i = 1; i < get_order(); i++) {
    if (get_knot(i) != get_knot(i-1)) {
      return true;
    }
  }
  return false;
}

/**
 * Writes the nurbsCurve to the indicated output stream in Egg format.
 */
void EggNurbsCurve::
write(std::ostream &out, int indent_level) const {
  write_header(out, indent_level, "<NurbsCurve>");

  if (get_curve_type() != CT_none) {
    indent(out, indent_level + 2)
      << "<Char*> type { " << get_curve_type() << " }\n";
  }
  if (get_subdiv() != 0) {
    indent(out, indent_level + 2)
      << "<Scalar> subdiv { " << get_subdiv() << " }\n";
  }
  indent(out, indent_level + 2)
    << "<Order> { " << get_order() << " }\n";
  indent(out, indent_level + 2)
    << "<Knots> {\n";
  write_long_list(out, indent_level+4, _knots.begin(), _knots.end(), "",
        "", 72);
  indent(out, indent_level + 2)
    << "}\n";

  write_body(out, indent_level+2);
  indent(out, indent_level) << "}\n";
}
