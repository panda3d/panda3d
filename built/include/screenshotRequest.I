/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file screenshotRequest.I
 * @author rdb
 * @date 2022-12-26
 */

/**
 *
 */
INLINE ScreenshotRequest::
ScreenshotRequest(Texture *tex) :
  _frame_number(ClockObject::get_global_clock()->get_frame_count()) {
  _result = tex;
  _result_ref = tex;
}


/**
 * Returns the frame number in which the request originated.
 */
INLINE int ScreenshotRequest::
get_frame_number() const {
  return _frame_number;
}

/**
 * Returns the resulting texture.  Can always be called.
 */
INLINE Texture *ScreenshotRequest::
get_result() const {
  return (Texture *)_result;
}
