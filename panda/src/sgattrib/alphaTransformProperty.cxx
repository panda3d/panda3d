// Filename: alphaTransformProperty.cxx
// Created by:  jason (01Aug00)
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

#include "alphaTransformProperty.h"

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformProperty::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int AlphaTransformProperty::
compare_to(const AlphaTransformProperty &other) const {
  if (_offset < other._offset)
    return -1;
  else if (_offset > other._offset)
    return 1;
  if (_scale < other._scale)
    return -1;
  else if (_scale > other._scale)
    return 1;
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTransformProperty::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void AlphaTransformProperty::
output(ostream &out) const {
  out << "Offset " << _offset << " Scale " << _scale << endl;
}
