/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file userVertexSlider.I
 * @author drose
 * @date 2005-03-28
 */

/**
 * Stores the indicated slider value.
 */
INLINE void UserVertexSlider::
set_slider(PN_stdfloat slider) {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);
  cdata->_slider = slider;
  mark_modified(current_thread);
}

/**
 *
 */
INLINE UserVertexSlider::CData::
CData() :
  _slider(0.0f)
{
}

/**
 *
 */
INLINE UserVertexSlider::CData::
CData(const UserVertexSlider::CData &copy) :
  _slider(copy._slider)
{
}
