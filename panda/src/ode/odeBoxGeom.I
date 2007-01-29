// Filename: odeBoxGeom.I
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

INLINE void OdeBoxGeom::
set_lengths(dReal lx, dReal ly, dReal lz) {
  dGeomBoxSetLengths(_id, lx, ly, lz);
}

INLINE LVecBase3f OdeBoxGeom::
get_lengths() {
  dVector3 res;
  dGeomBoxGetLengths(_id, res);
  return LVecBase3f(res[0], res[1], res[2]);
}

INLINE dReal OdeBoxGeom::
get_point_depth(dReal x, dReal y, dReal z) {
  return dGeomBoxPointDepth(_id, x, y, z);
}

