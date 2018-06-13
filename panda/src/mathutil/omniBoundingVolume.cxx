/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file omniBoundingVolume.cxx
 * @author drose
 * @date 2000-06-22
 */

#include "omniBoundingVolume.h"
#include "boundingHexahedron.h"
#include "config_mathutil.h"

#include <math.h>

TypeHandle OmniBoundingVolume::_type_handle;

/**
 *
 */
BoundingVolume *OmniBoundingVolume::
make_copy() const {
  return new OmniBoundingVolume(*this);
}

/**
 *
 */
LPoint3 OmniBoundingVolume::
get_approx_center() const {
  return LPoint3(0.0f, 0.0f, 0.0f);
}

/**
 *
 */
void OmniBoundingVolume::
xform(const LMatrix4 &) {
}

/**
 *
 */
void OmniBoundingVolume::
output(std::ostream &out) const {
  out << "omni";
}

/**
 *
 */
bool OmniBoundingVolume::
extend_other(BoundingVolume *other) const {
  other->set_infinite();
  return true;
}

/**
 *
 */
bool OmniBoundingVolume::
around_other(BoundingVolume *other,
             const BoundingVolume **,
             const BoundingVolume **) const {
  other->set_infinite();
  return true;
}

/**
 *
 */
int OmniBoundingVolume::
contains_other(const BoundingVolume *) const {
  return IF_possible | IF_some | IF_all;
}

/**
 *
 */
bool OmniBoundingVolume::
extend_by_point(const LPoint3 &) {
  return true;
}

/**
 *
 */
bool OmniBoundingVolume::
extend_by_sphere(const BoundingSphere *) {
  return true;
}

/**
 *
 */
bool OmniBoundingVolume::
extend_by_box(const BoundingBox *) {
  return true;
}

/**
 *
 */
bool OmniBoundingVolume::
extend_by_hexahedron(const BoundingHexahedron *) {
  return true;
}

/**
 *
 */
bool OmniBoundingVolume::
around_points(const LPoint3 *, const LPoint3 *) {
  return true;
}

/**
 *
 */
bool OmniBoundingVolume::
around_spheres(const BoundingVolume **,
               const BoundingVolume **) {
  return true;
}

/**
 *
 */
bool OmniBoundingVolume::
around_boxes(const BoundingVolume **,
             const BoundingVolume **) {
  return true;
}

/**
 *
 */
bool OmniBoundingVolume::
around_hexahedrons(const BoundingVolume **,
                   const BoundingVolume **) {
  return true;
}

/**
 *
 */
int OmniBoundingVolume::
contains_point(const LPoint3 &) const {
  return IF_possible | IF_some | IF_all;
}

/**
 *
 */
int OmniBoundingVolume::
contains_lineseg(const LPoint3 &, const LPoint3 &) const {
  return IF_possible | IF_some | IF_all;
}

/**
 *
 */
int OmniBoundingVolume::
contains_sphere(const BoundingSphere *) const {
  return IF_possible | IF_some | IF_all;
}

/**
 *
 */
int OmniBoundingVolume::
contains_box(const BoundingBox *) const {
  return IF_possible | IF_some | IF_all;
}

/**
 *
 */
int OmniBoundingVolume::
contains_hexahedron(const BoundingHexahedron *) const {
  return IF_possible | IF_some | IF_all;
}
