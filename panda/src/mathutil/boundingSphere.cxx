// Filename: boundingSphere.cxx
// Created by:  drose (01Oct99)
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

#include "boundingSphere.h"
#include "boundingHexahedron.h"
#include "boundingLine.h"
#include "config_mathutil.h"
#include "dcast.h"

#include <math.h>
#include <algorithm>

TypeHandle BoundingSphere::_type_handle;

BoundingVolume *BoundingSphere::
make_copy() const {
  return new BoundingSphere(*this);
}

LPoint3f BoundingSphere::
get_min() const {
  nassertr(!is_empty(), LPoint3f(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3f(0.0f, 0.0f, 0.0f));
  return LPoint3f(_center[0] - _radius,
                  _center[1] - _radius,
                  _center[2] - _radius);
}

LPoint3f BoundingSphere::
get_max() const {
  nassertr(!is_empty(), LPoint3f(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3f(0.0f, 0.0f, 0.0f));
  return LPoint3f(_center[0] + _radius,
                  _center[1] + _radius,
                  _center[2] + _radius);
}

LPoint3f BoundingSphere::
get_approx_center() const {
  nassertr(!is_empty(), LPoint3f(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3f(0.0f, 0.0f, 0.0f));
  return get_center();
}

void BoundingSphere::
xform(const LMatrix4f &mat) {
  nassertv(!mat.is_nan());

  if (!is_empty() && !is_infinite()) {
    // First, determine the longest axis of the matrix, in case it
    // contains a non-proportionate scale.

/*
    LVector3f x,y,z;
        mat.get_row3(x,0);
        mat.get_row3(y,1);
        mat.get_row3(z,2);

    float xd = dot(x, x);
    float yd = dot(y, y);
    float zd = dot(z, z);
*/
    float xd,yd,zd,scale;

        #define ROW_DOTTED(mat,ROWNUM)                        \
            (mat._m.m._##ROWNUM##0*mat._m.m._##ROWNUM##0 +    \
             mat._m.m._##ROWNUM##1*mat._m.m._##ROWNUM##1 +    \
             mat._m.m._##ROWNUM##2*mat._m.m._##ROWNUM##2)

    xd = ROW_DOTTED(mat,0);
    yd = ROW_DOTTED(mat,1);
    zd = ROW_DOTTED(mat,2);

        scale = max(xd,yd);
        scale = max(scale,zd);
        scale = sqrtf(scale);

    // Transform the radius
    _radius *= scale;

    // Transform the center
    _center = _center * mat;
  }
}

void BoundingSphere::
output(ostream &out) const {
  if (is_empty()) {
    out << "bsphere, empty";
  } else if (is_infinite()) {
    out << "bsphere, infinite";
  } else {
    out << "bsphere, c (" << _center << "), r " << _radius;
  }
}

bool BoundingSphere::
extend_other(BoundingVolume *other) const {
  return other->extend_by_sphere(this);
}

bool BoundingSphere::
around_other(BoundingVolume *other,
             const BoundingVolume **first,
             const BoundingVolume **last) const {
  return other->around_spheres(first, last);
}

int BoundingSphere::
contains_other(const BoundingVolume *other) const {
  return other->contains_sphere(this);
}


bool BoundingSphere::
extend_by_point(const LPoint3f &point) {
  nassertr(!point.is_nan(), false);

  if (is_empty()) {
    _center = point;
    _radius = 0.0f;
    _flags = 0;
  } else if (!is_infinite()) {
    LVector3f v = point - _center;
    float dist2 = dot(v, v);
    if (dist2 > _radius * _radius) {
      _radius = sqrtf(dist2);
    }
  }
  return true;
}

bool BoundingSphere::
extend_by_sphere(const BoundingSphere *sphere) {
  nassertr(!sphere->is_empty() && !sphere->is_infinite(), false);
  nassertr(!is_infinite(), false);

  if (is_empty()) {
    _center = sphere->_center;
    _radius = sphere->_radius;
    _flags = 0;
  } else {
    float dist = length(sphere->_center - _center);

    _radius = max(_radius, dist + sphere->_radius);
  }
  return true;
}

bool BoundingSphere::
extend_by_hexahedron(const BoundingHexahedron *hexahedron) {
  return extend_by_finite(hexahedron);
}

bool BoundingSphere::
extend_by_finite(const FiniteBoundingVolume *volume) {
  nassertr(!volume->is_empty(), false);

  LVector3f min1 = volume->get_min();
  LVector3f max1 = volume->get_max();

  if (is_empty()) {
    _center = (min1 + max1) * 0.5f;
    _radius = length(LVector3f(max1 - _center));
    _flags = 0;
  } else {
    LVector3f v = max1 - _center;
    float dist2 = dot(v, v);

    if (dist2 > _radius * _radius) {
      _radius = sqrtf(dist2);
    }
  }

  return true;
}

bool BoundingSphere::
around_points(const LPoint3f *first, const LPoint3f *last) {
  nassertr(first != last, false);

  // First, get the minmax of all the points to construct a bounding
  // box.
  const LPoint3f *p = first;

#ifndef NDEBUG
  // Skip any NaN points.
  int skipped_nan = 0;
  while (p != last && (*p).is_nan()) {
    ++p;
    ++skipped_nan;
  }
  if (p == last) {
    mathutil_cat.warning()
      << "BoundingSphere around NaN\n";
    return false;
  }
#endif

  LPoint3f min_box = *p;
  LPoint3f max_box = *p;
  ++p;

#ifndef NDEBUG
  // Skip more NaN points.
  while (p != last && (*p).is_nan()) {
    ++p;
    ++skipped_nan;
  }
#endif

  if (p == last) {
    // Only one point; we have a radius of zero.  This is not the same
    // thing as an empty sphere, because our volume contains one
    // point; an empty sphere contains no points.
    _center = min_box;
    _radius = 0.0f;

  } else {
    // More than one point; we have a nonzero radius.
    while (p != last) {
#ifndef NDEBUG
      // Skip more NaN points.
      if ((*p).is_nan()) {
        ++skipped_nan;
      } else
#endif
        {
          min_box.set(min(min_box[0], (*p)[0]),
                      min(min_box[1], (*p)[1]),
                      min(min_box[2], (*p)[2]));
          max_box.set(max(max_box[0], (*p)[0]),
                      max(max_box[1], (*p)[1]),
                      max(max_box[2], (*p)[2]));
        }
      ++p;
    }

    // Now take the center of the bounding box as the center of the sphere.
    _center = (min_box + max_box) * 0.5f;

    // Now walk back through to get the max distance from center.
    float max_dist2 = 0.0f;
    for (p = first; p != last; ++p) {
      LVector3f v = (*p) - _center;
      float dist2 = dot(v, v);
      max_dist2 = max(max_dist2, dist2);
    }

    _radius = sqrtf(max_dist2);
  }

#ifndef NDEBUG
  if (skipped_nan != 0) {
    mathutil_cat.warning()
      << "BoundingSphere ignored " << skipped_nan << " NaN points of "
      << (last - first) << " total.\n";
  }
#endif

  _flags = 0;

  return true;
}

bool BoundingSphere::
around_spheres(const BoundingVolume **first,
               const BoundingVolume **last) {
  return around_finite(first, last);
}

bool BoundingSphere::
around_hexahedrons(const BoundingVolume **first,
                   const BoundingVolume **last) {
  return around_finite(first, last);
}

bool BoundingSphere::
around_finite(const BoundingVolume **first,
              const BoundingVolume **last) {
  nassertr(first != last, false);

  // We're given a set of bounding volumes, at least the first one of
  // which is guaranteed to be finite and nonempty.  Some others may
  // not be.

  // First, get the minmax of all the points to construct a bounding
  // box.
  const BoundingVolume **p = first;
  nassertr(!(*p)->is_empty() && !(*p)->is_infinite(), false);
  nassertr((*p)->is_of_type(FiniteBoundingVolume::get_class_type()), false);
  const FiniteBoundingVolume *vol = DCAST(FiniteBoundingVolume, *p);
  LPoint3f min_box = vol->get_min();
  LPoint3f max_box = vol->get_max();

  bool any_unknown = false;

  for (++p; p != last; ++p) {
    nassertr(!(*p)->is_infinite(), false);
    if (!(*p)->is_empty() &&
        (*p)->is_of_type(FiniteBoundingVolume::get_class_type())) {
      const FiniteBoundingVolume *vol = DCAST(FiniteBoundingVolume, *p);
      LPoint3f min1 = vol->get_min();
      LPoint3f max1 = vol->get_max();
      min_box.set(min(min_box[0], min1[0]),
                  min(min_box[1], min1[1]),
                  min(min_box[2], min1[2]));
      max_box.set(max(max_box[0], max1[0]),
                  max(max_box[1], max1[1]),
                  max(max_box[2], max1[2]));

      if (!(*p)->is_of_type(BoundingSphere::get_class_type())) {
        any_unknown = true;
      }
    }
  }

  // Now take the center of the bounding box as the center of the sphere.
  _center = (min_box + max_box) * 0.5f;

  if (any_unknown) {
    // If we have any volumes in the list that we don't know what to
    // do with, we'll have to make the bounding sphere large enough to
    // enclose the bounding box.  Less than ideal, but too bad.
    _radius = length(max_box - _center);

  } else {
    // Otherwise, we do understand all the volumes in the list; make
    // the sphere as tight as we can.
    _radius = 0.0f;
    for (p = first; p != last; ++p) {
      if (!(*p)->is_empty()) {
        if ((*p)->is_of_type(BoundingSphere::get_class_type())) {
          const BoundingSphere *sphere = DCAST(BoundingSphere, *p);
          float dist = length(sphere->_center - _center);
          _radius = max(_radius, dist + sphere->_radius);
        } else {
          // Shouldn't get here, unless we missed a type from above.
          mathutil_cat.error()
            << "Unexpected type in BoundingSphere::around_finite()\n";
          nassertr(false, false);
        }
      }
    }
  }

  _flags = 0;
  return true;
}

int BoundingSphere::
contains_point(const LPoint3f &point) const {
  nassertr(!point.is_nan(), IF_no_intersection);

  if (is_empty()) {
    return IF_no_intersection;

  } else if (is_infinite()) {
    return IF_possible | IF_some | IF_all;

  } else {
    LVector3f v = point - _center;
    float dist2 = dot(v, v);
    return (dist2 <= _radius * _radius) ?
      IF_possible | IF_some | IF_all : IF_no_intersection;
  }
}

int BoundingSphere::
contains_lineseg(const LPoint3f &a, const LPoint3f &b) const {
  nassertr(!a.is_nan() && !b.is_nan(), IF_no_intersection);

  if (a == b) {
    return contains_point(a);
  }
  if (is_empty()) {
    return IF_no_intersection;

  } else if (is_infinite()) {
    return IF_possible | IF_some | IF_all;

  } else {
    LPoint3f from = a;
    LVector3f delta = b - a;
    float t1, t2;

    // Solve the equation for the intersection of a line with a sphere
    // using the quadratic equation.
    float A = dot(delta, delta);

    nassertr(A != 0.0f, 0);    // Trivial line segment.

    LVector3f fc = from - _center;
    float B = 2.0f * dot(delta, fc);
    float C = dot(fc, fc) - _radius * _radius;

    float radical = B*B - 4.0f*A*C;

    if (IS_NEARLY_ZERO(radical)) {
      // Tangent.
      t1 = t2 = -B / (2.0f*A);
      return (t1 >= 0.0f && t1 <= 1.0f) ?
                 IF_possible | IF_some : IF_no_intersection;
    }

    if (radical < 0.0f) {
      // No real roots: no intersection with the line.
      return IF_no_intersection;
    }

        float reciprocal_2A = 1.0f/(2.0f*A);
    float sqrt_radical = sqrtf(radical);

    t1 = ( -B - sqrt_radical ) * reciprocal_2A;
    t2 = ( -B + sqrt_radical ) * reciprocal_2A;

    if (t1 >= 0.0f && t2 <= 1.0f) {
      return IF_possible | IF_some | IF_all;
    } else if (t1 <= 1.0f && t2 >= 0.0f) {
      return IF_possible | IF_some;
    } else {
      return IF_no_intersection;
    }
  }
}

int BoundingSphere::
contains_sphere(const BoundingSphere *sphere) const {
  nassertr(!is_empty() && !is_infinite(), 0);
  nassertr(!sphere->is_empty() && !sphere->is_infinite(), 0);

  LVector3f v = sphere->_center - _center;
  float dist2 = dot(v, v);

  if (_radius >= sphere->_radius &&
      dist2 <= (_radius - sphere->_radius) * (_radius - sphere->_radius)) {
    // The other sphere is completely within this sphere.
    return IF_possible | IF_some | IF_all;

  } else if (dist2 > (_radius + sphere->_radius) * (_radius + sphere->_radius)) {
    // The other sphere is completely outside this sphere.
    return IF_no_intersection;

  } else {
    // The other sphere is partially within this sphere.
    return IF_possible | IF_some;
  }
}

int BoundingSphere::
contains_hexahedron(const BoundingHexahedron *hexahedron) const {
  return hexahedron->contains_sphere(this) & ~IF_all;
}

int BoundingSphere::
contains_line(const BoundingLine *line) const {
  return line->contains_sphere(this) & ~IF_all;
}
