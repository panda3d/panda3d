// Filename: cullBinBackToFront.I
// Created by:  drose (28Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::GeomData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinBackToFront::GeomData::
GeomData(Geom *geom, const TransformState *transform,
         const RenderState *state, float dist) :
  _geom(geom),
  _transform(transform),
  _state(state),
  _dist(dist)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::GeomData::operator <
//       Access: Public
//  Description: Specifies the correct sort ordering for these
//               objects.
////////////////////////////////////////////////////////////////////
INLINE bool CullBinBackToFront::GeomData::
operator < (const GeomData &other) const {
  return _dist > other._dist;
}


////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinBackToFront::
CullBinBackToFront(GraphicsStateGuardianBase *gsg) :
  CullBin(gsg)
{
}
