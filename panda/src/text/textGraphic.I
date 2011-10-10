// Filename: textGraphic.I
// Created by:  drose (18Aug06)
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
//     Function: TextGraphic::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextGraphic::
TextGraphic() {
  _frame = LVecBase4::zero();
  _instance_flag = false;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGraphic::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextGraphic::
TextGraphic(const NodePath &model, const LVecBase4 &frame) :
  _model(model),
  _frame(frame),
  _instance_flag(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextGraphic::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextGraphic::
TextGraphic(const NodePath &model, PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) :
  _model(model),
  _frame(left, right, bottom, top),
  _instance_flag(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextGraphic::get_model
//       Access: Published
//  Description: Returns the NodePath associated with the graphic,
//               that renders the desired image.
////////////////////////////////////////////////////////////////////
INLINE NodePath TextGraphic::
get_model() const {
  return _model;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGraphic::set_model
//       Access: Published
//  Description: Changes the NodePath associated with the graphic.
//               This NodePath should contain geometry that will
//               render the desired graphic image.
////////////////////////////////////////////////////////////////////
INLINE void TextGraphic::
set_model(const NodePath &model) {
  _model = model;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGraphic::get_frame
//       Access: Published
//  Description: Returns the frame specified for the graphic.  This is
//               the amount of space that will be reserved for the
//               graphic when it is embedded in a text paragraph, in
//               the form (left, right, bottom, top).
//
//               The actual graphic, as rendered by the NodePath
//               specified via set_model(), should more or less fit
//               within this rectangle.  It is not required to fit
//               completely within it, but if it does not, it may
//               visually overlap with nearby text.
////////////////////////////////////////////////////////////////////
INLINE LVecBase4 TextGraphic::
get_frame() const {
  return _frame;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGraphic::set_frame
//       Access: Published
//  Description: Specifies the (left, right, bottom, top) bounding
//               frame for the graphic.  See get_frame().
////////////////////////////////////////////////////////////////////
INLINE void TextGraphic::
set_frame(const LVecBase4 &frame) {
  _frame = frame;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGraphic::set_frame
//       Access: Published
//  Description: Specifies the (left, right, bottom, top) bounding
//               frame for the graphic.  See get_frame().
////////////////////////////////////////////////////////////////////
INLINE void TextGraphic::
set_frame(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top) {
  _frame.set(left, right, bottom, top);
}

////////////////////////////////////////////////////////////////////
//     Function: TextGraphic::get_instance_flag
//       Access: Published
//  Description: Returns the instance_flag.  See set_instance_flag().
////////////////////////////////////////////////////////////////////
INLINE bool TextGraphic::
get_instance_flag() const {
  return _instance_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: TextGraphic::set_instance_flag
//       Access: Published
//  Description: Sets the instance_flag.  When this is true, the
//               graphic is directly instanced to the scene graph
//               whenever it appears; when it is false, the graphic is
//               copied.  The default is false, which is best for most
//               applications.  You might need to set it true for
//               special kinds of "graphics" like interactive
//               elements, for instance a PGEntry.
////////////////////////////////////////////////////////////////////
INLINE void TextGraphic::
set_instance_flag(bool instance_flag) {
  _instance_flag = instance_flag;
}
