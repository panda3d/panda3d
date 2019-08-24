/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file touchTrackball.I
 * @author D. Lawrence
 * @date 2019-08-16
 */

GestureState TouchTrackball::
get_gesture_state() {
  return _gesture_state;
}