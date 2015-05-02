// Filename: odeQuadTreeSpace.cxx
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

#include "config_ode.h"
#include "odeQuadTreeSpace.h"

TypeHandle OdeQuadTreeSpace::_type_handle;

typedef struct { dVector4 vec; } sdVector4;

sdVector4 LVec3_to_sdVector4(const LVecBase3f& vec) {
  sdVector4 sdVec4;

  sdVec4.vec[0] = vec[0];
  sdVec4.vec[1] = vec[1];
  sdVec4.vec[2] = vec[2];
  sdVec4.vec[3] = 0;

  return sdVec4;
}

OdeQuadTreeSpace::
OdeQuadTreeSpace(dSpaceID id) :
  OdeSpace(id) {
}

OdeQuadTreeSpace::
OdeQuadTreeSpace(const LPoint3f &center,
                 const LVecBase3f &extents,
                 const int depth) :
  OdeSpace(dQuadTreeSpaceCreate(0,
                                LVec3_to_sdVector4(center).vec,
                                LVec3_to_sdVector4(extents).vec,
                                depth)) {
}

OdeQuadTreeSpace::
OdeQuadTreeSpace(OdeSpace &space,
                 const LPoint3f &center,
                 const LVecBase3f &extents,
                 const int depth) :
  OdeSpace(dQuadTreeSpaceCreate(space.get_id(),
                                LVec3_to_sdVector4(center).vec,
                                LVec3_to_sdVector4(extents).vec,
                                depth)) {
}

OdeQuadTreeSpace::
~OdeQuadTreeSpace() {
}

