// Filename: eggGroupNode.I
// Created by:  drose (16Jan99)
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
//     Function: EggGroupNode::TBNVertexValue::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool EggGroupNode::TBNVertexValue::
operator < (const TBNVertexValue &other) const {
  int compare = _pos.compare_to(other._pos);
  if (compare != 0) {
    return compare < 0;
  }
  compare = _normal.compare_to(other._normal);
  if (compare != 0) {
    return compare < 0;
  }
  compare = _uv.compare_to(other._uv);
  if (compare != 0) {
    return compare < 0;
  }
  return _uv_name < other._uv_name;
}
