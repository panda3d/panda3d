// Filename: stTransform.I
// Created by:  drose (06Oct10)
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
//     Function: STTransform::Default Constructor
//       Access: Published
//  Description: The default constructor creates an identity transform.
////////////////////////////////////////////////////////////////////
INLINE STTransform::
STTransform() :
  _pos(0.0f, 0.0f, 0.0f),
  _rotate(0.0f),
  _scale(1.0f)
{
}

////////////////////////////////////////////////////////////////////
//     Function: STTransform::Constructor
//       Access: Published
//  Description: Construct a transform with componentwise inputs.
////////////////////////////////////////////////////////////////////
INLINE STTransform::
STTransform(const LPoint3f &pos, float rotate, float scale) :
  _pos(pos),
  _rotate(rotate),
  _scale(scale)
{
}

////////////////////////////////////////////////////////////////////
//     Function: STTransform::Constructor
//       Access: Published
//  Description: Construct a transform with componentwise inputs.
////////////////////////////////////////////////////////////////////
INLINE STTransform::
STTransform(float x, float y, float z, float rotate, float scale) :
  _pos(x, y, z),
  _rotate(rotate),
  _scale(scale)
{
}

////////////////////////////////////////////////////////////////////
//     Function: STTransform::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE STTransform::
STTransform(const STTransform &copy) :
  _pos(copy._pos),
  _rotate(copy._rotate),
  _scale(copy._scale)
{
}

////////////////////////////////////////////////////////////////////
//     Function: STTransform::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void STTransform::
operator = (const STTransform &copy) {
  _pos = copy._pos;
  _rotate = copy._rotate;
  _scale = copy._scale;
}

////////////////////////////////////////////////////////////////////
//     Function: STTransform::CInstance constructor
//       Access: Public
//  Description: This is used internally to construct an STTransform
//               from a SpeedTree::CInstance object.
////////////////////////////////////////////////////////////////////
INLINE STTransform::
STTransform(const SpeedTree::CInstance &instance) {
  const SpeedTree::Vec3 &pos = instance.GetPos();
  _pos.set(pos[0], pos[1], pos[2]);
  _rotate = rad_2_deg(instance.GetRotationAngle());
  _scale = instance.GetScale();
}

////////////////////////////////////////////////////////////////////
//     Function: STTransform::CInstance operator
//       Access: Public
//  Description: This is used internally to convert an STTransform
//               into a SpeedTree::CInstance object.
////////////////////////////////////////////////////////////////////
INLINE STTransform::
operator SpeedTree::CInstance () const {
  SpeedTree::CInstance instance;
  instance.SetPos(SpeedTree::Vec3(_pos[0], _pos[1], _pos[2]));
  instance.SetRotation(deg_2_rad(_rotate));
  instance.SetScale(_scale);
  return instance;
}

////////////////////////////////////////////////////////////////////
//     Function: STTransform::TransformState operator
//       Access: Public
//  Description: This is used internally to convert an STTransform
//               into a TransformState pointer.
////////////////////////////////////////////////////////////////////
INLINE STTransform::
operator CPT(TransformState) () const {
  return TransformState::make_pos_hpr_scale(_pos, 
					    LVecBase3f(_rotate, 0.0f, 0.0f),
					    LVecBase3f(_scale, _scale, _scale));
}

////////////////////////////////////////////////////////////////////
//     Function: STTransform::ident_mat
//       Access: Published, Static
//  Description: Returns a global identity transform object.
////////////////////////////////////////////////////////////////////
INLINE const STTransform &STTransform::
ident_mat() {
  return _ident_mat;
}
