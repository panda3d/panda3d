// Filename: sceneGraphReducer.I
// Created by:  drose (22May00)
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
//     Function: SceneGraphReducer::AccumulatedTransitions::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SceneGraphReducer::AccumulatedTransitions::
AccumulatedTransitions() {
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::AccumulatedTransitions::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SceneGraphReducer::AccumulatedTransitions::
AccumulatedTransitions(const SceneGraphReducer::AccumulatedTransitions &copy) :
  _transform(copy._transform),
  _color(copy._color),
  _texture_matrix(copy._texture_matrix)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::AccumulatedTransitions::Copy Assignment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void SceneGraphReducer::AccumulatedTransitions::
operator = (const SceneGraphReducer::AccumulatedTransitions &copy) {
  _transform = copy._transform;
  _color = copy._color;
  _texture_matrix = copy._texture_matrix;
}
