// Filename: cullBinFixed.I
// Created by:  drose (29May02)
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
//     Function: CullBinFixed::ObjectData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinFixed::ObjectData::
ObjectData(CullableObject *object, int draw_order) :
  _object(object),
  _draw_order(draw_order)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinFixed::ObjectData::operator <
//       Access: Public
//  Description: Specifies the correct sort ordering for these
//               objects.
////////////////////////////////////////////////////////////////////
INLINE bool CullBinFixed::ObjectData::
operator < (const ObjectData &other) const {
  return _draw_order < other._draw_order;
}


////////////////////////////////////////////////////////////////////
//     Function: CullBinFixed::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinFixed::
CullBinFixed(const string &name, GraphicsStateGuardianBase *gsg) :
  CullBin(name, gsg)
{
}
