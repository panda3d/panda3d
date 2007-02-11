// Filename: odeQuadTreeSpace.cxx
// Created by:  joswilso (27Dec06)
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

