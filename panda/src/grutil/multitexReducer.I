// Filename: multitexReducer.I
// Created by:  drose (30Nov04)
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
//     Function: MultitexReducer::StageInfo::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool MultitexReducer::StageInfo::
operator < (const MultitexReducer::StageInfo &other) const {
  if (_stage != other._stage) {
    return _stage < other._stage;
  }
  if (_tex != other._tex) {
    return _tex < other._tex;
  }
  if (_tex_mat != other._tex_mat) {
    return _tex_mat < other._tex_mat;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::GeomInfo::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MultitexReducer::GeomInfo::
GeomInfo(GeomNode *geom_node, int index) :
  _geom_node(geom_node),
  _index(index)
{
}
