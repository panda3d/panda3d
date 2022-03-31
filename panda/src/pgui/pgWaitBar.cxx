/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgWaitBar.cxx
 * @author drose
 * @date 2002-03-14
 */

#include "pgWaitBar.h"
#include "pgMouseWatcherParameter.h"

#include "throw_event.h"

TypeHandle PGWaitBar::_type_handle;

/**
 *
 */
PGWaitBar::
PGWaitBar(const std::string &name) : PGItem(name)
{
  set_cull_callback();

  _range = 100.0;
  _value = 0.0;
  _bar_state = -1;
}

/**
 *
 */
PGWaitBar::
~PGWaitBar() {
}

/**
 *
 */
PGWaitBar::
PGWaitBar(const PGWaitBar &copy) :
  PGItem(copy),
  _range(copy._range),
  _value(copy._value)
{
  _bar_state = -1;
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *PGWaitBar::
make_copy() const {
  LightReMutexHolder holder(_lock);
  return new PGWaitBar(*this);
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
bool PGWaitBar::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  LightReMutexHolder holder(_lock);
  update();
  return PGItem::cull_callback(trav, data);
}

/**
 * Creates a PGWaitBar with the indicated dimensions, with the indicated
 * maximum range.
 */
void PGWaitBar::
setup(PN_stdfloat width, PN_stdfloat height, PN_stdfloat range) {
  LightReMutexHolder holder(_lock);
  set_state(0);
  clear_state_def(0);

  set_frame(-0.5f * width, 0.5f * width, -0.5f * height, 0.5f * height);

  PN_stdfloat bevel = 0.05f;

  PGFrameStyle style;
  style.set_width(bevel, bevel);

  style.set_color(0.6f, 0.6f, 0.6f, 1.0f);
  style.set_type(PGFrameStyle::T_bevel_in);
  set_frame_style(0, style);

  style.set_color(0.8f, 0.8f, 0.8f, 1.0f);
  style.set_type(PGFrameStyle::T_bevel_out);
  set_bar_style(style);
}

/**
 * Computes the appropriate size of the bar frame according to the percentage
 * completed.
 */
void PGWaitBar::
update() {
  LightReMutexHolder holder(_lock);
  int state = get_state();

  // If the bar was last drawn in this state and is still current, we don't
  // have to draw it again.
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
    const LVecBase4 &frame = get_frame();
    const LVecBase2 &width = style.get_width();

    // Put the bar within the item's frame's border.
    LVecBase4 bar_frame(frame[0] + width[0],
                         frame[1] - width[0],
                         frame[2] + width[1],
                         frame[3] - width[1]);

    // And scale the bar according to our value.
    PN_stdfloat frac = _value / _range;
    frac = std::max(std::min(frac, (PN_stdfloat)1.0), (PN_stdfloat)0.0);
    bar_frame[1] = bar_frame[0] + frac * (bar_frame[1] - bar_frame[0]);

    _bar = _bar_style.generate_into(root, bar_frame, 1);
  }

  // Indicate that the bar is current for this state.
  _bar_state = state;
}
