// Filename: omniBoundingVolume.cxx
// Created by:  drose (22Jun00)
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
LPoint3f OmniBoundingVolume::
get_approx_center() const {
  return LPoint3f(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::xform
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void OmniBoundingVolume::
xform(const LMatrix4f &) {
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
extend_by_point(const LPoint3f &) {
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
around_points(const LPoint3f *, const LPoint3f *) {
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
contains_point(const LPoint3f &) const {
  return IF_possible | IF_some | IF_all;
}

////////////////////////////////////////////////////////////////////
//     Function: OmniBoundingVolume::contains_lineseg
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int OmniBoundingVolume::
contains_lineseg(const LPoint3f &, const LPoint3f &) const {
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
//     Function: OmniBoundingVolume::contains_hexahedron
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int OmniBoundingVolume::
contains_hexahedron(const BoundingHexahedron *) const {
  return IF_possible | IF_some | IF_all;
}
