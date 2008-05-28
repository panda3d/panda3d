// Filename: odeHashSpace.I
// Created by:  joswilso (27Dec06)
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

INLINE void OdeHashSpace::
set_levels(int minlevel, int maxlevel) {
  dHashSpaceSetLevels(_id, minlevel, maxlevel);
}

INLINE int OdeHashSpace::
get_min_level() const {
  int min, max;
  dHashSpaceGetLevels(_id, &min, &max);
  return min;
}

INLINE int OdeHashSpace::
get_max_level() const {
  int min, max;
  dHashSpaceGetLevels(_id, &min, &max);
  return max;
}
