// Filename: cullBinFixed.I
// Created by:  drose (29May02)
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


////////////////////////////////////////////////////////////////////
//     Function: CullBinFixed::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinFixed::
CullBinFixed(const string &name, GraphicsStateGuardianBase *gsg,
             const PStatCollector &draw_region_pcollector) :
  CullBin(name, BT_fixed, gsg, draw_region_pcollector)
{
}

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

