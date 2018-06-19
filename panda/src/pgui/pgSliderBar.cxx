/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgSliderBar.cxx
 * @author masad
 * @date 2004-10-19
 */

#include "pgSliderBar.h"
#include "pgMouseWatcherParameter.h"
#include "clockObject.h"
#include "throw_event.h"
#include "config_pgui.h"
#include "throw_event.h"
#include "transformState.h"
#include "mouseButton.h"

using std::max;
using std::min;

TypeHandle PGSliderBar::_type_handle;

/**
 *
 */
PGSliderBar::
PGSliderBar(const std::string &name)
  : PGItem(name)
{
  set_cull_callback();

  _min_value = 0.0f;
  _max_value = 1.0f;
  set_scroll_size(0.01f);
  set_page_size(0.1f);
  _ratio = 0.0f;
  _resize_thumb = false;
  _manage_pieces = false;
  _axis.set(1.0f, 0.0f, 0.0f);
  _needs_remanage = false;
  _needs_recompute = true;
  _needs_reposition = false;
  _scroll_button_held = nullptr;
  _mouse_button_page = false;
  _dragging = false;
  _thumb_width = 0.1f;

  set_active(true);
}

/**
 *
 */
PGSliderBar::
~PGSliderBar() {
}

/**
 *
 */
PGSliderBar::
PGSliderBar(const PGSliderBar &copy) :
  PGItem(copy),
  _min_value(copy._min_value),
  _max_value(copy._max_value),
  _scroll_value(copy._scroll_value),
  _scroll_ratio(copy._scroll_ratio),
  _page_value(copy._page_value),
  _page_ratio(copy._page_ratio),
  _ratio(copy._ratio),
  _resize_thumb(copy._resize_thumb),
  _manage_pieces(copy._manage_pieces),
  _axis(copy._axis)
{
  _needs_remanage = false;
  _needs_recompute = true;
  _scroll_button_held = nullptr;
  _mouse_button_page = false;
  _dragging = false;
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *PGSliderBar::
make_copy() const {
  LightReMutexHolder holder(_lock);
  return new PGSliderBar(*this);
}

/**
 * This is a callback hook function, called whenever a mouse or keyboard
 * button is depressed while the mouse is within the region.
 */
void PGSliderBar::
press(const MouseWatcherParameter &param, bool background) {
  LightReMutexHolder holder(_lock);
  if (param.has_mouse()) {
    _mouse_pos = param.get_mouse();
  }
  if (get_active() && param.get_button() == MouseButton::one()) {
    if (_needs_recompute) {
      recompute();
    }

    if (_range_x != 0.0f) {
      _mouse_button_page = true;
      _scroll_button_held = nullptr;
      advance_page();
      _next_advance_time =
        ClockObject::get_global_clock()->get_frame_time() + scroll_initial_delay;
    }
  }
  PGItem::press(param, background);
}

/**
 * This is a callback hook function, called whenever a mouse or keyboard
 * button previously depressed with press() is released.
 */
void PGSliderBar::
release(const MouseWatcherParameter &param, bool background) {
  LightReMutexHolder holder(_lock);
  if (MouseButton::is_mouse_button(param.get_button())) {
    _mouse_button_page = false;
  }
  if (_dragging) {
    end_drag();
  }
  PGItem::release(param, background);
}

/**
 * This is a callback hook function, called whenever a mouse is moved while
 * within the region.
 */
void PGSliderBar::
move(const MouseWatcherParameter &param) {
  LightReMutexHolder holder(_lock);
  _mouse_pos = param.get_mouse();
  if (_dragging) {
    // We only get here if we the user originally clicked on the track, which
    // caused the slider to move all the way to the mouse position, and then
    // started dragging the mouse along the track.  In this case, we start
    // moving the thumb as if the user had started by dragging the thumb
    // directly.
    continue_drag();
  }
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool PGSliderBar::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  LightReMutexHolder holder(_lock);
  if (_manage_pieces && _needs_remanage) {
    remanage();
  }
  if (_needs_recompute) {
    recompute();
  }

  if (_scroll_button_held != nullptr &&
      _next_advance_time <= ClockObject::get_global_clock()->get_frame_time()) {
    advance_scroll();
  }

  if (_mouse_button_page &&
      _next_advance_time <= ClockObject::get_global_clock()->get_frame_time()) {
    advance_page();
  }

  if (_needs_reposition) {
    reposition();
  }

  return PGItem::cull_callback(trav, data);
}

/**
 * Transforms the contents of this node by the indicated matrix, if it means
 * anything to do so.  For most kinds of nodes, this does nothing.
 */
void PGSliderBar::
xform(const LMatrix4 &mat) {
  LightReMutexHolder holder(_lock);
  PGItem::xform(mat);
  _axis = _axis * mat;

  // Make sure we set the thumb to identity position first, so it won't be
  // accidentally flattened.
  if (_thumb_button != nullptr) {
    _thumb_button->clear_transform();
  }

  _needs_remanage = true;
  _needs_recompute = true;
}

/**
 * This is a callback hook function, called whenever the slider value is
 * adjusted by the user or programmatically.
 */
void PGSliderBar::
adjust() {
  LightReMutexHolder holder(_lock);
  std::string event = get_adjust_event();
  play_sound(event);
  throw_event(event);

  if (has_notify()) {
    get_notify()->slider_bar_adjust(this);
  }
}

/**
 * Creates PGSliderBar that represents a vertical or horizontal scroll bar (if
 * vertical is true or false, respectively), with additional buttons for
 * scrolling, and a range of 0 .. 1.
 *
 * length here is the measurement along the scroll bar, and width is the
 * measurement across the scroll bar, whether it is vertical or horizontal (so
 * for a horizontal scroll bar, the length is actually the x dimension, and
 * the width is the y dimension).
 */
void PGSliderBar::
setup_scroll_bar(bool vertical, PN_stdfloat length, PN_stdfloat width, PN_stdfloat bevel) {
  LightReMutexHolder holder(_lock);
  set_state(0);
  clear_state_def(0);

  if (vertical) {
    set_frame(-width / 2.0f, width / 2.0f, -length / 2.0f, length / 2.0f);
    _axis = LVector3::rfu(0.0f, 0.0f, -1.0f);
  } else {
    set_frame(-length / 2.0f, length / 2.0f, -width / 2.0f, width / 2.0f);
    _axis = LVector3::rfu(1.0f, 0.0f, 0.0f);
  }

  PGFrameStyle style;
  style.set_color(0.6f, 0.6f, 0.6f, 1.0f);
  style.set_type(PGFrameStyle::T_flat);
  set_frame_style(0, style);

  style.set_color(0.8f, 0.8f, 0.8f, 1.0f);
  style.set_type(PGFrameStyle::T_bevel_out);
  style.set_width(bevel, bevel);

  // Remove the button nodes created by a previous call to setup(), if any.
  if (_thumb_button != nullptr) {
    remove_child(_thumb_button);
    set_thumb_button(nullptr);
  }
  if (_left_button != nullptr) {
    remove_child(_left_button);
    set_left_button(nullptr);
  }
  if (_right_button != nullptr) {
    remove_child(_right_button);
    set_right_button(nullptr);
  }

  PT(PGButton) thumb = new PGButton("thumb");
  thumb->setup("", bevel);
  thumb->set_frame(-width / 2.0f, width / 2.0f,
                   -width / 2.0f, width / 2.0f);
  add_child(thumb);
  set_thumb_button(thumb);

  PT(PGButton) left = new PGButton("left");
  left->setup("", bevel);
  left->set_frame(-width / 2.0f, width / 2.0f,
                  -width / 2.0f, width / 2.0f);
  left->set_transform(TransformState::make_pos(((width - length) / 2.0f) * _axis));
  add_child(left);
  set_left_button(left);

  PT(PGButton) right = new PGButton("right");
  right->setup("", bevel);
  right->set_frame(-width / 2.0f, width / 2.0f,
                   -width / 2.0f, width / 2.0f);
  right->set_transform(TransformState::make_pos(((length - width) / 2.0f) * _axis));
  add_child(right);
  set_right_button(right);

  set_resize_thumb(true);
  set_manage_pieces(true);
}

/**
 * Creates PGSliderBar that represents a slider that the user can use to
 * control an analog quantity.
 *
 * This is functionally the same as a scroll bar, but it has a distinctive
 * look.
 */
void PGSliderBar::
setup_slider(bool vertical, PN_stdfloat length, PN_stdfloat width, PN_stdfloat bevel) {
  LightReMutexHolder holder(_lock);
  set_state(0);
  clear_state_def(0);

  if (vertical) {
    set_frame(-width / 2.0f, width / 2.0f, -length / 2.0f, length / 2.0f);
    _axis = LVector3::rfu(0.0f, 0.0f, -1.0f);
  } else {
    set_frame(-length / 2.0f, length / 2.0f, -width / 2.0f, width / 2.0f);
    _axis = LVector3::rfu(1.0f, 0.0f, 0.0f);
  }

  PGFrameStyle style;
  style.set_color(0.6f, 0.6f, 0.6f, 1.0f);
  style.set_type(PGFrameStyle::T_flat);
  style.set_visible_scale(1.0f, 0.25f);
  style.set_width(bevel, bevel);
  set_frame_style(0, style);

  // Remove the button nodes created by a previous call to setup(), if any.
  if (_thumb_button != nullptr) {
    remove_child(_thumb_button);
    set_thumb_button(nullptr);
  }
  if (_left_button != nullptr) {
    remove_child(_left_button);
    set_left_button(nullptr);
  }
  if (_right_button != nullptr) {
    remove_child(_right_button);
    set_right_button(nullptr);
  }

  PT(PGButton) thumb = new PGButton("thumb");
  thumb->setup(" ", bevel);
  thumb->set_frame(-width / 4.0f, width / 4.0f,
                   -width / 2.0f, width / 2.0f);
  add_child(thumb);
  set_thumb_button(thumb);

  set_resize_thumb(false);
  set_manage_pieces(true);
}

/**
 * Sets whether the PGItem is active for mouse watching.  This is not
 * necessarily related to the active/inactive appearance of the item, which is
 * controlled by set_state(), but it does affect whether it responds to mouse
 * events.
 */
void PGSliderBar::
set_active(bool active) {
  LightReMutexHolder holder(_lock);
  PGItem::set_active(active);

  // This also implicitly sets the managed pieces.
  if (_thumb_button != nullptr) {
    _thumb_button->set_active(active);
  }
  if (_left_button != nullptr) {
    _left_button->set_active(active);
  }
  if (_right_button != nullptr) {
    _right_button->set_active(active);
  }
}

/**
 * Manages the position and size of the scroll bars and the thumb.  Normally
 * this should not need to be called directly.
 */
void PGSliderBar::
remanage() {
  LightReMutexHolder holder(_lock);
  _needs_remanage = false;

  const LVecBase4 &frame = get_frame();

  PN_stdfloat width, length;
  if (fabs(_axis[0]) > fabs(_axis[1] + _axis[2])) {
    // The slider is X-dominant.
    width = frame[3] - frame[2];
    length = frame[1] - frame[0];

  } else {
    // The slider is Y-dominant.
    width = frame[1] - frame[0];
    length = frame[3] - frame[2];
  }

  LVector3 center = LVector3::rfu((frame[0] + frame[1]) / 2.0f,
                                    0.0f,
                                    (frame[2] + frame[3]) / 2.0f);

  if (_left_button != nullptr) {
    _left_button->set_frame(-width / 2.0f, width / 2.0f,
                            -width / 2.0f, width / 2.0f);
    _left_button->set_transform(TransformState::make_pos(center + ((width - length) / 2.0f) * _axis));
  }

  if (_right_button != nullptr) {
    _right_button->set_frame(-width / 2.0f, width / 2.0f,
                             -width / 2.0f, width / 2.0f);
    _right_button->set_transform(TransformState::make_pos(center + ((length - width) / 2.0f) * _axis));
  }

  if (_thumb_button != nullptr) {
    _thumb_button->set_frame(-width / 2.0f, width / 2.0f,
                             -width / 2.0f, width / 2.0f);
    _thumb_button->set_transform(TransformState::make_pos(center));
  }

  recompute();
}

/**
 * Recomputes the position and size of the thumb.  Normally this should not
 * need to be called directly.
 */
void PGSliderBar::
recompute() {
  LightReMutexHolder holder(_lock);
  _needs_recompute = false;

  if (_min_value != _max_value) {
    _scroll_ratio = fabs(_scroll_value / (_max_value - _min_value));
    _page_ratio = fabs(_page_value / (_max_value - _min_value));

  } else {
    _scroll_ratio = 0.0f;
    _page_ratio = 0.0f;
  }

  if (!has_frame()) {
    _min_x = 0.0f;
    _max_x = 0.0f;
    _thumb_width = 0.0f;
    _range_x = 0.0f;
    _thumb_start.set(0.0f, 0.0f, 0.0f);

  } else {
    LVecBase4 frame = get_frame();
    reduce_region(frame, _left_button);
    reduce_region(frame, _right_button);

    if (fabs(_axis[0]) > fabs(_axis[1] + _axis[2])) {
      // The slider is X-dominant.

      _min_x = frame[0];
      _max_x = frame[1];

      PN_stdfloat trough_width = _max_x - _min_x;

      if (_thumb_button == nullptr) {
        _thumb_width = 0.0f;
        _range_x = 0.0f;
        _thumb_start.set(0.0f, 0.0f, 0.0f);

      } else {
        const LVecBase4 &thumb_frame = _thumb_button->get_frame();

        if (_resize_thumb) {
          // If we're allowed to adjust the thumb's size, we don't need to
          // find out how wide it is.
          _thumb_width = trough_width * min((PN_stdfloat)1.0, _page_ratio);
          _thumb_button->set_frame(-_thumb_width / 2.0f, _thumb_width / 2.0f,
                                   thumb_frame[2], thumb_frame[3]);
        } else {
          // If we're not adjusting the thumb's size, we do need to know its
          // current width.
          _thumb_width = thumb_frame[1] - thumb_frame[0];
        }

        _range_x = trough_width - _thumb_width;

        if (_axis[0] >= 0.0f) {
          // The slider runs forwards, left to right.
          _thumb_start = (_min_x - thumb_frame[0]) * _axis;
        } else {
          // The slider runs backwards: right to left.
          _thumb_start = (thumb_frame[1] - _max_x) * _axis;
        }
        _thumb_start += LVector3::rfu(0.0f, 0.0f, (frame[2] + frame[3]) / 2.0f);
      }

    } else {
      // The slider is Y-dominant.  We call it X in the variable names, but
      // it's really Y (or even Z).

      _min_x = frame[2];
      _max_x = frame[3];

      PN_stdfloat trough_width = _max_x - _min_x;

      if (_thumb_button == nullptr) {
        _thumb_width = 0.0f;
        _range_x = 0.0f;
        _thumb_start.set(0.0f, 0.0f, 0.0f);

      } else {
        const LVecBase4 &thumb_frame = _thumb_button->get_frame();

        if (_resize_thumb) {
          // If we're allowed to adjust the thumb's size, we don't need to
          // find out how wide it is.
          _thumb_width = trough_width * min((PN_stdfloat)1.0, _page_ratio);
          _thumb_button->set_frame(thumb_frame[0], thumb_frame[1],
                                   -_thumb_width / 2.0f, _thumb_width / 2.0f);
        } else {
          // If we're not adjusting the thumb's size, we do need to know its
          // current width.
          _thumb_width = thumb_frame[3] - thumb_frame[2];
        }

        _range_x = trough_width - _thumb_width;

        if (_axis[1] >= 0.0f && _axis[2] >= 0.0f) {
          // The slider runs forwards, bottom to top.
          _thumb_start = (_min_x - thumb_frame[2]) * _axis;
        } else {
          // The slider runs backwards: top to bottom.
          _thumb_start = (thumb_frame[3] - _max_x) * _axis;
        }
        _thumb_start += LVector3::rfu((frame[0] + frame[1]) / 2.0f, 0.0f, 0.0f);
      }
    }
  }

  reposition();
}

/**
 * Called when the user changes the frame size.
 */
void PGSliderBar::
frame_changed() {
  LightReMutexHolder holder(_lock);
  PGItem::frame_changed();
  _needs_remanage = true;
  _needs_recompute = true;
}

/**
 * Called whenever a watched PGItem's local transform has been changed.
 */
void PGSliderBar::
item_transform_changed(PGItem *) {
  LightReMutexHolder holder(_lock);
  _needs_recompute = true;
}

/**
 * Called whenever a watched PGItem's frame has been changed.
 */
void PGSliderBar::
item_frame_changed(PGItem *) {
  LightReMutexHolder holder(_lock);
  _needs_recompute = true;
}

/**
 * Called whenever a watched PGItem's draw_mask has been changed.
 */
void PGSliderBar::
item_draw_mask_changed(PGItem *) {
  LightReMutexHolder holder(_lock);
  _needs_recompute = true;
}

/**
 * Called whenever the "press" event is triggered on a watched PGItem.  See
 * PGItem::press().
 */
void PGSliderBar::
item_press(PGItem *item, const MouseWatcherParameter &param) {
  LightReMutexHolder holder(_lock);
  if (param.has_mouse()) {
    _mouse_pos = param.get_mouse();
  }
  if (item == _left_button || item == _right_button) {
    _scroll_button_held = item;
    _mouse_button_page = false;
    advance_scroll();
    _next_advance_time =
      ClockObject::get_global_clock()->get_frame_time() + scroll_initial_delay;

  } else if (item == _thumb_button) {
    _scroll_button_held = nullptr;
    begin_drag();
  }
}

/**
 * Called whenever the "release" event is triggered on a watched PGItem.  See
 * PGItem::release().
 */
void PGSliderBar::
item_release(PGItem *item, const MouseWatcherParameter &) {
  LightReMutexHolder holder(_lock);
  if (item == _scroll_button_held) {
    _scroll_button_held = nullptr;

  } else if (item == _thumb_button) {
    _scroll_button_held = nullptr;
    if (_dragging) {
      end_drag();
    }
  }
}

/**
 * Called whenever the "move" event is triggered on a watched PGItem.  See
 * PGItem::move().
 */
void PGSliderBar::
item_move(PGItem *item, const MouseWatcherParameter &param) {
  LightReMutexHolder holder(_lock);
  _mouse_pos = param.get_mouse();
  if (item == _thumb_button) {
    if (_dragging) {
      continue_drag();
    }
  }
}

/**
 * A lighter-weight version of recompute(), this just moves the thumb,
 * assuming all other properties are unchanged.
 */
void PGSliderBar::
reposition() {
  _needs_reposition = false;

  PN_stdfloat t = get_ratio();

  if (_thumb_button != nullptr) {
    LPoint3 pos = (t * _range_x) * _axis + _thumb_start;
    CPT(TransformState) transform = TransformState::make_pos(pos);
    CPT(TransformState) orig_transform = _thumb_button->get_transform();

    // It's important not to update the transform frivolously, or we'll get
    // caught in an update loop.
    if (transform == orig_transform) {
      // No change.
    } else if (*transform != *orig_transform) {
      _thumb_button->set_transform(transform);
    }
  }
}

/**
 * Advances the scroll bar by one unit in the left or right direction while
 * the user is holding down the left or right scroll button.
 */
void PGSliderBar::
advance_scroll() {
  if (_scroll_button_held == _left_button) {
    internal_set_ratio(max(_ratio - _scroll_ratio, (PN_stdfloat)0.0));

  } else if (_scroll_button_held == _right_button) {
    internal_set_ratio(min(_ratio + _scroll_ratio, (PN_stdfloat)1.0));
  }

  _next_advance_time =
    ClockObject::get_global_clock()->get_frame_time() + scroll_continued_delay;
}

/**
 * Advances the scroll bar by one page in the left or right direction while
 * the user is holding down the mouse button on the track.
 */
void PGSliderBar::
advance_page() {
  // Is the mouse position left or right of the current thumb position?
  LPoint3 mouse = mouse_to_local(_mouse_pos) - _thumb_start;
  PN_stdfloat target_ratio = mouse.dot(_axis) / _range_x;

  PN_stdfloat t;
  if (target_ratio < _ratio) {
    t = max(_ratio - _page_ratio + _scroll_ratio, target_ratio);

  } else {
    t = min(_ratio + _page_ratio - _scroll_ratio, target_ratio);
  }
  internal_set_ratio(t);
  if (t == target_ratio) {
    // We made it; begin dragging from now on until the user releases the
    // mouse.
    begin_drag();
  }

  _next_advance_time =
    ClockObject::get_global_clock()->get_frame_time() + scroll_continued_delay;
}

/**
 * Called when the user clicks down on the thumb button, possibly to begin
 * dragging.
 */
void PGSliderBar::
begin_drag() {
  if (_needs_recompute) {
    recompute();
  }
  if (_range_x != 0.0f) {
    PN_stdfloat current_x = mouse_to_local(_mouse_pos).dot(_axis);
    _drag_start_x = current_x - get_ratio() * _range_x;
    _dragging = true;
  }
}

/**
 * Called as the user moves the mouse while still dragging on the thumb
 * button.
 */
void PGSliderBar::
continue_drag() {
  if (_needs_recompute) {
    recompute();
  }
  if (_range_x != 0.0f) {
    PN_stdfloat current_x = mouse_to_local(_mouse_pos).dot(_axis);
    internal_set_ratio((current_x - _drag_start_x) / _range_x);
  }
}

/**
 * Called as the user releases the mouse after dragging.
 */
void PGSliderBar::
end_drag() {
  _dragging = false;
}
