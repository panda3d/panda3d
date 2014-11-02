// Filename: boundingLine.cxx
// Created by:  drose (04Jul00)
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

#include "boundingLine.h"
#include "boundingSphere.h"
#include "boundingBox.h"
#include "config_mathutil.h"

#include <math.h>

TypeHandle BoundingLine::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
BoundingVolume *BoundingLine::
make_copy() const {
  return new BoundingLine(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::get_approx_center
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3 BoundingLine::
get_approx_center() const {
  nassertr(!is_empty(), LPoint3(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3(0.0f, 0.0f, 0.0f));
  return (get_point_a() + get_point_b()) / 2.0;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::xform
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void BoundingLine::
xform(const LMatrix4 &mat) {
  nassertv(!mat.is_nan());

  if (!is_empty() && !is_infinite()) {
    _origin = _origin * mat;
    _vector = _vector * mat;
    if (!_vector.normalize()) {
      // If we just scaled the line down to nothing, it becomes an
      // empty volume.
      _flags |= F_empty;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void BoundingLine::
output(ostream &out) const {
  if (is_empty()) {
    out << "bline, empty";
  } else if (is_infinite()) {
    out << "bline, infinite";
  } else {
    out << "bline, (" << _origin << ") - (" << _origin + _vector << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::as_bounding_line
//       Access: Public, Virtual
//  Description: Virtual downcast method.  Returns this object as a
//               pointer of the indicated type, if it is in fact that
//               type.  Returns NULL if it is not that type.
////////////////////////////////////////////////////////////////////
const BoundingLine *BoundingLine::
as_bounding_line() const {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::extend_other
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool BoundingLine::
extend_other(BoundingVolume *other) const {
  return other->extend_by_line(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::around_other
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool BoundingLine::
around_other(BoundingVolume *other,
             const BoundingVolume **first,
             const BoundingVolume **last) const {
  return other->around_lines(first, last);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::contains_other
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int BoundingLine::
contains_other(const BoundingVolume *other) const {
  return other->contains_line(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::extend_by_line
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool BoundingLine::
extend_by_line(const BoundingLine *line) {
  nassertr(!line->is_empty() && !line->is_infinite(), false);
  nassertr(!is_infinite(), false);

  if (is_empty()) {
    _origin = line->_origin;
    _vector = line->_vector;
    _flags = 0;
  } else {
    _flags = F_infinite;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::contains_sphere
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int BoundingLine::
contains_sphere(const BoundingSphere *sphere) const {
  nassertr(!is_empty() && !is_infinite(), 0);
  nassertr(!sphere->is_empty() && !sphere->is_infinite(), 0);

  PN_stdfloat r = sphere->get_radius();

  if (r * r >= sqr_dist_to_line(sphere->get_center())) {
    return IF_possible | IF_some;
  } else {
    return IF_no_intersection;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::contains_box
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int BoundingLine::
contains_box(const BoundingBox *box) const {
  nassertr(!is_empty() && !is_infinite(), 0);
  nassertr(!box->is_empty() && !box->is_infinite(), 0);

  LPoint3 center = (box->get_minq() + box->get_maxq()) * 0.5f;
  PN_stdfloat r2 = (box->get_maxq() - box->get_minq()).length_squared() * 0.25f;

  if (r2 >= sqr_dist_to_line(center)) {
    return IF_possible;
  } else {
    return IF_no_intersection;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingLine::sqr_dist_to_line
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat BoundingLine::
sqr_dist_to_line(const LPoint3 &point) const {
  nassertr(!point.is_nan(), 0.0f);
  nassertr(!is_empty() && !is_infinite(), 0.0f);
  nassertr(!_vector.almost_equal(LVector3(0.0f, 0.0f, 0.0f)), 0.0f);

  // The formula for the distance from a point to the line based on
  // the quadratic equation.

  PN_stdfloat A = dot(_vector, _vector);
  nassertr(A != 0.0f, 0.0f);
  LVector3 fc = _origin - point;
  PN_stdfloat B = 2.0 * dot(_vector, fc);
  PN_stdfloat fc_d2 = dot(fc, fc);

  PN_stdfloat r2 = fc_d2 - B*B / 4.0*A;

  nassertr(!cnan(r2), 0.0f);
  return r2;
}
