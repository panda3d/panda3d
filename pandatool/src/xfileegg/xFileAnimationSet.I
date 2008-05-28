// Filename: xFileAnimationSet.I
// Created by:  drose (04Oct04)
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
//     Function: XFileAnimationSet::FrameEntry::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE XFileAnimationSet::FrameEntry::
FrameEntry() :
  _scale(1.0, 1.0, 1.0),
  _rot(LQuaterniond::ident_quat()),
  _trans(LVector3d::zero()),
  _mat(LMatrix4d::ident_mat())
{
}

////////////////////////////////////////////////////////////////////
//     Function: XFileAnimationSet::FrameEntry::get_mat
//       Access: Public
//  Description: Returns the frame's data as a matrix, composing the
//               matrix first if necessary, as indicated by the
//               FrameData's _flags member.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4d &XFileAnimationSet::FrameEntry::
get_mat(int flags) const {
  if ((flags & FDF_mat) == 0) {
    ((FrameEntry *)this)->_mat = LMatrix4d::scale_mat(_scale) * _rot;
    ((FrameEntry *)this)->_mat.set_row(3, _trans);
  }
  return _mat;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileAnimationSet::FrameData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE XFileAnimationSet::FrameData::
FrameData() :
  _flags(0)
{
}
