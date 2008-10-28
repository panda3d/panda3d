// Filename: textGraphic.h
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

#ifndef TEXTGRAPHIC_H
#define TEXTGRAPHIC_H

#include "pandabase.h"

#include "config_text.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : TextGraphic
// Description : This defines a special model that has been
//               constructed for the purposes of embedding an
//               arbitrary graphic image within a text paragraph.
//
//               It can be any arbitrary model, though it should be
//               built along the same scale as the text, and it should
//               probably be at least mostly two-dimensional.
//               Typically, this means it should be constructed in the
//               X-Z plane, and it should have a maximum vertical (Z)
//               height of 1.0.
//
//               The frame specifies an arbitrary bounding volume in
//               the form (left, right, bottom, top).  This indicates
//               the amount of space that will be reserved within the
//               paragraph.  The actual model is not actually required
//               to fit within this rectangle, but if it does not, it
//               may visually overlap with nearby text.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_TEXT TextGraphic {
PUBLISHED:
  INLINE TextGraphic();
  INLINE TextGraphic(const NodePath &model, const LVecBase4f &frame);
  INLINE TextGraphic(const NodePath &model, float left, float right, float bottom, float top);

  INLINE NodePath get_model() const;
  INLINE void set_model(const NodePath &model);

  INLINE LVecBase4f get_frame() const;
  INLINE void set_frame(const LVecBase4f &frame);
  INLINE void set_frame(float left, float right, float bottom, float top);

  INLINE bool get_instance_flag() const;
  INLINE void set_instance_flag(bool instance_flag);

private:
  NodePath _model;
  LVecBase4f _frame;
  bool _instance_flag;
};

#include "textGraphic.I"

#endif
