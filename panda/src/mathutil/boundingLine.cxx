// Filename: boundingLine.cxx
// Created by:  drose (04Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "boundingLine.h"
#include "boundingSphere.h"
#include "config_mathutil.h"

#include <math.h>

TypeHandle BoundingLine::_type_handle;

BoundingVolume *BoundingLine::
make_copy() const {
  return new BoundingLine(*this);
}

LPoint3f BoundingLine::
get_approx_center() const {
  nassertr(!is_empty(), LPoint3f(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3f(0.0f, 0.0f, 0.0f));
  return (get_point_a() + get_point_b()) / 2.0;
}

void BoundingLine::
xform(const LMatrix4f &mat) {
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

bool BoundingLine::
extend_other(BoundingVolume *other) const {
  return other->extend_by_line(this);
}

bool BoundingLine::
around_other(BoundingVolume *other,
             const BoundingVolume **first,
             const BoundingVolume **last) const {
  return other->around_lines(first, last);
}

int BoundingLine::
contains_other(const BoundingVolume *other) const {
  return other->contains_line(this);
}


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

int BoundingLine::
contains_sphere(const BoundingSphere *sphere) const {
  nassertr(!is_empty() && !is_infinite(), 0);
  nassertr(!sphere->is_empty() && !sphere->is_infinite(), 0);

  float r = sphere->get_radius();

  if (r * r >= sqr_dist_to_line(sphere->get_center())) {
    return IF_possible | IF_some;
  } else {
    return IF_no_intersection;
  }
}

float BoundingLine::
sqr_dist_to_line(const LPoint3f &point) const {
  nassertr(!point.is_nan(), 0.0f);
  nassertr(!is_empty() && !is_infinite(), 0.0f);
  nassertr(!_vector.almost_equal(LVector3f(0.0f, 0.0f, 0.0f)), 0.0f);

  // The formula for the distance from a point to the line based on
  // the quadratic equation.

  float A = dot(_vector, _vector);
  nassertr(A != 0.0f, 0.0f);
  LVector3f fc = _origin - point;
  float B = 2.0 * dot(_vector, fc);
  float fc_d2 = dot(fc, fc);

  float r2 = fc_d2 - B*B / 4.0*A;

  nassertr(!cnan(r2), 0.0f);
  return r2;
}
