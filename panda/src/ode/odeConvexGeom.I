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

INLINE void OdeGeom::
set_convex(dReal *_planes, unsigned int _count, dReal *_points, unsigned int _pointcount, unsigned int *_polygons) {
  dGeomSetConvex(_id, *_planes, _count, *_points, _pointcount, *_polygons);
}

