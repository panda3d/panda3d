// Filename: geometricBoundingVolume.cxx
// Created by:  drose (07Oct99)
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
