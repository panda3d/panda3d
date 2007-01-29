// Filename: odeSphereGeom.I
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

INLINE void OdeSphereGeom::
set_radius(dReal radius) {
  dGeomSphereSetRadius(_id, radius);
}

INLINE dReal OdeSphereGeom::
get_radius() const {
  return dGeomSphereGetRadius(_id);
}

INLINE dReal OdeSphereGeom::
get_point_depth(dReal x, dReal y, dReal z) const {
  return dGeomSpherePointDepth(_id, x, y, z);
}

