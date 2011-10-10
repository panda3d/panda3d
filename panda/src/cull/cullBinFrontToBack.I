// Filename: cullBinFrontToBack.I
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
//     Function: CullBinFrontToBack::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinFrontToBack::
CullBinFrontToBack(const string &name, GraphicsStateGuardianBase *gsg,
                   const PStatCollector &draw_region_pcollector) :
  CullBin(name, BT_front_to_back, gsg, draw_region_pcollector)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinFrontToBack::ObjectData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinFrontToBack::ObjectData::
ObjectData(CullableObject *object, PN_stdfloat dist) :
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

