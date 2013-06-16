// Filename: bulletSliderConstraint.I
// Created by:  enn0x (01Mar10)
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
//     Function: BulletSliderConstraint::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BulletSliderConstraint::
~BulletSliderConstraint() {

  delete _constraint;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_frame_a
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) BulletSliderConstraint::
get_frame_a() const {

  return btTrans_to_TransformState(_constraint->getFrameOffsetA());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletSliderConstraint::get_frame_b
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) BulletSliderConstraint::
get_frame_b() const {

  return btTrans_to_TransformState(_constraint->getFrameOffsetB());
}

