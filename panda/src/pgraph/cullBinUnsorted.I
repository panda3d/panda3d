// Filename: cullBinUnsorted.I
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
//     Function: CullBinUnsorted::GeomData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinUnsorted::GeomData::
GeomData(Geom *geom, const TransformState *transform,
         const RenderState *state) :
  _geom(geom),
  _transform(transform),
  _state(state)
{
}


////////////////////////////////////////////////////////////////////
//     Function: CullBinUnsorted::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinUnsorted::
CullBinUnsorted(GraphicsStateGuardianBase *gsg) :
  CullBin(gsg)
{
}
