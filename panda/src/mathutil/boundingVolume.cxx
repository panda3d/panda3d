// Filename: boundingVolume.cxx
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

#include "boundingVolume.h"
#include "finiteBoundingVolume.h"

#include "indent.h"

TypeHandle BoundingVolume::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: BoundingVolume::around
//       Access: Public
//  Description: Resets the volume to enclose only the volumes
//               indicated.  Returns true if successful, false if the
//               volume doesn't know how to do that or can't do that.
////////////////////////////////////////////////////////////////////
bool BoundingVolume::
around(const BoundingVolume **first, const BoundingVolume **last) {
  _flags = F_empty;

  // Skip any empty volumes at the beginning of the list.  We want to
  // get to the first real volume.
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

    // This is a double-dispatch.  We call this virtual function on
    // the volume we were given, which will in turn call the
    // appropriate virtual function in our own class to perform the
    // operation.
    if (!(*first)->around_other(this, first, last)) {
      okflag = false;
    }
  }

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingVolume::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BoundingVolume::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingVolume::extend_by_sphere
//       Access: Protected, Virtual
//  Description: Double-dispatch support: called by extend_other()
//               when the type we're extending by is known to be a
//               sphere.
////////////////////////////////////////////////////////////////////
bool BoundingVolume::
extend_by_sphere(const BoundingSphere *) {
  _flags = F_infinite;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingVolume::extend_by_hexahedron
//       Access: Protected, Virtual
//  Description: Double-dispatch support: called by extend_other()
//               when the type we're extending by is known to be a
//               hexahedron.
////////////////////////////////////////////////////////////////////
bool BoundingVolume::
extend_by_hexahedron(const BoundingHexahedron *) {
  _flags = F_infinite;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingVolume::extend_by_line
//       Access: Protected, Virtual
//  Description: Double-dispatch support: called by extend_other()
//               when the type we're extending by is known to be a
//               line.
////////////////////////////////////////////////////////////////////
bool BoundingVolume::
extend_by_line(const BoundingLine *) {
  _flags = F_infinite;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingVolume::around_spheres
//       Access: Protected, Virtual
//  Description: Double-dispatch support: called by around_other()
//               when the type of the first element in the list is
//               known to be a nonempty sphere.
////////////////////////////////////////////////////////////////////
bool BoundingVolume::
around_spheres(const BoundingVolume **, const BoundingVolume **) {
  _flags = F_infinite;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingVolume::around_hexahedrons
//       Access: Protected, Virtual
//  Description: Double-dispatch support: called by around_other()
//               when the type of the first element in the list is
//               known to be a nonempty hexahedron.
////////////////////////////////////////////////////////////////////
bool BoundingVolume::
around_hexahedrons(const BoundingVolume **, const BoundingVolume **) {
  _flags = F_infinite;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingVolume::around_lines
//       Access: Protected, Virtual
//  Description: Double-dispatch support: called by around_other()
//               when the type of the first element in the list is
//               known to be a nonempty line.
////////////////////////////////////////////////////////////////////
bool BoundingVolume::
around_lines(const BoundingVolume **, const BoundingVolume **) {
  _flags = F_infinite;
  if (is_of_type(FiniteBoundingVolume::get_class_type())) {
    // If it's a FiniteBoundingVolume, we can't do any better than
    // making it infinite.  So we return true.
    return true;
  }

  // Otherwise, we might do better, and we require each class to
  // define a function.  If we get here, the function isn't defined,
  // so we return false to indicate this.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingVolume::contains_sphere
//       Access: Protected, Virtual
//  Description: Double-dispatch support: called by contains_other()
//               when the type we're testing for intersection is known
//               to be a sphere.
////////////////////////////////////////////////////////////////////
int BoundingVolume::
contains_sphere(const BoundingSphere *) const {
  return IF_dont_understand;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingVolume::contains_hexahedron
//       Access: Protected, Virtual
//  Description: Double-dispatch support: called by contains_other()
//               when the type we're testing for intersection is known
//               to be a hexahedron.
////////////////////////////////////////////////////////////////////
int BoundingVolume::
contains_hexahedron(const BoundingHexahedron *) const {
  return IF_dont_understand;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundingVolume::contains_line
//       Access: Protected, Virtual
//  Description: Double-dispatch support: called by contains_other()
//               when the type we're testing for intersection is known
//               to be a line.
////////////////////////////////////////////////////////////////////
int BoundingVolume::
contains_line(const BoundingLine *) const {
  return IF_dont_understand;
}
