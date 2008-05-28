// Filename: odeBoxGeom.I
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

INLINE void OdeGeom::
set_convex(dReal *_planes, unsigned int _count, dReal *_points, unsigned int _pointcount, unsigned int *_polygons) {
  dGeomSetConvex(_id, *_planes, _count, *_points, _pointcount, *_polygons);
}

