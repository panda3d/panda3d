/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgVirtualFrame.cxx
 * @author drose
 * @date 2005-08-17
 */

#include "pgVirtualFrame.h"
#include "scissorEffect.h"
#include "sceneGraphReducer.h"

TypeHandle PGVirtualFrame::_type_handle;

/**
 *
 */
PGVirtualFrame::
PGVirtualFrame(const std::string &name) : PGItem(name)
{
  _has_clip_frame = false;
  _clip_frame.set(0.0f, 0.0f, 0.0f, 0.0f);

  setup_child_nodes();
}

/**
 *
 */
PGVirtualFrame::
~PGVirtualFrame() {
}

/**
 *
 */
PGVirtualFrame::
PGVirtualFrame(const PGVirtualFrame &copy) :
  PGItem(copy),
  _has_clip_frame(copy._has_clip_frame),
  _clip_frame(copy._clip_frame)
{
  setup_child_nodes();

  // Reassign the clip planes according to the clip frame.
  if (_has_clip_frame) {
    set_clip_frame(_clip_frame);
  } else {
    clear_clip_frame();
  }
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *PGVirtualFrame::
make_copy() const {
  LightReMutexHolder holder(_lock);
  return new PGVirtualFrame(*this);
}

/**
 * This is called by r_copy_subgraph(); the copy has already been made of this
 * particular node (and this is the copy); this function's job is to copy all
 * of the children from the original.
 *
 * Note that it includes the parameter inst_map, which is a map type, and is
 * not (and cannot be) exported from PANDA.DLL.  Thus, any derivative of
 * PandaNode that is not also a member of PANDA.DLL *cannot* access this map,
 * and probably should not even override this function.
 */
void PGVirtualFrame::
r_copy_children(const PandaNode *from, PandaNode::InstanceMap &inst_map,
                Thread *current_thread) {
  LightReMutexHolder holder(_lock);
  PandaNode::r_copy_children(from, inst_map, current_thread);

  // Reassign the canvas_node to point to the new copy, if it's there.
  const PGVirtualFrame *from_frame = DCAST(PGVirtualFrame, from);
  PandaNode *from_canvas_node = from_frame->get_canvas_node();
  PandaNode *from_canvas_parent = from_frame->get_canvas_parent();

  InstanceMap::const_iterator ci;
  ci = inst_map.find(from_canvas_node);
  if (ci != inst_map.end()) {
    remove_child(_canvas_node);
    _canvas_node = DCAST(ModelNode, (*ci).second);
  }

  ci = inst_map.find(from_canvas_parent);
  if (ci != inst_map.end()) {
    remove_child(_canvas_parent);
    _canvas_parent = DCAST(ModelNode, (*ci).second);
  }

  // Reassign the clip planes according to the clip frame.
  if (_has_clip_frame) {
    set_clip_frame(_clip_frame);
  } else {
    clear_clip_frame();
  }
}

/**
 * Creates a PGVirtualFrame with the indicated dimensions.
 */
void PGVirtualFrame::
setup(PN_stdfloat width, PN_stdfloat height) {
  LightReMutexHolder holder(_lock);
  set_state(0);
  clear_state_def(0);

  set_frame(0, width, 0, height);

  PN_stdfloat bevel = 0.05f;

  PGFrameStyle style;
  style.set_width(bevel, bevel);

  style.set_color(0.8f, 0.8f, 0.8f, 1.0f);
  style.set_type(PGFrameStyle::T_bevel_out);
  set_frame_style(0, style);

  set_clip_frame(bevel, width - 2 * bevel,
                 bevel, height - 2 * bevel);
}

/**
 * Sets the bounding rectangle of the clip frame.  This is the size of the
 * small window through which we can see the virtual canvas.  Normally, this
 * is the same size as the actual frame or smaller (typically it is smaller by
 * the size of the bevel, or to make room for scroll bars).
 */
void PGVirtualFrame::
set_clip_frame(const LVecBase4 &frame) {
  LightReMutexHolder holder(_lock);
  if (!_has_clip_frame || _clip_frame != frame) {
    _has_clip_frame = true;
    _clip_frame = frame;

    CPT(RenderEffect) scissor_effect = ScissorEffect::make_node
      (LPoint3(_clip_frame[0], _clip_frame[2], _clip_frame[2]),
       LPoint3(_clip_frame[1], _clip_frame[2], _clip_frame[2]),
       LPoint3(_clip_frame[1], _clip_frame[3], _clip_frame[3]),
       LPoint3(_clip_frame[0], _clip_frame[3], _clip_frame[3]));

    _canvas_parent->set_effect(scissor_effect);
    clip_frame_changed();
  }
}

/**
 * Removes the clip frame from the item.  This disables clipping.
 */
void PGVirtualFrame::
clear_clip_frame() {
  LightReMutexHolder holder(_lock);
  if (_has_clip_frame) {
    _has_clip_frame = false;

    _canvas_parent->clear_effect(ScissorEffect::get_class_type());
    clip_frame_changed();
  }
}

/**
 * Called when the user changes the clip_frame size.
 */
void PGVirtualFrame::
clip_frame_changed() {
}

/**
 * Creates the special canvas_node and canvas_parent for this object.
 */
void PGVirtualFrame::
setup_child_nodes() {
  _canvas_parent = new ModelNode("canvas_parent");
  _canvas_parent->set_preserve_transform(ModelNode::PT_local);
  add_child(_canvas_parent);

  _canvas_node = new ModelNode("canvas");
  _canvas_node->set_preserve_transform(ModelNode::PT_local);
  _canvas_parent->add_child(_canvas_node);
}
