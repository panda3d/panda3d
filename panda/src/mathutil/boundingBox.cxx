/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingBox.cxx
 * @author drose
 * @date 2007-05-31
 */

#include "boundingBox.h"
#include "boundingSphere.h"
#include "boundingHexahedron.h"
#include "boundingLine.h"
#include "boundingPlane.h"
#include "config_mathutil.h"
#include "dcast.h"

#include <math.h>
#include <algorithm>

using std::max;
using std::min;

const int BoundingBox::plane_def[6][3] = {
  { 0, 4, 5 },
  { 4, 6, 7 },
  { 6, 2, 3 },
  { 2, 0, 1 },
  { 1, 5, 7 },
  { 2, 6, 4 },
};

TypeHandle BoundingBox::_type_handle;

/**
 *
 */
BoundingVolume *BoundingBox::
make_copy() const {
  return new BoundingBox(*this);
}

/**
 *
 */
LPoint3 BoundingBox::
get_min() const {
  nassertr(!is_empty(), _min);
  nassertr(!is_infinite(), _min);
  return _min;
}

/**
 *
 */
LPoint3 BoundingBox::
get_max() const {
  nassertr(!is_empty(), _max);
  nassertr(!is_infinite(), _max);
  return _max;
}

/**
 *
 */
PN_stdfloat BoundingBox::
get_volume() const {
  nassertr(!is_infinite(), 0.0f);
  if (is_empty()) {
    return 0.0f;
  }

  // Volume of a box: width x depth x height
  return (_max[0] - _min[0]) * (_max[1] - _min[1]) * (_max[2] - _min[2]);
}

/**
 *
 */
LPoint3 BoundingBox::
get_approx_center() const {
  nassertr(!is_empty(), LPoint3::zero());
  nassertr(!is_infinite(), LPoint3::zero());
  return (_min + _max) * 0.5f;
}

/**
 *
 */
void BoundingBox::
xform(const LMatrix4 &mat) {
  nassertv(!mat.is_nan());

  if (!is_empty() && !is_infinite()) {
    // We need to transform the eight corners of the cube, and then determine
    // the new box.
    LPoint3 x = get_point(0) * mat;
    LPoint3 n = x;
    for (int i = 1; i < 8; ++i) {
      LPoint3 p = get_point(i) * mat;
      n.set(min(n[0], p[0]), min(n[1], p[1]), min(n[2], p[2]));
      x.set(max(x[0], p[0]), max(x[1], p[1]), max(x[2], p[2]));
    }
    _max = x;
    _min = n;
  }
}

/**
 *
 */
void BoundingBox::
output(std::ostream &out) const {
  if (is_empty()) {
    out << "bbox, empty";
  } else if (is_infinite()) {
    out << "bbox, infinite";
  } else {
    out << "bbox, (" << _min << ") to (" << _max << ")";
  }
}

/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
const BoundingBox *BoundingBox::
as_bounding_box() const {
  return this;
}

/**
 *
 */
bool BoundingBox::
extend_other(BoundingVolume *other) const {
  return other->extend_by_box(this);
}

/**
 *
 */
bool BoundingBox::
around_other(BoundingVolume *other,
             const BoundingVolume **first,
             const BoundingVolume **last) const {
  return other->around_boxes(first, last);
}

/**
 *
 */
int BoundingBox::
contains_other(const BoundingVolume *other) const {
  return other->contains_box(this);
}


/**
 *
 */
bool BoundingBox::
extend_by_point(const LPoint3 &point) {
  nassertr(!point.is_nan(), false);

  if (is_empty()) {
    _min = point;
    _max = point;
    _flags = 0;

  } else if (!is_infinite()) {
    _min.set(min(_min[0], point[0]), min(_min[1], point[1]), min(_min[2], point[2]));
    _max.set(max(_max[0], point[0]), max(_max[1], point[1]), max(_max[2], point[2]));
  }

  return true;
}

/**
 *
 */
bool BoundingBox::
extend_by_box(const BoundingBox *box) {
  nassertr(!box->is_empty() && !box->is_infinite(), false);
  nassertr(!is_infinite(), false);

  if (is_empty()) {
    _min = box->_min;
    _max = box->_max;
    _flags = 0;

  } else {
    _min.set(min(_min[0], box->_min[0]),
             min(_min[1], box->_min[1]),
             min(_min[2], box->_min[2]));
    _max.set(max(_max[0], box->_max[0]),
             max(_max[1], box->_max[1]),
             max(_max[2], box->_max[2]));
  }
  return true;
}

/**
 *
 */
bool BoundingBox::
extend_by_finite(const FiniteBoundingVolume *volume) {
  nassertr(!volume->is_empty() && !volume->is_infinite(), false);

  LVector3 min1 = volume->get_min();
  LVector3 max1 = volume->get_max();

  if (is_empty()) {
    _min = min1;
    _max = max1;
    _flags = 0;

  } else {
    _min.set(min(_min[0], min1[0]),
             min(_min[1], min1[1]),
             min(_min[2], min1[2]));
    _max.set(max(_max[0], max1[0]),
             max(_max[1], max1[1]),
             max(_max[2], max1[2]));
  }

  return true;
}

/**
 *
 */
bool BoundingBox::
around_points(const LPoint3 *first, const LPoint3 *last) {
  nassertr(first != last, false);

  // Get the minmax of all the points to construct a bounding box.
  const LPoint3 *p = first;

#ifndef NDEBUG
  // Skip any NaN points.
  int skipped_nan = 0;
  while (p != last && (*p).is_nan()) {
    ++p;
    ++skipped_nan;
  }
  if (p == last) {
    mathutil_cat.warning()
      << "BoundingBox around NaN\n";
    return false;
  }
#endif

  _min = *p;
  _max = *p;
  ++p;

#ifndef NDEBUG
  // Skip more NaN points.
  while (p != last && (*p).is_nan()) {
    ++p;
    ++skipped_nan;
  }
#endif

  while (p != last) {
#ifndef NDEBUG
    // Skip more NaN points.
    if ((*p).is_nan()) {
      ++skipped_nan;
    } else
#endif
      {
        _min.set(min(_min[0], (*p)[0]),
                 min(_min[1], (*p)[1]),
                 min(_min[2], (*p)[2]));
        _max.set(max(_max[0], (*p)[0]),
                 max(_max[1], (*p)[1]),
                 max(_max[2], (*p)[2]));
      }
    ++p;
  }

#ifndef NDEBUG
  if (skipped_nan != 0) {
    mathutil_cat.warning()
      << "BoundingBox ignored " << skipped_nan << " NaN points of "
      << (last - first) << " total.\n";
  }
#endif

  _flags = 0;

  return true;
}

/**
 *
 */
bool BoundingBox::
around_finite(const BoundingVolume **first,
                 const BoundingVolume **last) {
  nassertr(first != last, false);

  // We're given a set of bounding volumes, at least the first one of which is
  // guaranteed to be finite and nonempty.  Some others may not be.

  // First, get the box of all the points to construct a bounding box.
  const BoundingVolume **p = first;
  nassertr(!(*p)->is_empty() && !(*p)->is_infinite(), false);
  const FiniteBoundingVolume *vol = DCAST(FiniteBoundingVolume, *p);
  _min = vol->get_min();
  _max = vol->get_max();

  for (++p; p != last; ++p) {
    nassertr(!(*p)->is_infinite(), false);
    if (!(*p)->is_empty()) {
      const FiniteBoundingVolume *vol = DCAST(FiniteBoundingVolume, *p);
      LPoint3 min1 = vol->get_min();
      LPoint3 max1 = vol->get_max();
      _min.set(min(_min[0], min1[0]),
               min(_min[1], min1[1]),
               min(_min[2], min1[2]));
      _max.set(max(_max[0], max1[0]),
               max(_max[1], max1[1]),
               max(_max[2], max1[2]));
    }
  }

  _flags = 0;
  return true;
}

/**
 *
 */
int BoundingBox::
contains_point(const LPoint3 &point) const {
  nassertr(!point.is_nan(), IF_no_intersection);

  if (is_empty()) {
    return IF_no_intersection;

  } else if (is_infinite()) {
    return IF_possible | IF_some | IF_all;

  } else {
    if (point[0] >= _min[0] && point[0] <= _max[0] &&
        point[1] >= _min[1] && point[1] <= _max[1] &&
        point[2] >= _min[2] && point[2] <= _max[2]) {
      return IF_possible | IF_some | IF_all;
    } else {
      return IF_no_intersection;
    }
  }
}

/**
 *
 */
int BoundingBox::
contains_lineseg(const LPoint3 &a, const LPoint3 &b) const {
  nassertr(!a.is_nan() && !b.is_nan(), IF_no_intersection);

  if (a == b) {
    return contains_point(a);
  }
  if (is_empty()) {
    return IF_no_intersection;

  } else if (is_infinite()) {
    return IF_possible | IF_some | IF_all;

  } else {
    // Set a bit for each plane a and b are on the wrong side of.
    unsigned int a_bits = 0;

    if (a[0] < _min[0]) {
      a_bits |= 0x01;
    } else if (a[0] > _max[0]) {
      a_bits |= 0x02;
    }

    if (a[1] < _min[1]) {
      a_bits |= 0x04;
    } else if (a[1] > _max[1]) {
      a_bits |= 0x08;
    }

    if (a[2] < _min[2]) {
      a_bits |= 0x10;
    } else if (a[2] > _max[2]) {
      a_bits |= 0x20;
    }

    unsigned int b_bits = 0;

    if (b[0] < _min[0]) {
      b_bits |= 0x01;
    } else if (b[0] > _max[0]) {
      b_bits |= 0x02;
    }

    if (b[1] < _min[1]) {
      b_bits |= 0x04;
    } else if (b[1] > _max[1]) {
      b_bits |= 0x08;
    }

    if (b[2] < _min[2]) {
      b_bits |= 0x10;
    } else if (b[2] > _max[2]) {
      b_bits |= 0x20;
    }

    if ((a_bits & b_bits) != 0) {
      // If there are any bits in common, the segment is wholly outside the
      // box (both points are on the wrong side of the same plane).
      return IF_no_intersection;

    } else if ((a_bits | b_bits) == 0) {
      // If there are no bits at all, the segment is wholly within the box.
      return IF_possible | IF_some | IF_all;

    } else if (a_bits == 0 || b_bits == 0) {
      // If either point is within the box, the segment is partially within
      // the box.
      return IF_possible | IF_some;

    } else {
      unsigned int differ = (a_bits ^ b_bits);
      if (differ == 0x03 || differ == 0x0c || differ == 0x30) {
        // If the line segment stretches straight across the box, the segment
        // is partially within.
        return IF_possible | IF_some;

      } else {
        // Otherwise, it's hard to tell whether it does or doesn't.
        return IF_possible;
      }
    }
  }
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a box.
 */
int BoundingBox::
contains_box(const BoundingBox *box) const {
  nassertr(!is_empty() && !is_infinite(), 0);
  nassertr(!box->is_empty() && !box->is_infinite(), 0);

  const LPoint3 &min1 = box->get_minq();
  const LPoint3 &max1 = box->get_maxq();

  if (min1[0] >= _min[0] && max1[0] <= _max[0] &&
      min1[1] >= _min[1] && max1[1] <= _max[1] &&
      min1[2] >= _min[2] && max1[2] <= _max[2]) {
    // The other volume is completely within this volume.
    return IF_possible | IF_some | IF_all;

  } else if (max1[0] >= _min[0] && min1[0] <= _max[0] &&
             max1[1] >= _min[1] && min1[1] <= _max[1] &&
             max1[2] >= _min[2] && min1[2] <= _max[2]) {
    // The other volume is partially within this volume.
    return IF_possible;

  } else {
    // The other volume is not within this volume.
    return IF_no_intersection;
  }
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a hexahedron.
 */
int BoundingBox::
contains_hexahedron(const BoundingHexahedron *hexahedron) const {
  // First, try the quick bounding-box test.  If that's decisive, we'll accept
  // it.
  int result = contains_finite(hexahedron);
  if (result == IF_no_intersection || ((result & IF_all) != 0)) {
    return result;
  }

  // If that was inconclusive, we'll look more closely with the somewhat more
  // expensive reverse answer.
  return hexahedron->contains_box(this) & ~IF_all;
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a line.
 */
int BoundingBox::
contains_line(const BoundingLine *line) const {
  return line->contains_box(this) & ~IF_all;
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a plane.
 */
int BoundingBox::
contains_plane(const BoundingPlane *plane) const {
  return plane->contains_box(this) & ~IF_all;
}

/**
 *
 */
int BoundingBox::
contains_finite(const FiniteBoundingVolume *volume) const {
  nassertr(!is_empty() && !is_infinite(), 0);
  nassertr(!volume->is_empty() && !volume->is_infinite(), 0);

  LPoint3 min1 = volume->get_min();
  LPoint3 max1 = volume->get_max();

  if (min1[0] >= _min[0] && max1[0] <= _max[0] &&
      min1[1] >= _min[1] && max1[1] <= _max[1] &&
      min1[2] >= _min[2] && max1[2] <= _max[2]) {
    // The other volume is completely within this volume.
    return IF_possible | IF_some | IF_all;

  } else if (max1[0] >= _min[0] && min1[0] <= _max[0] &&
             max1[1] >= _min[1] && min1[1] <= _max[1] &&
             max1[2] >= _min[2] && min1[2] <= _max[2]) {
    // The other volume is partially within this volume.
    return IF_possible;

  } else {
    // The other volume is not within this volume.
    return IF_no_intersection;
  }
}
