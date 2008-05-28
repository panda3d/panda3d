// Filename: cLwoPolygons.I
// Created by:  drose (25Apr01)
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
//     Function: CLwoPolygons::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CLwoPolygons::
CLwoPolygons(LwoToEggConverter *converter, const LwoPolygons *polygons,
             CLwoPoints *points) :
  _converter(converter),
  _polygons(polygons),
  _points(points)
{
  _tags = (LwoTags *)NULL;
  _surf_ptags = (LwoPolygonTags *)NULL;
}
