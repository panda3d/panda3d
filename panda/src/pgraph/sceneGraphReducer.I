// Filename: sceneGraphReducer.I
// Created by:  drose (14Mar02)
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


////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::AccumulatedAttribs::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SceneGraphReducer::AccumulatedAttribs::
AccumulatedAttribs() {
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::AccumulatedAttribs::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SceneGraphReducer::AccumulatedAttribs::
AccumulatedAttribs(const SceneGraphReducer::AccumulatedAttribs &copy) :
  _transform(copy._transform),
  _color(copy._color),
  _color_scale(copy._color_scale),
  _tex_matrix(copy._tex_matrix),
  _other(copy._other)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::AccumulatedAttribs::Copy Assignment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void SceneGraphReducer::AccumulatedAttribs::
operator = (const SceneGraphReducer::AccumulatedAttribs &copy) {
  _transform = copy._transform;
  _color = copy._color;
  _color_scale = copy._color_scale;
  _tex_matrix = copy._tex_matrix;
  _other = copy._other;
}
