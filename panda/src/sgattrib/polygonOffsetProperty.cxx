// Filename: polygonOffsetProperty.cxx
// Created by:  jason (12Jul00)
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

#include "polygonOffsetProperty.h"

////////////////////////////////////////////////////////////////////
//     Function: PolygonOffsetProperty::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PolygonOffsetProperty::
output(ostream &out) const {
  out << "Units " << _units << " Factor " << _factor << endl;
}
