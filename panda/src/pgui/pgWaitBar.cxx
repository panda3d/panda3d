// Filename: pgWaitBar.cxx
// Created by:  drose (14Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
PGWaitBar::
PGWaitBar(const PGWaitBar &copy) :
  PGItem(copy),
  _range(copy._range),
  _value(copy._value)
{
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
PandaNode *PGWaitBar::
make_copy() const {
  return new PGWaitBar(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::has_cull_callback
//       Access: Protected, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool PGWaitBar::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::cull_callback
//       Access: Protected, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
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
bool PGWaitBar::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  update();
  return PGItem::cull_callback(trav, data);
}

////////////////////////////////////////////////////////////////////
//     Function: PGWaitBar::setup
//       Access: Published
//  Description: Creates a PGWaitBar with the indicated dimensions,
//               with the indicated maximum range.
////////////////////////////////////////////////////////////////////
void PGWaitBar::
setup(float width, float height, float range) {
  set_state(0);
  clear_state_def(0);

  set_frame(-0.5f * width, 0.5f * width, -0.5f * height, 0.5f * height);

  float bevel = 0.05f;

  PGFrameStyle style;
  style.set_width(bevel, bevel);

  style.set_color(0.6f, 0.6f, 0.6f, 1.0f);
  style.set_type(PGFrameStyle::T_bevel_in);
  set_frame_style(0, style);

  style.set_color(0.8f, 0.8f, 0.8f, 1.0f);
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
  _bar.remove_node();

  // Now create new bar geometry.
  if ((_value != 0.0f) && (_range != 0.0f)) {
    NodePath &root = get_state_def(state);
    nassertv(!root.is_empty());

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
    
    _bar = _bar_style.generate_into(root, bar_frame, 1);

    // Make sure the bar is rendered after the frame.
    _bar.set_bin("fixed", 1);
  }

  // Indicate that the bar is current for this state.
  _bar_state = state;
}
