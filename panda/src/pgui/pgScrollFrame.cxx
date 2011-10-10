// Filename: pgScrollFrame.cxx
// Created by:  drose (17Aug05)
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

#include "pgScrollFrame.h"

TypeHandle PGScrollFrame::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PGScrollFrame::
PGScrollFrame(const string &name) : PGVirtualFrame(name)
{
  set_cull_callback();

  _needs_remanage = false;
  _needs_recompute_canvas = false;
  _needs_recompute_clip = false;
  _has_virtual_frame = false;
  _virtual_frame.set(0.0f, 0.0f, 0.0f, 0.0f);
  _manage_pieces = false;
  _auto_hide = false;
  _horizontal_slider = NULL;
  _vertical_slider = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGScrollFrame::
~PGScrollFrame() {
  set_horizontal_slider(NULL);
  set_vertical_slider(NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::Copy Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
PGScrollFrame::
PGScrollFrame(const PGScrollFrame &copy) :
  PGVirtualFrame(copy),
  _has_virtual_frame(copy._has_virtual_frame),
  _virtual_frame(copy._virtual_frame),
  _manage_pieces(copy._manage_pieces),
  _auto_hide(copy._auto_hide)
{
  _needs_remanage = false;
  _needs_recompute_canvas = true;
  _needs_recompute_clip = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *PGScrollFrame::
make_copy() const {
  LightReMutexHolder holder(_lock);
  return new PGScrollFrame(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::cull_callback
//       Access: Protected, Virtual
//  Description: This function will be called during the cull
//               traversal to perform any additional operations that
//               should be performed at cull time.  This may include
//               additional manipulation of render state or additional
//               visible/invisible decisions, or any other arbitrary
//               operation.
//
//               Note that this function will *not* be called unless
//               set_cull_callback() is called in the constructor of
//               the derived class.  It is necessary to call
//               set_cull_callback() to indicated that we require
//               cull_callback() to be called.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool PGScrollFrame::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  LightReMutexHolder holder(_lock);
  if (_manage_pieces && _needs_remanage) {
    remanage();
  }
  if (_needs_recompute_clip) {
    recompute_clip();
  }
  if (_needs_recompute_canvas) {
    recompute_canvas();
  }
  return PGVirtualFrame::cull_callback(trav, data);
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this node by the indicated
//               matrix, if it means anything to do so.  For most
//               kinds of nodes, this does nothing.
////////////////////////////////////////////////////////////////////
void PGScrollFrame::
xform(const LMatrix4 &mat) {
  LightReMutexHolder holder(_lock);
  PGVirtualFrame::xform(mat);

  _needs_remanage = true;
  _needs_recompute_clip = true;
}
  
////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::setup
//       Access: Published
//  Description: Creates a PGScrollFrame with the indicated 
//               dimensions, and the indicated virtual frame.
////////////////////////////////////////////////////////////////////
void PGScrollFrame::
setup(PN_stdfloat width, PN_stdfloat height,
      PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top,
      PN_stdfloat slider_width, PN_stdfloat bevel) {
  LightReMutexHolder holder(_lock);
  set_state(0);
  clear_state_def(0);

  set_frame(0, width, 0, height);

  PGFrameStyle style;
  style.set_width(bevel, bevel);

  style.set_color(0.8f, 0.8f, 0.8f, 1.0f);
  style.set_type(PGFrameStyle::T_ridge);
  set_frame_style(0, style);

  set_clip_frame(bevel, width - bevel,
                 bevel, height - bevel);

  set_virtual_frame(left, right, bottom, top);

  // Remove the slider nodes created by a previous call to setup(), if
  // any.
  if (_horizontal_slider != (PGSliderBar *)NULL) {
    remove_child(_horizontal_slider);
    set_horizontal_slider(NULL);
  }
  if (_vertical_slider != (PGSliderBar *)NULL) {
    remove_child(_vertical_slider);
    set_vertical_slider(NULL);
  }

  // Create new slider bars.
  PT(PGSliderBar) horizontal_slider = new PGSliderBar("horizontal");
  horizontal_slider->setup_scroll_bar(false, width - slider_width - bevel * 2, slider_width, bevel);
  horizontal_slider->set_transform(TransformState::make_pos(LVector3::rfu(width / 2.0f - slider_width / 2.0f, 0, slider_width / 2.0f + bevel)));
  add_child(horizontal_slider);
  set_horizontal_slider(horizontal_slider);

  PT(PGSliderBar) vertical_slider = new PGSliderBar("vertical");
  vertical_slider->setup_scroll_bar(true, width - slider_width - bevel * 2, slider_width, bevel);
  add_child(vertical_slider);
  vertical_slider->set_transform(TransformState::make_pos(LVector3::rfu(width - slider_width / 2.0f - bevel, 0, width / 2.0f + slider_width / 2.0f)));
  set_vertical_slider(vertical_slider);

  set_manage_pieces(true);
  set_auto_hide(true);
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::remanage
//       Access: Published
//  Description: Manages the position and size of the scroll bars.
//               Normally this should not need to be called directly.
////////////////////////////////////////////////////////////////////
void PGScrollFrame::
remanage() {
  LightReMutexHolder holder(_lock);
  _needs_remanage = false;

  const LVecBase4 &frame = get_frame();
  LVecBase4 clip = get_frame_style(get_state()).get_internal_frame(frame);

  // Determine which scroll bars we have in the frame.
  
  bool got_horizontal = false;
  PN_stdfloat horizontal_width = 0.0f;
  if (_horizontal_slider != (PGSliderBar *)NULL) {
    got_horizontal = true;
    const LVecBase4 &slider_frame = _horizontal_slider->get_frame();
    horizontal_width = slider_frame[3] - slider_frame[2];
  }
  
  bool got_vertical = false;
  PN_stdfloat vertical_width = 0.0f;
  if (_vertical_slider != (PGSliderBar *)NULL) {
    got_vertical = true;
    const LVecBase4 &slider_frame = _vertical_slider->get_frame();
    vertical_width = slider_frame[1] - slider_frame[0];
  }

  if (_auto_hide) {
    // Should we hide or show either of the scroll bars?  TODO.

    // Start by figuring out what our maximum clipping area will be.
    PN_stdfloat clip_width = clip[1] - clip[0];
    PN_stdfloat clip_height = clip[3] - clip[2];

    // And our virtual frame too.
    const LVecBase4 &virtual_frame = get_virtual_frame();
    PN_stdfloat virtual_width = virtual_frame[1] - virtual_frame[0];
    PN_stdfloat virtual_height = virtual_frame[3] - virtual_frame[2];

    if (virtual_width <= clip_width &&
        virtual_height <= clip_height) {
      // No need for either slider.
      got_horizontal = false;
      got_vertical = false;

    } else {
      if (virtual_width <= clip_width - vertical_width) {
        // No need for the horizontal slider.
        got_horizontal = false;
      }
      
      if (virtual_height <= clip_height - horizontal_width) {
        // No need for the vertical slider.
        got_vertical = false;
        
        // Now reconsider the need for the horizontal slider.
        if (virtual_width <= clip_width) {
          got_horizontal = false;
        }
      }
    }

    // Now show or hide the sliders appropriately.
    if (_horizontal_slider != (PGSliderBar *)NULL) {
      if (got_horizontal) {
        _horizontal_slider->set_overall_hidden(false);
      } else {
        _horizontal_slider->set_overall_hidden(true);
        _horizontal_slider->set_ratio(0.0f);
        horizontal_width = 0.0f;
      }
    }
    if (_vertical_slider != (PGSliderBar *)NULL) {
      if (got_vertical) {
        _vertical_slider->set_overall_hidden(false);
      } else {
        _vertical_slider->set_overall_hidden(true);
        _vertical_slider->set_ratio(0.0f);
        vertical_width = 0.0f;
      }
    }

    // Showing or hiding one of the scroll bars might have set this
    // flag again indirectly; we clear it again to avoid a feedback
    // loop.
    _needs_remanage = false;
}

  // Are either or both of the scroll bars hidden?
  if (got_horizontal && _horizontal_slider->is_overall_hidden()) {
    got_horizontal = false;
    horizontal_width = 0.0f;
  }
  if (got_vertical && _vertical_slider->is_overall_hidden()) {
    got_vertical = false;
    vertical_width = 0.0f;
  }

  if (got_horizontal) {
    _horizontal_slider->set_frame(clip[0], clip[1] - vertical_width,
                                  clip[2], clip[2] + horizontal_width);
    _horizontal_slider->clear_transform();
  }
  if (got_vertical) {
    _vertical_slider->set_frame(clip[1] - vertical_width, clip[1],
                                clip[2] + horizontal_width, clip[3]);
    _vertical_slider->clear_transform();
  }
    

  recompute();
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::frame_changed
//       Access: Protected, Virtual
//  Description: Called when the user changes the frame size.
////////////////////////////////////////////////////////////////////
void PGScrollFrame::
frame_changed() {
  LightReMutexHolder holder(_lock);
  PGVirtualFrame::frame_changed();
  _needs_remanage = true;
  _needs_recompute_clip = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::item_transform_changed
//       Access: Protected, Virtual
//  Description: Called whenever a watched PGItem's local transform
//               has been changed.
////////////////////////////////////////////////////////////////////
void PGScrollFrame::
item_transform_changed(PGItem *) {
  LightReMutexHolder holder(_lock);
  _needs_recompute_clip = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::item_frame_changed
//       Access: Protected, Virtual
//  Description: Called whenever a watched PGItem's frame
//               has been changed.
////////////////////////////////////////////////////////////////////
void PGScrollFrame::
item_frame_changed(PGItem *) {
  LightReMutexHolder holder(_lock);
  _needs_recompute_clip = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::item_draw_mask_changed
//       Access: Protected, Virtual
//  Description: Called whenever a watched PGItem's draw_mask
//               has been changed.
////////////////////////////////////////////////////////////////////
void PGScrollFrame::
item_draw_mask_changed(PGItem *) {
  LightReMutexHolder holder(_lock);
  _needs_remanage = true;
  _needs_recompute_clip = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::slider_bar_adjust
//       Access: Protected, Virtual
//  Description: Called whenever a watched PGSliderBar's value
//               has been changed by the user or programmatically.
////////////////////////////////////////////////////////////////////
void PGScrollFrame::
slider_bar_adjust(PGSliderBar *) {
  LightReMutexHolder holder(_lock);
  _needs_recompute_canvas = true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::recompute_clip
//       Access: Private
//  Description: Recomputes the clipping window of the PGScrollFrame,
//               based on the position of the slider bars.
////////////////////////////////////////////////////////////////////
void PGScrollFrame::
recompute_clip() {
  LightReMutexHolder holder(_lock);
  _needs_recompute_clip = false;
  _needs_recompute_canvas = true;

  // Figure out how to remove the scroll bars from the clip region.
  LVecBase4 clip = get_frame_style(get_state()).get_internal_frame(get_frame());
  reduce_region(clip, _horizontal_slider);
  reduce_region(clip, _vertical_slider);

  set_clip_frame(clip);

  if (_horizontal_slider != (PGSliderBar *)NULL) {
    _horizontal_slider->set_page_size((clip[1] - clip[0]) / (_virtual_frame[1] - _virtual_frame[0]));
  }
  if (_vertical_slider != (PGSliderBar *)NULL) {
    _vertical_slider->set_page_size((clip[3] - clip[2]) / (_virtual_frame[3] - _virtual_frame[2]));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::recompute_canvas
//       Access: Private
//  Description: Recomputes the portion of the virtual canvas that is
//               visible within the PGScrollFrame, based on the values
//               of the slider bars.
////////////////////////////////////////////////////////////////////
void PGScrollFrame::
recompute_canvas() {
  LightReMutexHolder holder(_lock);
  _needs_recompute_canvas = false;

  const LVecBase4 &clip = get_clip_frame();

  PN_stdfloat x = interpolate_canvas(clip[0], clip[1], 
                               _virtual_frame[0], _virtual_frame[1],
                               _horizontal_slider);

  PN_stdfloat y = interpolate_canvas(clip[3], clip[2], 
                               _virtual_frame[3], _virtual_frame[2],
                               _vertical_slider);

  get_canvas_node()->set_transform(TransformState::make_pos(LVector3::rfu(x, 0, y)));
}

////////////////////////////////////////////////////////////////////
//     Function: PGScrollFrame::interpolate_canvas
//       Access: Private
//  Description: Computes the linear translation that should be
//               applied to the virtual canvas node, based on the
//               corresponding slider bar's position.
////////////////////////////////////////////////////////////////////
PN_stdfloat PGScrollFrame::
interpolate_canvas(PN_stdfloat clip_min, PN_stdfloat clip_max, 
                   PN_stdfloat canvas_min, PN_stdfloat canvas_max,
                   PGSliderBar *slider_bar) {
  LightReMutexHolder holder(_lock);
  PN_stdfloat t = 0.0f;
  if (slider_bar != (PGSliderBar *)NULL) {
    t = slider_bar->get_ratio();
  }

  PN_stdfloat min = clip_min - canvas_min;
  PN_stdfloat max = clip_max - canvas_max;

  return min + t * (max - min);
}
