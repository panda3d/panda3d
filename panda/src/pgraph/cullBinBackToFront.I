// Filename: cullBinBackToFront.I
// Created by:  drose (28Feb02)
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


////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::ObjectData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinBackToFront::ObjectData::
ObjectData(CullableObject *object, float dist) :
  _object(object),
  _dist(dist)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::ObjectData::operator <
//       Access: Public
//  Description: Specifies the correct sort ordering for these
//               objects.
////////////////////////////////////////////////////////////////////
INLINE bool CullBinBackToFront::ObjectData::
operator < (const ObjectData &other) const {
  return _dist > other._dist;
}


////////////////////////////////////////////////////////////////////
//     Function: CullBinBackToFront::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinBackToFront::
CullBinBackToFront(const string &name, GraphicsStateGuardianBase *gsg) :
  CullBin(name, gsg)
{
}
