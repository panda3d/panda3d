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
#include "throw_event.h"
#include "renderRelation.h"
#include "colorTransition.h"

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
enter() {
  if (get_active()) {
    set_state(_button_down ? S_depressed : S_rollover);
  }
  PGItem::enter();
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::exit
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse exits the region.
////////////////////////////////////////////////////////////////////
void PGButton::
exit() {
  if (get_active()) {
    set_state(S_ready);
  }
  PGItem::exit();
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::button_down
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void PGButton::
button_down(ButtonHandle button) {
  if (get_active()) {
    _button_down = true;
    set_state(S_depressed);
  }
  PGItem::button_down(button);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::button_up
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button previously depressed with
//               button_down() is release.  The bool is_within flag is
//               true if the button was released while the mouse was
//               still within the region, or false if it was released
//               outside the region.
////////////////////////////////////////////////////////////////////
void PGButton::
button_up(ButtonHandle button, bool is_within) {
  _button_down = false;
  if (get_active()) {
    if (is_within) {
      set_state(S_rollover);
      click();
    } else {
      set_state(S_ready);
    }
  }
  PGItem::button_up(button, is_within);
}

////////////////////////////////////////////////////////////////////
//     Function: PGButton::click
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               button is clicked normally.
////////////////////////////////////////////////////////////////////
void PGButton::
click() {
  throw_event(get_click_event());
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
  TextNode *text_node = get_text_node();
  text_node->set_text(label);
  PT_Node geom = text_node->generate();

  new RenderRelation(get_state_def(S_ready), geom);
  NodeRelation *dep = new RenderRelation(get_state_def(S_depressed), geom);
  new RenderRelation(get_state_def(S_rollover), geom);
  new RenderRelation(get_state_def(S_inactive), geom);

  PGFrameStyle style;
  style.set_type(PGFrameStyle::T_flat);
  style.set_color(1.0, 1.0, 1.0, 1.0);
  set_frame_style(S_ready, style);

  style.set_color(0.0, 0.0, 0.0, 1.0);
  set_frame_style(S_depressed, style);
  ColorTransition *col_trans = new ColorTransition(Colorf(0.0, 1.0, 1.0, 1.0));
  dep->set_transition(col_trans);

  style.set_color(1.0, 1.0, 0.0, 1.0);
  set_frame_style(S_rollover, style);

  style.set_color(0.6, 0.6, 0.6, 1.0);
  set_frame_style(S_inactive, style);

  set_frame(text_node->get_card_actual());
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
  instance_to_state_def(S_ready, ready);
  instance_to_state_def(S_depressed, depressed);
  instance_to_state_def(S_rollover, rollover);
  instance_to_state_def(S_inactive, inactive);
}
