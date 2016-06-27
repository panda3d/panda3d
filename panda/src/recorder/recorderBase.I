/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file recorderBase.I
 * @author drose
 * @date 2004-01-24
 */

/**
 * Returns true if this recorder is presently recording data for saving to a
 * session file, false otherwise.  If this is true, record_data() will be
 * called from time to time.
 */
INLINE bool RecorderBase::
is_recording() const {
  return (_flags & F_recording) != 0;
}

/**
 * Returns true if this recorder is presently playing back data from session
 * file, false otherwise.  If this is true, play_data() will be called from
 * time to time.
 */
INLINE bool RecorderBase::
is_playing() const {
  return (_flags & F_playing) != 0;
}
