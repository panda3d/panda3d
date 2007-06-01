// Filename: animChannelMatrixFixed.cxx
// Created by:  drose (19Jan06)
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

#include "animChannelMatrixFixed.h"
#include "compose_matrix.h"

TypeHandle AnimChannelMatrixFixed::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::Copy Constructor
//       Access: Protected
//  Description: Creates a new AnimChannelMatrixFixed, just like this
//               one, without copying any children.  The new copy is
//               added to the indicated parent.  Intended to be called
//               by make_copy() only.
////////////////////////////////////////////////////////////////////
AnimChannelMatrixFixed::
AnimChannelMatrixFixed(AnimGroup *parent, const AnimChannelMatrixFixed &copy) : 
  AnimChannelFixed<ACMatrixSwitchType>(parent, copy),
  _value_no_scale_shear(copy._value_no_scale_shear),
  _scale(copy._scale),
  _hpr(copy._hpr),
  _quat(copy._quat),
  _pos(copy._pos),
  _shear(copy._shear)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::Constructor
//       Access: Public
//  Description: This flavor creates an AnimChannelMatrixFixed that
//               *is* in a hierarchy.
////////////////////////////////////////////////////////////////////
AnimChannelMatrixFixed::
AnimChannelMatrixFixed(AnimGroup *parent, const string &name, 
                       const LMatrix4f &value)
  : AnimChannelFixed<ACMatrixSwitchType>(parent, name, value)
{
  // Decompose the matrix into components in case we will be blending.
  decompose_matrix(_value, _scale, _shear, _hpr, _pos);
  compose_matrix(_value_no_scale_shear, LVecBase3f(1.0f, 1.0f, 1.0f),
                 _hpr, _pos);

  _quat.set_hpr(_hpr);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::Constructor
//       Access: Public
//  Description: This flavor creates an AnimChannelMatrixFixed that is not
//               in a hierarchy.
////////////////////////////////////////////////////////////////////
AnimChannelMatrixFixed::
AnimChannelMatrixFixed(const string &name, const LMatrix4f &value)
  : AnimChannelFixed<ACMatrixSwitchType>(name, value)
{
  // Decompose the matrix into components in case we will be blending.
  decompose_matrix(_value, _scale, _shear, _hpr, _pos);
  compose_matrix(_value_no_scale_shear, LVecBase3f(1.0f, 1.0f, 1.0f),
                 _hpr, _pos);

  _quat.set_hpr(_hpr);
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::get_value_no_scale_shear
//       Access: Public, Virtual
//  Description: Gets the value of the channel at the indicated frame,
//               without any scale or shear information.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_value_no_scale_shear(int frame, LMatrix4f &mat) {
  mat = _value_no_scale_shear;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::get_scale
//       Access: Public, Virtual
//  Description: Gets the scale value at the indicated frame.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_scale(int, LVecBase3f &scale) {
  scale = _scale;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::get_hpr
//       Access: Public, Virtual
//  Description: Returns the h, p, and r components associated
//               with the current frame.  As above, this only makes
//               sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_hpr(int, LVecBase3f &hpr) {
  hpr = _hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::get_quat
//       Access: Public, Virtual
//  Description: Returns the rotation component associated with the
//               current frame, expressed as a quaternion.  As above,
//               this only makes sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_quat(int, LQuaternionf &quat) {
  quat = _quat;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::get_pos
//       Access: Public, Virtual
//  Description: Returns the x, y, and z translation components
//               associated with the current frame.  As above, this
//               only makes sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_pos(int, LVecBase3f &pos) {
  pos = _pos;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixFixed::get_shear
//       Access: Public, Virtual
//  Description: Returns the a, b, and c shear components associated
//               with the current frame.  As above, this only makes
//               sense for a matrix-type channel.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixFixed::
get_shear(int, LVecBase3f &shear) {
  shear = _shear;
}
