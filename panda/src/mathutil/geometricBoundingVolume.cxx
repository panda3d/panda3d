// Filename: geometricBoundingVolume.cxx
// Created by:  drose (07Oct99)
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

#include "geometricBoundingVolume.h"

TypeHandle GeometricBoundingVolume::_type_handle;


bool GeometricBoundingVolume::
extend_by_point(const LPoint3f &) {
  return false;
}

bool GeometricBoundingVolume::
around_points(const LPoint3f *, const LPoint3f *) {
  _flags = F_empty;
  return false;
}

int GeometricBoundingVolume::
contains_point(const LPoint3f &) const {
  return IF_dont_understand;
}

int GeometricBoundingVolume::
contains_lineseg(const LPoint3f &, const LPoint3f &) const {
  return IF_dont_understand;
}
