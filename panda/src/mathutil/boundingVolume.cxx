/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingVolume.cxx
 * @author drose
 * @date 1999-10-01
 */

#include "boundingVolume.h"
#include "finiteBoundingVolume.h"
#include "unionBoundingVolume.h"
#include "intersectionBoundingVolume.h"
#include "boundingBox.h"
#include "boundingLine.h"
#include "boundingPlane.h"
#include "boundingSphere.h"
#include "boundingHexahedron.h"
#include "config_mathutil.h"

#include "indent.h"

using std::istream;
using std::ostream;
using std::string;

TypeHandle BoundingVolume::_type_handle;


/**
 * Resets the volume to enclose only the volumes indicated.  Returns true if
 * successful, false if the volume doesn't know how to do that or can't do
 * that.
 */
bool BoundingVolume::
around(const BoundingVolume **first, const BoundingVolume **last) {
  _flags = F_empty;

  // Skip any empty volumes at the beginning of the list.  We want to get to
  // the first real volume.
  while (first != last && (*first)->is_empty()) {
    if ((*first)->is_infinite()) {
      // If we go around an infinite volume, we're infinite too.
      _flags = F_infinite;
      return true;
    }
    ++first;
  }

  bool okflag = true;

  if (first != last) {
    // Check for more infinite bounding volumes in the list.
    const BoundingVolume **bvi;
    for (bvi = first; bvi != last; ++bvi) {
      if ((*bvi)->is_infinite()) {
        _flags = F_infinite;
        return true;
      }
    }

    // This is a double-dispatch.  We call this virtual function on the volume
    // we were given, which will in turn call the appropriate virtual function
    // in our own class to perform the operation.
    if (!(*first)->around_other(this, first, last)) {
      okflag = false;
    }
  }

  return okflag;
}

/**
 *
 */
void BoundingVolume::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
GeometricBoundingVolume *BoundingVolume::
as_geometric_bounding_volume() {
  return nullptr;
}

/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
const GeometricBoundingVolume *BoundingVolume::
as_geometric_bounding_volume() const {
  return nullptr;
}

/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
const FiniteBoundingVolume *BoundingVolume::
as_finite_bounding_volume() const {
  return nullptr;
}

/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
const BoundingSphere *BoundingVolume::
as_bounding_sphere() const {
  return nullptr;
}

/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
const BoundingBox *BoundingVolume::
as_bounding_box() const {
  return nullptr;
}

/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
const BoundingHexahedron *BoundingVolume::
as_bounding_hexahedron() const {
  return nullptr;
}

/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
const BoundingLine *BoundingVolume::
as_bounding_line() const {
  return nullptr;
}

/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
const BoundingPlane *BoundingVolume::
as_bounding_plane() const {
  return nullptr;
}

/**
 * Returns the BoundsType corresponding to the indicated string.
 */
BoundingVolume::BoundsType BoundingVolume::
string_bounds_type(const string &str) {
  if (strcmp(str.c_str(), "default") == 0) {
    return BT_default;

  } else if (strcmp(str.c_str(), "best") == 0) {
    return BT_best;

  } else if (strcmp(str.c_str(), "fastest") == 0) {
    return BT_fastest;

  } else if (strcmp(str.c_str(), "sphere") == 0) {
    return BT_sphere;

  } else if (strcmp(str.c_str(), "box") == 0) {
    return BT_box;
  }

  return BT_default;
}

/**
 * Double-dispatch support: called by extend_other() when the type we're
 * extending by is known to be a sphere.
 */
bool BoundingVolume::
extend_by_sphere(const BoundingSphere *sphere) {
  return extend_by_finite(sphere);
}

/**
 * Double-dispatch support: called by extend_other() when the type we're
 * extending by is known to be a box.
 */
bool BoundingVolume::
extend_by_box(const BoundingBox *box) {
  return extend_by_finite(box);
}

/**
 * Double-dispatch support: called by extend_other() when the type we're
 * extending by is known to be a hexahedron.
 */
bool BoundingVolume::
extend_by_hexahedron(const BoundingHexahedron *hexahedron) {
  return extend_by_finite(hexahedron);
}

/**
 * Double-dispatch support: called by extend_other() when the type we're
 * extending by is known to be a line.
 */
bool BoundingVolume::
extend_by_line(const BoundingLine *line) {
  return extend_by_geometric(line);
}

/**
 * Double-dispatch support: called by extend_other() when the type we're
 * extending by is known to be a plane.
 */
bool BoundingVolume::
extend_by_plane(const BoundingPlane *plane) {
  return extend_by_geometric(plane);
}

/**
 * Double-dispatch support: called by extend_other() when the type we're
 * extending by is known to be a union.
 */
bool BoundingVolume::
extend_by_union(const UnionBoundingVolume *unionv) {
  return extend_by_geometric(unionv);
}

/**
 * Double-dispatch support: called by extend_other() when the type we're
 * extending by is known to be a intersection.
 */
bool BoundingVolume::
extend_by_intersection(const IntersectionBoundingVolume *intersection) {
  return extend_by_geometric(intersection);
}

/**
 * Generic handler for a FiniteBoundingVolume.
 */
bool BoundingVolume::
extend_by_finite(const FiniteBoundingVolume *volume) {
  return extend_by_geometric(volume);
}

/**
 * Generic handler for a GeometricBoundingVolume.
 */
bool BoundingVolume::
extend_by_geometric(const GeometricBoundingVolume *volume) {
  mathutil_cat.warning()
    << get_type() << "::extend_by_geometric() called with " << volume->get_type() << "\n";
  _flags = F_infinite;
  return false;
}

/**
 * Double-dispatch support: called by around_other() when the type of the
 * first element in the list is known to be a nonempty sphere.
 */
bool BoundingVolume::
around_spheres(const BoundingVolume **first, const BoundingVolume **last) {
  return around_finite(first, last);
}

/**
 * Double-dispatch support: called by around_other() when the type of the
 * first element in the list is known to be a nonempty box.
 */
bool BoundingVolume::
around_boxes(const BoundingVolume **first, const BoundingVolume **last) {
  return around_finite(first, last);
}

/**
 * Double-dispatch support: called by around_other() when the type of the
 * first element in the list is known to be a nonempty hexahedron.
 */
bool BoundingVolume::
around_hexahedrons(const BoundingVolume **first, const BoundingVolume **last) {
  return around_finite(first, last);
}

/**
 * Double-dispatch support: called by around_other() when the type of the
 * first element in the list is known to be a nonempty line.
 */
bool BoundingVolume::
around_lines(const BoundingVolume **first, const BoundingVolume **last) {
  return around_geometric(first, last);
}

/**
 * Double-dispatch support: called by around_other() when the type of the
 * first element in the list is known to be a nonempty plane.
 */
bool BoundingVolume::
around_planes(const BoundingVolume **first, const BoundingVolume **last) {
  return around_geometric(first, last);
}

/**
 * Double-dispatch support: called by around_other() when the type of the
 * first element in the list is known to be a union object.
 */
bool BoundingVolume::
around_unions(const BoundingVolume **first, const BoundingVolume **last) {
  return around_geometric(first, last);
}

/**
 * Double-dispatch support: called by around_other() when the type of the
 * first element in the list is known to be an intersection object.
 */
bool BoundingVolume::
around_intersections(const BoundingVolume **first, const BoundingVolume **last) {
  return around_geometric(first, last);
}

/**
 * Generic handler for a FiniteBoundingVolume.
 */
bool BoundingVolume::
around_finite(const BoundingVolume **first, const BoundingVolume **last) {
  return around_geometric(first, last);
}

/**
 * Generic handler for a GeometricBoundingVolume.
 */
bool BoundingVolume::
around_geometric(const BoundingVolume **first, const BoundingVolume **last) {
  mathutil_cat.warning()
    << get_type() << "::extend_by_geometric() called with " << first[0]->get_type() << "\n";
  _flags = F_infinite;
  return false;
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a sphere.
 */
int BoundingVolume::
contains_sphere(const BoundingSphere *sphere) const {
  return contains_finite(sphere);
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a box.
 */
int BoundingVolume::
contains_box(const BoundingBox *box) const {
  return contains_finite(box);
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a hexahedron.
 */
int BoundingVolume::
contains_hexahedron(const BoundingHexahedron *hexahedron) const {
  return contains_finite(hexahedron);
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a line.
 */
int BoundingVolume::
contains_line(const BoundingLine *line) const {
  return contains_geometric(line);
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a plane.
 */
int BoundingVolume::
contains_plane(const BoundingPlane *plane) const {
  return contains_geometric(plane);
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be a union object.
 */
int BoundingVolume::
contains_union(const UnionBoundingVolume *unionv) const {
  return unionv->other_contains_union(this);
}

/**
 * Double-dispatch support: called by contains_other() when the type we're
 * testing for intersection is known to be an intersection object.
 */
int BoundingVolume::
contains_intersection(const IntersectionBoundingVolume *intersection) const {
  return intersection->other_contains_intersection(this);
}

/**
 * Generic handler for a FiniteBoundingVolume.
 */
int BoundingVolume::
contains_finite(const FiniteBoundingVolume *volume) const {
  return contains_geometric(volume);
}

/**
 * Generic handler for a GeometricBoundingVolume.
 */
int BoundingVolume::
contains_geometric(const GeometricBoundingVolume *volume) const {
  mathutil_cat.warning()
    << get_type() << "::contains_geometric() called with " << volume->get_type() << "\n";
  return IF_dont_understand;
}

ostream &
operator << (ostream &out, BoundingVolume::BoundsType type) {
  switch (type) {
  case BoundingVolume::BT_default:
    return out << "default";

  case BoundingVolume::BT_best:
    return out << "best";

  case BoundingVolume::BT_fastest:
    return out << "fastest";

  case BoundingVolume::BT_sphere:
    return out << "sphere";

  case BoundingVolume::BT_box:
    return out << "box";
  }

  mathutil_cat.error()
    << "Invalid BoundingVolume::BoundsType value: " << (int)type << "\n";
  nassertr(false, out);
  return out;
}

istream &
operator >> (istream &in, BoundingVolume::BoundsType &type) {
  string word;
  in >> word;
  type = BoundingVolume::string_bounds_type(word);
  if (type == BoundingVolume::BT_default) {
    mathutil_cat->error()
      << "Invalid BoundingVolume::BoundsType string: " << word << "\n";
  }
  return in;
}
