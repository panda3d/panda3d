// Filename: cullBinFrontToBack.I
// Created by:  drose (29May02)
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
//     Function: CullBinFrontToBack::ObjectData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinFrontToBack::ObjectData::
ObjectData(CullableObject *object, float dist) :
  _object(object),
  _dist(dist)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinFrontToBack::ObjectData::operator <
//       Access: Public
//  Description: Specifies the correct sort ordering for these
//               objects.
////////////////////////////////////////////////////////////////////
INLINE bool CullBinFrontToBack::ObjectData::
operator < (const ObjectData &other) const {
  return _dist < other._dist;
}


////////////////////////////////////////////////////////////////////
//     Function: CullBinFrontToBack::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinFrontToBack::
CullBinFrontToBack(const string &name, GraphicsStateGuardianBase *gsg) :
  CullBin(name, gsg)
{
}
