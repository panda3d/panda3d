// Filename: pgWaitBar.cxx
// Created by:  drose (12Jul01)
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

#include "pgWaitBar.h"
#include "pgMouseWatcherParameter.h"

#include "throw_event.h"

TypeHandle PGWaitBar::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PGWaitBar::
PGWaitBar(const string &name) : PGItem(name)
{
  _range = 100.0;
  _value = 0.0;
  _bar_state = -1;
  _bar_arc = (NodeRelation *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGWaitBar::
~PGWaitBar() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PGWaitBar::
PGWaitBar(const PGWaitBar &copy) :
  PGItem(copy),
  _range(copy._range),
  _value(copy._value)
{
  _bar_state = -1;
  _bar_arc = (NodeRelation *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PGWaitBar::
operator = (const PGWaitBar &copy) {
  PGItem::operator = (copy);
  _range = copy._range;
  _value = copy._value;

  _bar_state = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *PGWaitBar::
make_copy() const {
  return new PGWaitBar(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::draw_item
//       Access: Public, Virtual
//  Description: Called by the PGTop's traversal to draw this
//               particular item.
////////////////////////////////////////////////////////////////////
void PGWaitBar::
draw_item(PGTop *top, GraphicsStateGuardian *gsg, 
          const AllAttributesWrapper &attrib) {
  update();
  PGItem::draw_item(top, gsg, attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::setup
//       Access: Public
//  Description: Creates a PGWaitBar with the indicated dimensions,
//               with the indicated maximum range.
////////////////////////////////////////////////////////////////////
void PGWaitBar::
setup(float width, float height, float range) {
  set_state(0);
  clear_state_def(0);

  set_frame(-0.5 * width, 0.5 * width, -0.5 * height, 0.5 * height);

  PGFrameStyle style;
  style.set_width(0.05, 0.05);

  style.set_color(0.6, 0.6, 0.6, 1.0);
  style.set_type(PGFrameStyle::T_bevel_in);
  set_frame_style(0, style);

  style.set_color(0.8, 0.8, 0.8, 1.0);
  style.set_type(PGFrameStyle::T_bevel_out);
  set_bar_style(style);
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::update
//       Access: Private
//  Description: Computes the appropriate size of the bar frame
//               according to the percentage completed.
////////////////////////////////////////////////////////////////////
void PGWaitBar:: 
update() {
  int state = get_state();

  // If the bar was last drawn in this state and is still current, we
  // don't have to draw it again.
  if (_bar_state == state) {
    return;
  }

  // Remove the old bar geometry, if any.
  if (_bar_arc != (NodeRelation *)NULL) {
    if (_bar_arc->is_attached()) {
      remove_arc(_bar_arc);
    }
    _bar_arc = (NodeRelation *)NULL;
  }

  // Now create new bar geometry.
  if (_value != 0.0 && _range != 0.0) {
    Node *node = get_state_def(state);
    nassertv(node != (Node *)NULL);

    PGFrameStyle style = get_frame_style(state);
    const LVecBase4f &frame = get_frame();
    const LVecBase2f &width = style.get_width();

    // Put the bar within the item's frame's border.
    LVecBase4f bar_frame(frame[0] + width[0],
                         frame[1] - width[0],
                         frame[2] + width[1],
                         frame[3] - width[1]);

    // And scale the bar according to our value.
    float frac = _value / _range;
    frac = max(min(frac, 1.0f), 0.0f);
    bar_frame[1] = bar_frame[0] + frac * (bar_frame[1] - bar_frame[0]);
    
    _bar_arc = _bar_style.generate_into(node, bar_frame);
  }

  // Indicate that the bar is current for this state.
  _bar_state = state;
}
