// Filename: cLwoPolygons.I
// Created by:  drose (25Apr01)
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
