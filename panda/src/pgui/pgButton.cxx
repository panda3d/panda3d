// Filename: pgButton.cxx
// Created by:  drose (03Jul01)
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

#include "pgButton.h"
#include "pgMouseWatcherParameter.h"

#include "throw_event.h"
#include "renderRelation.h"
#include "colorTransition.h"
#include "transformTransition.h"
#include "mouseButton.h"
#include "mouseWatcherParameter.h"

TypeHandle PGButton::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGButton::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PGButton::
PGButton(const string &name) : PGItem(name)
{
  _button_down = false;
  _click_buttons.insert(MouseButton::one());
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGButton::
~PGButton() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PGButton::
PGButton(const PGButton &copy) :
  PGItem(copy)
{
  _button_down = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PGButton::
operator = (const PGButton &copy) {
  PGItem::operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *PGButton::
make_copy() const {
  return new PGButton(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::enter
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse enters the region.
////////////////////////////////////////////////////////////////////
void PGButton::
enter(const MouseWatcherParameter &param) {
  if (get_active()) {
    set_state(_button_down ? S_depressed : S_rollover);
  }
  PGItem::enter(param);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::exit
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse exits the region.
////////////////////////////////////////////////////////////////////
void PGButton::
exit(const MouseWatcherParameter &param) {
  if (get_active()) {
    set_state(S_ready);
  }
  PGItem::exit(param);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::press
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void PGButton::
press(const MouseWatcherParameter &param, bool background) {
  if (has_click_button(param.get_button())) {
    if (get_active()) {
      _button_down = true;
      set_state(S_depressed);
    }
  }
  PGItem::press(param, background);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::release
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button previously depressed with
//               press() is released.
////////////////////////////////////////////////////////////////////
void PGButton::
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
  PGItem::release(param, background);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::click
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               button is clicked down-and-up by the user normally.
////////////////////////////////////////////////////////////////////
void PGButton::
click(const MouseWatcherParameter &param) {
  PGMouseWatcherParameter *ep = new PGMouseWatcherParameter(param);
  string event = get_click_event(param.get_button());
  play_sound(event);
  throw_event(event, EventParameter(ep));
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::setup
//       Access: Published
//  Description: Sets up the button as a default text button using the
//               indicated label string.  The TextNode defined by
//               PGItem::get_text_node() will be used to create the
//               label geometry.  This automatically sets up the frame
//               according to the size of the text.
////////////////////////////////////////////////////////////////////
void PGButton::
setup(const string &label) {
  clear_state_def(S_ready);
  clear_state_def(S_depressed);
  clear_state_def(S_rollover);
  clear_state_def(S_inactive);

  TextNode *text_node = get_text_node();
  text_node->set_text(label);
  PT_Node geom = text_node->generate();

  LVecBase4f frame = text_node->get_card_actual();
  set_frame(frame[0] - 0.4, frame[1] + 0.4, frame[2] - 0.15, frame[3] + 0.15);

  new RenderRelation(get_state_def(S_ready), geom);
  NodeRelation *down = new RenderRelation(get_state_def(S_depressed), geom);
  new RenderRelation(get_state_def(S_rollover), geom);
  NodeRelation *inact = new RenderRelation(get_state_def(S_inactive), geom);

  PGFrameStyle style;
  style.set_color(0.8, 0.8, 0.8, 1.0);
  style.set_width(0.1, 0.1);

  style.set_type(PGFrameStyle::T_bevel_out);
  set_frame_style(S_ready, style);

  style.set_color(0.9, 0.9, 0.9, 1.0);
  set_frame_style(S_rollover, style);

  ColorTransition *ct = new ColorTransition(Colorf(0.8, 0.8, 0.8, 1.0));
  inact->set_transition(ct);
  style.set_color(0.6, 0.6, 0.6, 1.0);
  set_frame_style(S_inactive, style);

  style.set_type(PGFrameStyle::T_bevel_in);
  style.set_color(0.8, 0.8, 0.8, 1.0);
  set_frame_style(S_depressed, style);
  LMatrix4f translate = LMatrix4f::translate_mat(0.05, 0.0, -0.05);
  TransformTransition *tt = new TransformTransition(translate);
  down->set_transition(tt);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::setup
//       Access: Published
//  Description: Sets up the button using the indicated NodePath as
//               arbitrary geometry.
////////////////////////////////////////////////////////////////////
void PGButton::
setup(const ArcChain &ready, const ArcChain &depressed, 
      const ArcChain &rollover, const ArcChain &inactive) {
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
//     Function: PGButton::set_active
//       Access: Published, Virtual
//  Description: Toggles the active/inactive state of the button.  In
//               the case of a PGButton, this also changes its visual
//               appearance.
////////////////////////////////////////////////////////////////////
void PGButton:: 
set_active(bool active) {
  if (active != get_active()) {
    PGItem::set_active(active);
    set_state(active ? S_ready : S_inactive);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::add_click_button
//       Access: Published
//  Description: Adds the indicated button to the set of buttons that
//               can effectively "click" the PGButton.  Normally, this
//               is just MouseButton::one().  Returns true if the
//               button was added, or false if it was already there.
////////////////////////////////////////////////////////////////////
bool PGButton::
add_click_button(const ButtonHandle &button) {
  return _click_buttons.insert(button).second;
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::remove_click_button
//       Access: Published
//  Description: Removes the indicated button from the set of buttons
//               that can effectively "click" the PGButton.  Normally,
//               this is just MouseButton::one().  Returns true if the
//               button was removed, or false if it was not in the
//               set.
////////////////////////////////////////////////////////////////////
bool PGButton::
remove_click_button(const ButtonHandle &button) {
  return (_click_buttons.erase(button) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::has_click_button
//       Access: Published
//  Description: Returns true if the indicated button is on the set of
//               buttons that can effectively "click" the PGButton.
//               Normally, this is just MouseButton::one().
////////////////////////////////////////////////////////////////////
bool PGButton::
has_click_button(const ButtonHandle &button) {
  return (_click_buttons.count(button) != 0);
}
