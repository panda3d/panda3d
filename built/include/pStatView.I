/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatView.I
 * @author drose
 * @date 2000-07-12
 */

/**
 * Returns the current PStatThreadData associated with the view.  This was set
 * by a previous call to set_thread_data().
 */
INLINE const PStatThreadData *PStatView::
get_thread_data() {
  return _thread_data;
}

/**
 * Returns the current PStatClientData associated with the view.  This was
 * also set by a previous call to set_thread_data().
 */
INLINE const PStatClientData *PStatView::
get_client_data() {
  return _client_data;
}


/**
 * Sets to a particular frame number (or the nearest available), extracted
 * from the View's PStatThreadData pointer.  See the comments in the other
 * flavor of set_to_frame().
 */
INLINE void PStatView::
set_to_frame(int frame_number) {
  set_to_frame(_thread_data->get_frame(frame_number));
}

/**
 * Sets to the frame that occurred at the indicated time (or the nearest
 * available frame), extracted from the View's PStatThreadData pointer.  See
 * the comments in set_to_frame.
 */
INLINE void PStatView::
set_to_time(double time) {
  set_to_frame(_thread_data->get_frame_at_time(time));
}

/**
 * Returns true if we are showing level data, false if time data.
 */
INLINE bool PStatView::
get_show_level() const {
  return _show_level;
}

/**
 * Returns an index number that can be used to determine when the set of known
 * levels has changed.  Each time the set of levels in the view changes
 * (because of new data arriving from the client, for instance), this number
 * is incremented.
 */
INLINE int PStatView::
get_level_index() const {
  return _level_index;
}
