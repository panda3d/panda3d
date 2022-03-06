/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textGraphic.h
 * @author drose
 * @date 2006-08-18
 */

#ifndef TEXTGRAPHIC_H
#define TEXTGRAPHIC_H

#include "pandabase.h"

#include "config_text.h"
#include "memoryBase.h"
#include "nodePath.h"

/**
 * This defines a special model that has been constructed for the purposes of
 * embedding an arbitrary graphic image within a text paragraph.
 *
 * It can be any arbitrary model, though it should be built along the same
 * scale as the text, and it should probably be at least mostly two-
 * dimensional.  Typically, this means it should be constructed in the X-Z
 * plane, and it should have a maximum vertical (Z) height of 1.0.
 *
 * The frame specifies an arbitrary bounding volume in the form (left, right,
 * bottom, top).  This indicates the amount of space that will be reserved
 * within the paragraph.  The actual model is not actually required to fit
 * within this rectangle, but if it does not, it may visually overlap with
 * nearby text.
 */
class EXPCL_PANDA_TEXT TextGraphic : public MemoryBase {
PUBLISHED:
  INLINE TextGraphic();
  INLINE explicit TextGraphic(const NodePath &model, const LVecBase4 &frame);
  INLINE explicit TextGraphic(const NodePath &model, PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top);

  INLINE NodePath get_model() const;
  INLINE void set_model(const NodePath &model);
  MAKE_PROPERTY(model, get_model, set_model);

  INLINE LVecBase4 get_frame() const;
  INLINE void set_frame(const LVecBase4 &frame);
  INLINE void set_frame(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top);
  MAKE_PROPERTY(frame, get_frame, set_frame);

  INLINE bool get_instance_flag() const;
  INLINE void set_instance_flag(bool instance_flag);
  MAKE_PROPERTY(instance_flag, get_instance_flag, set_instance_flag);

private:
  NodePath _model;
  LVecBase4 _frame;
  bool _instance_flag;
};

#include "textGraphic.I"

#endif
