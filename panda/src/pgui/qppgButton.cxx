// Filename: qppgButton.cxx
// Created by:  drose (13Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "qppgButton.h"
#include "pgMouseWatcherParameter.h"

#include "throw_event.h"
#include "mouseButton.h"
#include "mouseWatcherParameter.h"
#include "colorAttrib.h"
#include "transformState.h"

TypeHandle qpPGButton::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpPGButton::
qpPGButton(const string &name) : qpPGItem(name)
{
  _button_down = false;
  _click_buttons.insert(MouseButton::one());

  set_active(true);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpPGButton::
~qpPGButton() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
qpPGButton::
qpPGButton(const qpPGButton &copy) :
  qpPGItem(copy)
{
  _button_down = false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *qpPGButton::
make_copy() const {
  return new qpPGButton(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::enter
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse enters the region.
////////////////////////////////////////////////////////////////////
void qpPGButton::
enter(const MouseWatcherParameter &param) {
  if (get_active()) {
    set_state(_button_down ? S_depressed : S_rollover);
  }
  qpPGItem::enter(param);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::exit
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse exits the region.
////////////////////////////////////////////////////////////////////
void qpPGButton::
exit(const MouseWatcherParameter &param) {
  if (get_active()) {
    set_state(S_ready);
  }
  qpPGItem::exit(param);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::press
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void qpPGButton::
press(const MouseWatcherParameter &param, bool background) {
  if (has_click_button(param.get_button())) {
    if (get_active()) {
      _button_down = true;
      set_state(S_depressed);
    }
  }
  qpPGItem::press(param, background);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::release
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button previously depressed with
//               press() is released.
////////////////////////////////////////////////////////////////////
void qpPGButton::
release(const MouseWatcherParameter &param, bool background) {
  if (has_click_button(param.get_button())) {
    _button_down = false;
    if (get_active()) {
      if (param.is_outside()) {
        set_state(S_ready);
      } else {
        set_state(S_rollover);
        click(param);
      }
    }
  }
  qpPGItem::release(param, background);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::click
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               button is clicked down-and-up by the user normally.
////////////////////////////////////////////////////////////////////
void qpPGButton::
click(const MouseWatcherParameter &param) {
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_click_event(param.get_button());
  play_sound(event);
  throw_event(event, EventParameter(ep));
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::setup
//       Access: Published
//  Description: Sets up the button as a default text button using the
//               indicated label string.  The TextNode defined by
//               PGItem::get_text_node() will be used to create the
//               label geometry.  This automatically sets up the frame
//               according to the size of the text.
////////////////////////////////////////////////////////////////////
void qpPGButton::
setup(const string &label) {
  clear_state_def(S_ready);
  clear_state_def(S_depressed);
  clear_state_def(S_rollover);
  clear_state_def(S_inactive);

  qpTextNode *text_node = get_text_node();
  text_node->set_text(label);
  PT(PandaNode) geom = text_node->generate();

  LVecBase4f frame = text_node->get_card_actual();
  set_frame(frame[0] - 0.4f, frame[1] + 0.4f, frame[2] - 0.15f, frame[3] + 0.15f);

  PT(PandaNode) ready = new PandaNode("ready");
  PT(PandaNode) depressed = new PandaNode("depressed");
  PT(PandaNode) rollover = new PandaNode("rollover");
  PT(PandaNode) inactive = new PandaNode("inactive");

  get_state_def(S_ready)->add_child(ready);
  get_state_def(S_depressed)->add_child(depressed);
  get_state_def(S_rollover)->add_child(rollover);
  get_state_def(S_inactive)->add_child(inactive);

  ready->add_child(geom);
  depressed->add_child(geom);
  rollover->add_child(geom);
  inactive->add_child(geom);

  PGFrameStyle style;
  style.set_color(0.8f, 0.8f, 0.8f, 1.0);
  style.set_width(0.1f, 0.1f);

  style.set_type(PGFrameStyle::T_bevel_out);
  set_frame_style(S_ready, style);

  style.set_color(0.9f, 0.9f, 0.9f, 1.0);
  set_frame_style(S_rollover, style);

  inactive->set_attrib(ColorAttrib::make_flat(Colorf(0.8f, 0.8f, 0.8f, 1.0f)));
  style.set_color(0.6f, 0.6f, 0.6f, 1.0);
  set_frame_style(S_inactive, style);

  style.set_type(PGFrameStyle::T_bevel_in);
  style.set_color(0.8f, 0.8f, 0.8f, 1.0);
  set_frame_style(S_depressed, style);
  depressed->set_transform(TransformState::make_pos(LVector3f(0.05f, 0.0f, -0.05f)));
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::setup
//       Access: Published
//  Description: Sets up the button using the indicated NodePath as
//               arbitrary geometry.
////////////////////////////////////////////////////////////////////
void qpPGButton::
setup(const qpNodePath &ready, const qpNodePath &depressed, 
      const qpNodePath &rollover, const qpNodePath &inactive) {
  clear_state_def(S_ready);
  clear_state_def(S_depressed);
  clear_state_def(S_rollover);
  clear_state_def(S_inactive);

  instance_to_state_def(S_ready, ready);
  instance_to_state_def(S_depressed, depressed);
  instance_to_state_def(S_rollover, rollover);
  instance_to_state_def(S_inactive, inactive);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::set_active
//       Access: Published, Virtual
//  Description: Toggles the active/inactive state of the button.  In
//               the case of a qpPGButton, this also changes its visual
//               appearance.
////////////////////////////////////////////////////////////////////
void qpPGButton:: 
set_active(bool active) {
  if (active != get_active()) {
    qpPGItem::set_active(active);
    set_state(active ? S_ready : S_inactive);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::add_click_button
//       Access: Published
//  Description: Adds the indicated button to the set of buttons that
//               can effectively "click" the qpPGButton.  Normally, this
//               is just MouseButton::one().  Returns true if the
//               button was added, or false if it was already there.
////////////////////////////////////////////////////////////////////
bool qpPGButton::
add_click_button(const ButtonHandle &button) {
  return _click_buttons.insert(button).second;
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::remove_click_button
//       Access: Published
//  Description: Removes the indicated button from the set of buttons
//               that can effectively "click" the qpPGButton.  Normally,
//               this is just MouseButton::one().  Returns true if the
//               button was removed, or false if it was not in the
//               set.
////////////////////////////////////////////////////////////////////
bool qpPGButton::
remove_click_button(const ButtonHandle &button) {
  return (_click_buttons.erase(button) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: qpPGButton::has_click_button
//       Access: Published
//  Description: Returns true if the indicated button is on the set of
//               buttons that can effectively "click" the qpPGButton.
//               Normally, this is just MouseButton::one().
////////////////////////////////////////////////////////////////////
bool qpPGButton::
has_click_button(const ButtonHandle &button) {
  return (_click_buttons.count(button) != 0);
}
