// Filename: boundingPlane.cxx
// Created by:  drose (19Aug05)
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

#include "boundingPlane.h"
#include "boundingSphere.h"
#include "boundingBox.h"
#include "boundingHexahedron.h"
#include "config_mathutil.h"

TypeHandle BoundingPlane::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
BoundingVolume *BoundingPlane::
make_copy() const {
  return new BoundingPlane(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::get_approx_center
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3 BoundingPlane::
get_approx_center() const {
  nassertr(!is_empty(), LPoint3(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3(0.0f, 0.0f, 0.0f));
  return _plane.get_point();
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::xform
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void BoundingPlane::
xform(const LMatrix4 &mat) {
  nassertv(!mat.is_nan());

  if (!is_empty() && !is_infinite()) {
    _plane.xform(mat);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void BoundingPlane::
output(ostream &out) const {
  if (is_empty()) {
    out << "bplane, empty";
  } else if (is_infinite()) {
    out << "bplane, infinite";
  } else {
    out << "bplane: " << _plane;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::as_bounding_plane
//       Access: Public, Virtual
//  Description: Virtual downcast method.  Returns this object as a
//               pointer of the indicated type, if it is in fact that
//               type.  Returns NULL if it is not that type.
////////////////////////////////////////////////////////////////////
const BoundingPlane *BoundingPlane::
as_bounding_plane() const {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::extend_other
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool BoundingPlane::
extend_other(BoundingVolume *other) const {
  return other->extend_by_plane(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::around_other
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool BoundingPlane::
around_other(BoundingVolume *other,
             const BoundingVolume **first,
             const BoundingVolume **last) const {
  return other->around_planes(first, last);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::contains_other
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int BoundingPlane::
contains_other(const BoundingVolume *other) const {
  return other->contains_plane(this);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::extend_by_plane
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool BoundingPlane::
extend_by_plane(const BoundingPlane *plane) {
  nassertr(!plane->is_empty() && !plane->is_infinite(), false);
  nassertr(!is_infinite(), false);

  if (is_empty()) {
    _plane = plane->get_plane();
    _flags = 0;
  } else {
    _flags = F_infinite;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::contains_sphere
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int BoundingPlane::
contains_sphere(const BoundingSphere *sphere) const {
  nassertr(!is_empty() && !is_infinite(), 0);
  nassertr(!sphere->is_empty() && !sphere->is_infinite(), 0);

  PN_stdfloat r = sphere->get_radius();
  PN_stdfloat d = _plane.dist_to_plane(sphere->get_center());

  if (d <= -r) {
    // The sphere is completely behind the plane.
    return IF_all | IF_possible | IF_some;

  } else if (d <= r) {
    // The sphere is intersecting with the plane itself.
    return IF_possible | IF_some;

  } else {
    // The sphere is completely in front of the plane.
    return IF_no_intersection;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::contains_box
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int BoundingPlane::
contains_box(const BoundingBox *box) const {
  nassertr(!is_empty() && !is_infinite(), 0);
  nassertr(!box->is_empty() && !box->is_infinite(), 0);

  // Put the box inside a sphere for the purpose of this test.
  const LPoint3 &min = box->get_minq();
  const LPoint3 &max = box->get_maxq();
  LPoint3 center = (min + max) * 0.5f;
  PN_stdfloat radius2 = (max - center).length_squared();

  int result = IF_possible | IF_some | IF_all;

  PN_stdfloat dist = _plane.dist_to_plane(center);
  PN_stdfloat dist2 = dist * dist;

  if (dist2 <= radius2) {
    // The sphere is not completely behind this plane, but some of
    // it is.
    
    // Look a little closer.
    bool all_in = true;
    bool all_out = true;
    for (int i = 0; i < 8 && (all_in || all_out) ; ++i) {
      if (_plane.dist_to_plane(box->get_point(i)) < 0.0f) {
        // This point is inside the plane.
        all_out = false;
      } else {
        // This point is outside the plane.
        all_in = false;
      }
    }
    
    if (all_out) {
      return IF_no_intersection;
    } else if (!all_in) {
      result &= ~IF_all;
    }
    
  } else if (dist >= 0.0f) {
    // The sphere is completely in front of this plane.
    return IF_no_intersection;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::contains_line
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int BoundingPlane::
contains_line(const BoundingLine *line) const {
  return IF_possible;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::contains_plane
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int BoundingPlane::
contains_plane(const BoundingPlane *plane) const {
  return IF_possible;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingPlane::contains_hexahedron
//       Access: Protected, Virtual
//  Description: Double-dispatch support: called by contains_other()
//               when the type we're testing for intersection is known
//               to be a hexahedron.
////////////////////////////////////////////////////////////////////
int BoundingPlane::
contains_hexahedron(const BoundingHexahedron *hexahedron) const {
  nassertr(!is_empty() && !is_infinite(), 0);
  nassertr(!hexahedron->is_empty() && !hexahedron->is_infinite(), 0);

  int result = IF_possible | IF_some | IF_all;

  bool all_in = true;
  bool all_out = true;
  for (int i = 0; i < 8 && (all_in || all_out) ; ++i) {
    if (_plane.dist_to_plane(hexahedron->get_point(i)) < 0.0f) {
      // This point is inside the plane.
      all_out = false;
    } else {
      // This point is outside the plane.
      all_in = false;
    }
  }
    
  if (all_out) {
    return IF_no_intersection;
  } else if (!all_in) {
    result &= ~IF_all;
  }

  return result;
}
