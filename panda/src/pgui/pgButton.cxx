/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgButton.cxx
 * @author drose
 * @date 2002-03-13
 */

#include "pgButton.h"
#include "pgMouseWatcherParameter.h"

#include "throw_event.h"
#include "mouseButton.h"
#include "mouseWatcherParameter.h"
#include "colorAttrib.h"
#include "transformState.h"

TypeHandle PGButton::_type_handle;

/**
 *
 */
PGButton::
PGButton(const std::string &name) : PGItem(name)
{
  _button_down = false;
  _click_buttons.insert(MouseButton::one());

  set_active(true);
}

/**
 *
 */
PGButton::
~PGButton() {
}

/**
 *
 */
PGButton::
PGButton(const PGButton &copy) :
  PGItem(copy),
  _click_buttons(copy._click_buttons)
{
  _button_down = false;
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *PGButton::
make_copy() const {
  LightReMutexHolder holder(_lock);
  return new PGButton(*this);
}

/**
 * This is a callback hook function, called whenever the mouse enters the
 * region.
 */
void PGButton::
enter_region(const MouseWatcherParameter &param) {
  LightReMutexHolder holder(_lock);
  if (get_active()) {
    set_state(_button_down ? S_depressed : S_rollover);
  }
  PGItem::enter_region(param);
}

/**
 * This is a callback hook function, called whenever the mouse exits the
 * region.
 */
void PGButton::
exit_region(const MouseWatcherParameter &param) {
  LightReMutexHolder holder(_lock);
  if (get_active()) {
    set_state(S_ready);
  }
  PGItem::exit_region(param);
}

/**
 * This is a callback hook function, called whenever a mouse or keyboard
 * button is depressed while the mouse is within the region.
 */
void PGButton::
press(const MouseWatcherParameter &param, bool background) {
  LightReMutexHolder holder(_lock);
  if (has_click_button(param.get_button())) {
    if (get_active()) {
      _button_down = true;
      set_state(S_depressed);
    }
  }
  PGItem::press(param, background);
}

/**
 * This is a callback hook function, called whenever a mouse or keyboard
 * button previously depressed with press() is released.
 */
void PGButton::
release(const MouseWatcherParameter &param, bool background) {
  LightReMutexHolder holder(_lock);
  if (has_click_button(param.get_button())) {
    _button_down = false;
    if (get_active()) {
      // Note that a "click" may come from a keyboard button press.  In that
      // case, instead of checking that the mouse cursor is still over the
      // button, we check whether the item has keyboard focus.
      if (param.is_outside() &&
          (MouseButton::is_mouse_button(param.get_button()) || !get_focus())) {
        set_state(S_ready);
      } else {
        set_state(S_rollover);
        click(param);
      }
    }
  }
  PGItem::release(param, background);
}

/**
 * This is a callback hook function, called whenever the button is clicked
 * down-and-up by the user normally.
 */
void PGButton::
click(const MouseWatcherParameter &param) {
  LightReMutexHolder holder(_lock);
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  std::string event = get_click_event(param.get_button());
  play_sound(event);
  throw_event(event, EventParameter(ep));

  if (has_notify()) {
    get_notify()->button_click(this, param);
  }
}

/**
 * Sets up the button as a default text button using the indicated label
 * string.  The TextNode defined by PGItem::get_text_node() will be used to
 * create the label geometry.  This automatically sets up the frame according
 * to the size of the text.
 */
void PGButton::
setup(const std::string &label, PN_stdfloat bevel) {
  LightReMutexHolder holder(_lock);
  clear_state_def(S_ready);
  clear_state_def(S_depressed);
  clear_state_def(S_rollover);
  clear_state_def(S_inactive);

  TextNode *text_node = get_text_node();
  text_node->set_text(label);
  PT(PandaNode) geom = text_node->generate();

  LVecBase4 frame = text_node->get_card_actual();
  set_frame(frame[0] - 0.4, frame[1] + 0.4, frame[2] - 0.15f, frame[3] + 0.15f);

  PT(PandaNode) ready = new PandaNode("ready");
  PT(PandaNode) depressed = new PandaNode("depressed");
  PT(PandaNode) rollover = new PandaNode("rollover");
  PT(PandaNode) inactive = new PandaNode("inactive");

  PGFrameStyle style;
  style.set_color(0.8f, 0.8f, 0.8f, 1.0f);
  style.set_width(bevel, bevel);

  style.set_type(PGFrameStyle::T_bevel_out);
  set_frame_style(S_ready, style);

  style.set_color(0.9f, 0.9f, 0.9f, 1.0f);
  set_frame_style(S_rollover, style);

  inactive->set_attrib(ColorAttrib::make_flat(LColor(0.8f, 0.8f, 0.8f, 1.0f)));
  style.set_color(0.6f, 0.6f, 0.6f, 1.0f);
  set_frame_style(S_inactive, style);

  style.set_type(PGFrameStyle::T_bevel_in);
  style.set_color(0.8f, 0.8f, 0.8f, 1.0f);
  set_frame_style(S_depressed, style);
  depressed->set_transform(TransformState::make_pos(LVector3(0.05f, 0.0f, -0.05f)));

  get_state_def(S_ready).attach_new_node(ready, 1);
  get_state_def(S_depressed).attach_new_node(depressed, 1);
  get_state_def(S_rollover).attach_new_node(rollover, 1);
  get_state_def(S_inactive).attach_new_node(inactive, 1);

  ready->add_child(geom);
  depressed->add_child(geom);
  rollover->add_child(geom);
  inactive->add_child(geom);
}

/**
 * Sets up the button using the indicated NodePath as arbitrary geometry.
 */
void PGButton::
setup(const NodePath &ready, const NodePath &depressed,
      const NodePath &rollover, const NodePath &inactive) {
  LightReMutexHolder holder(_lock);
  clear_state_def(S_ready);
  clear_state_def(S_depressed);
  clear_state_def(S_rollover);
  clear_state_def(S_inactive);

  instance_to_state_def(S_ready, ready);
  instance_to_state_def(S_depressed, depressed);
  instance_to_state_def(S_rollover, rollover);
  instance_to_state_def(S_inactive, inactive);

  // Set the button frame size.
  LPoint3 min_point, max_point;
  ready.calc_tight_bounds(min_point, max_point);
  set_frame(min_point[0], max_point[0],
            min_point[2], max_point[2]);
}

/**
 * Toggles the active/inactive state of the button.  In the case of a
 * PGButton, this also changes its visual appearance.
 */
void PGButton::
set_active(bool active) {
  LightReMutexHolder holder(_lock);
  if (active != get_active()) {
    PGItem::set_active(active);
    set_state(active ? S_ready : S_inactive);
  }
}

/**
 * Adds the indicated button to the set of buttons that can effectively
 * "click" the PGButton.  Normally, this is just MouseButton::one().  Returns
 * true if the button was added, or false if it was already there.
 */
bool PGButton::
add_click_button(const ButtonHandle &button) {
  LightReMutexHolder holder(_lock);
  return _click_buttons.insert(button).second;
}

/**
 * Removes the indicated button from the set of buttons that can effectively
 * "click" the PGButton.  Normally, this is just MouseButton::one().  Returns
 * true if the button was removed, or false if it was not in the set.
 */
bool PGButton::
remove_click_button(const ButtonHandle &button) {
  LightReMutexHolder holder(_lock);
  return (_click_buttons.erase(button) != 0);
}

/**
 * Returns true if the indicated button is on the set of buttons that can
 * effectively "click" the PGButton.  Normally, this is just
 * MouseButton::one().
 */
bool PGButton::
has_click_button(const ButtonHandle &button) {
  LightReMutexHolder holder(_lock);
  return (_click_buttons.count(button) != 0);
}
