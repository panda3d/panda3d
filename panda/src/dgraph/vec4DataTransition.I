// Filename: vec4DataTransition.I
// Created by:  jason (03Aug00)
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
//     Function: Vec4DataTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Vec4DataTransition::
Vec4DataTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: Vec4DataTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Vec4DataTransition::
Vec4DataTransition(const LMatrix4f &matrix) :
  VectorDataTransition<LPoint4f, LMatrix4f>(matrix)
{
}
