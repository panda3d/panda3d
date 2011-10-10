// Filename: omniBoundingVolume.cxx
// Created by:  drose (22Jun00)
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

#include "omniBoundingVolume.h"
#include "boundingHexahedron.h"
#include "config_mathutil.h"

#include <math.h>

TypeHandle OmniBoundingVolume::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
BoundingVolume *OmniBoundingVolume::
make_copy() const {
  return new OmniBoundingVolume(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::get_approx_center
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 OmniBoundingVolume::
get_approx_center() const {
  return LPoint3(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::xform
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void OmniBoundingVolume::
xform(const LMatrix4 &) {
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void OmniBoundingVolume::
output(ostream &out) const {
  out << "omni";
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::extend_other
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool OmniBoundingVolume::
extend_other(BoundingVolume *other) const {
  other->set_infinite();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::around_other
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool OmniBoundingVolume::
around_other(BoundingVolume *other,
             const BoundingVolume **,
             const BoundingVolume **) const {
  other->set_infinite();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::contains_other
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int OmniBoundingVolume::
contains_other(const BoundingVolume *) const {
  return IF_possible | IF_some | IF_all;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::extend_by_point
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool OmniBoundingVolume::
extend_by_point(const LPoint3 &) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::extend_by_sphere
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool OmniBoundingVolume::
extend_by_sphere(const BoundingSphere *) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::extend_by_box
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool OmniBoundingVolume::
extend_by_box(const BoundingBox *) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::extend_by_hexahedron
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool OmniBoundingVolume::
extend_by_hexahedron(const BoundingHexahedron *) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::around_points
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool OmniBoundingVolume::
around_points(const LPoint3 *, const LPoint3 *) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::around_spheres
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool OmniBoundingVolume::
around_spheres(const BoundingVolume **,
               const BoundingVolume **) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::around_boxes
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool OmniBoundingVolume::
around_boxes(const BoundingVolume **,
             const BoundingVolume **) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::around_hexahedrons
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool OmniBoundingVolume::
around_hexahedrons(const BoundingVolume **,
                   const BoundingVolume **) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::contains_point
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int OmniBoundingVolume::
contains_point(const LPoint3 &) const {
  return IF_possible | IF_some | IF_all;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::contains_lineseg
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int OmniBoundingVolume::
contains_lineseg(const LPoint3 &, const LPoint3 &) const {
  return IF_possible | IF_some | IF_all;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::contains_sphere
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int OmniBoundingVolume::
contains_sphere(const BoundingSphere *) const {
  return IF_possible | IF_some | IF_all;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::contains_box
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int OmniBoundingVolume::
contains_box(const BoundingBox *) const {
  return IF_possible | IF_some | IF_all;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::contains_hexahedron
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int OmniBoundingVolume::
contains_hexahedron(const BoundingHexahedron *) const {
  return IF_possible | IF_some | IF_all;
}
