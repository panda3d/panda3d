// Filename: boundingHexahedron.cxx
// Created by:  drose (03Oct99)
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

#include "boundingHexahedron.h"
#include "boundingSphere.h"
#include "config_mathutil.h"

#include <math.h>
#include <algorithm>

TypeHandle BoundingHexahedron::_type_handle;

BoundingHexahedron::
BoundingHexahedron(const Frustumf &frustum, bool is_ortho,
                   CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  float fs = 1.0f;
  if (!is_ortho) {
    fs = frustum._ffar / frustum._fnear;
  }

  // We build the points based on a Z-up right-handed frustum.  If the
  // requested coordinate system is otherwise, we'll convert it in a
  // second pass.
  _points[0].set(frustum._l * fs, frustum._ffar, frustum._b * fs);
  _points[1].set(frustum._r * fs, frustum._ffar, frustum._b * fs);
  _points[2].set(frustum._r * fs, frustum._ffar, frustum._t * fs);
  _points[3].set(frustum._l * fs, frustum._ffar, frustum._t * fs);
  _points[4].set(frustum._l, frustum._fnear, frustum._b);
  _points[5].set(frustum._r, frustum._fnear, frustum._b);
  _points[6].set(frustum._r, frustum._fnear, frustum._t);
  _points[7].set(frustum._l, frustum._fnear, frustum._t);

  _flags = 0;

  // Now fix the coordinate system, if necessary.
  if (cs == CS_zup_right) {
    set_centroid();
    set_planes();
  } else {
    xform(LMatrix4f::convert_mat(CS_zup_right, cs));
  }
}

BoundingHexahedron::
BoundingHexahedron(const LPoint3f &fll, const LPoint3f &flr,
                   const LPoint3f &fur, const LPoint3f &ful,
                   const LPoint3f &nll, const LPoint3f &nlr,
                   const LPoint3f &nur, const LPoint3f &nul) {
  _points[0] = fll;
  _points[1] = flr;
  _points[2] = fur;
  _points[3] = ful;
  _points[4] = nll;
  _points[5] = nlr;
  _points[6] = nur;
  _points[7] = nul;

  _flags = 0;
  set_centroid();
  set_planes();
}

BoundingVolume *BoundingHexahedron::
make_copy() const {
  return new BoundingHexahedron(*this);
}

LPoint3f BoundingHexahedron::
get_min() const {
  nassertr(!is_empty(), LPoint3f(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3f(0.0f, 0.0f, 0.0f));
  int i;
  LPoint3f m = _points[0];
  for (i = 1; i < num_points; i++) {
    m.set(min(m[0], _points[i][0]),
          min(m[1], _points[i][1]),
          min(m[2], _points[i][2]));
  }
  return m;
}

LPoint3f BoundingHexahedron::
get_max() const {
  nassertr(!is_empty(), LPoint3f(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3f(0.0f, 0.0f, 0.0f));
  int i;
  LPoint3f m = _points[0];
  for (i = 1; i < num_points; i++) {
    m.set(max(m[0], _points[i][0]),
          max(m[1], _points[i][1]),
          max(m[2], _points[i][2]));
  }
  return m;
}

LPoint3f BoundingHexahedron::
get_approx_center() const {
  nassertr(!is_empty(), LPoint3f(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3f(0.0f, 0.0f, 0.0f));
  return _centroid;
}

void BoundingHexahedron::
xform(const LMatrix4f &mat) {
  if (!is_empty() && !is_infinite()) {
    for (int i = 0; i < num_points; i++) {
      _points[i] = _points[i] * mat;
    }
    set_centroid();
    set_planes();
  }
}

void BoundingHexahedron::
output(ostream &out) const {
  if (is_empty()) {
    out << "bhexahedron, empty";
  } else if (is_infinite()) {
    out << "bhexahedron, infinite";
  } else {
    out << "bhexahedron, min " << get_min() << " max " << get_max();
  }
}

void BoundingHexahedron::
write(ostream &out, int indent_level) const {
  if (is_empty()) {
    indent(out, indent_level) << "bhexahedron, empty\n";
  } else if (is_infinite()) {
    out << "bhexahedron, infinite\n";
  } else {
    indent(out, indent_level)
      << "bhexahedron, min " << get_min() << " max " << get_max() << ":\n";
    int i;
    for (i = 0; i < num_points; i++) {
      indent(out, indent_level + 2) << _points[i] << "\n";
    }
    indent(out, indent_level + 2) << "centroid is " << _centroid << "\n";
  }
}

bool BoundingHexahedron::
extend_other(BoundingVolume *other) const {
  return other->extend_by_hexahedron(this);
}

bool BoundingHexahedron::
around_other(BoundingVolume *other,
             const BoundingVolume **first,
             const BoundingVolume **last) const {
  return other->around_hexahedrons(first, last);
}

int BoundingHexahedron::
contains_other(const BoundingVolume *other) const {
  return other->contains_hexahedron(this);
}


bool BoundingHexahedron::
extend_by_point(const LPoint3f &) {
  mathutil_cat.error()
    << "BoundingHexahedron::extend_by_point() called\n";
  return false;
}

bool BoundingHexahedron::
extend_by_sphere(const BoundingSphere *) {
  mathutil_cat.error()
    << "BoundingHexahedron::extend_by_sphere() called\n";
  return false;
}

bool BoundingHexahedron::
extend_by_hexahedron(const BoundingHexahedron *) {
  mathutil_cat.error()
    << "BoundingHexahedron::extend_by_hexahedron() called\n";
  return false;
}

bool BoundingHexahedron::
around_points(const LPoint3f *, const LPoint3f *) {
  mathutil_cat.error()
    << "BoundingHexahedron::around_points() called\n";
  return false;
}

bool BoundingHexahedron::
around_spheres(const BoundingVolume **,
               const BoundingVolume **) {
  mathutil_cat.error()
    << "BoundingHexahedron::around_spheres() called\n";
  return false;
}

bool BoundingHexahedron::
around_hexahedrons(const BoundingVolume **,
                   const BoundingVolume **) {
  mathutil_cat.error()
    << "BoundingHexahedron::around_hexahedrons() called\n";
  return false;
}

int BoundingHexahedron::
contains_point(const LPoint3f &point) const {
  if (is_empty()) {
    return IF_no_intersection;

  } else if (is_infinite()) {
    return IF_possible | IF_some | IF_all;

  } else {
    // The hexahedron contains the point iff the point is behind all of
    // the planes.
    for (int i = 0; i < num_planes; i++) {
      const Planef &p = _planes[i];
      if (p.dist_to_plane(point) > 0.0f) {
        return IF_no_intersection;
      }
    }
    return IF_possible | IF_some | IF_all;
  }
}

int BoundingHexahedron::
contains_lineseg(const LPoint3f &a, const LPoint3f &b) const {
  if (is_empty()) {
    return IF_no_intersection;

  } else if (is_infinite()) {
    return IF_possible | IF_some | IF_all;

  } else {
    // The hexahedron does not contains the line segment if both points
    // are in front of any one plane.
    for (int i = 0; i < num_planes; i++) {
      const Planef &p = _planes[i];
      if (p.dist_to_plane(a) > 0.0f ||
          p.dist_to_plane(b) > 0.0f) {
        return IF_no_intersection;
      }
    }

    // If there is no plane that both points are in front of, the
    // hexahedron may or may not contain the line segment.  For the
    // moment, we won't bother to check that more thoroughly, though.
    return IF_possible;
  }
}

int BoundingHexahedron::
contains_sphere(const BoundingSphere *sphere) const {
  nassertr(!is_empty(), 0);

  // The hexahedron contains the sphere iff the sphere is at least
  // partly behind all of the planes.
  const LPoint3f &center = sphere->get_center();
  float radius = sphere->get_radius();

  int result = IF_possible | IF_some | IF_all;

  for (int i = 0; i < num_planes; i++) {
    const Planef &p = _planes[i];
    float dist = p.dist_to_plane(center);

    if (dist > radius) {
      // The sphere is completely in front of this plane; it's thus
      // completely outside of the hexahedron.
      return IF_no_intersection;

    } else if (dist > -radius) {
      // The sphere is not completely behind this plane, but some of
      // it is.
      result &= ~IF_all;
    }
  }

  return result;
}

int BoundingHexahedron::
contains_hexahedron(const BoundingHexahedron *hexahedron) const {
  nassertr(!is_empty(), 0);
  nassertr(!hexahedron->is_empty(), 0);

  // Check minmax.
  LPoint3f min1 = get_min();
  LPoint3f min2 = hexahedron->get_min();
  LPoint3f max1 = get_max();
  LPoint3f max2 = hexahedron->get_max();

  if (min1[0] > max2[0] || min1[1] > max2[1] || min1[2] > max2[2] ||
      min2[0] > max1[0] || min2[1] > max1[1] || min2[2] > max1[2] ||
      max1[0] < min2[0] || max1[1] < min2[1] || max1[2] < min2[2] ||
      max2[0] < min1[0] || max2[1] < min1[1] || max2[2] < min1[2]) {
    return IF_no_intersection;
  }

  int result = IF_possible | IF_all;

  for (int i = 0; i < num_points; i++) {
    if (contains_point(hexahedron->_points[i])) {
      result |= IF_some;
    } else {
      result &= ~IF_all;
    }
  }

  return result;
}

void BoundingHexahedron::
set_planes() {
  _planes[0] = Planef(_points[0], _points[3], _points[2]);

  // Test to see if we have accidentally inverted our frustum by
  // transforming it with a -1 matrix.  We do this by ensuring that
  // the centroid is in front of all of the planes (actually, we only
  // need to test the first plane).
  if (_planes[0].dist_to_plane(_centroid) > 0) {
    // Oops!  We're flipped!  Rebuild the planes in the opposite
    // direction.
    _planes[0] = Planef(_points[0], _points[2], _points[3]);
    _planes[1] = Planef(_points[0], _points[5], _points[1]);
    _planes[2] = Planef(_points[1], _points[6], _points[2]);
    _planes[3] = Planef(_points[2], _points[7], _points[3]);
    _planes[4] = Planef(_points[3], _points[4], _points[0]);
    _planes[5] = Planef(_points[4], _points[7], _points[6]);

    nassertv(_planes[0].dist_to_plane(_centroid) < 0);

  } else {
    // No, a perfectly sane universe.
    _planes[1] = Planef(_points[0], _points[1], _points[5]);
    _planes[2] = Planef(_points[1], _points[2], _points[6]);
    _planes[3] = Planef(_points[2], _points[3], _points[7]);
    _planes[4] = Planef(_points[3], _points[0], _points[4]);
    _planes[5] = Planef(_points[4], _points[6], _points[7]);
  }
}

void BoundingHexahedron::
set_centroid() {
  LPoint3f net = _points[0];
  for (int i = 1; i < num_points; i++) {
    net += _points[i];
  }
  _centroid = net / (float)num_points;
}
