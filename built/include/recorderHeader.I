/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file recorderHeader.I
 * @author drose
 * @date 2004-01-29
 */

/**
 *
 */
INLINE RecorderHeader::
RecorderHeader() {
  time(&_start_time);
  _random_seed = (int)_start_time;
}

/**
 *
 */
INLINE RecorderHeader::
RecorderHeader(const RecorderHeader &copy) :
  _start_time(copy._start_time),
  _random_seed(copy._random_seed)
{
}

/**
 *
 */
INLINE void RecorderHeader::
operator = (const RecorderHeader &copy) {
  _start_time = copy._start_time;
  _random_seed = copy._random_seed;
}

/**
 *
 */
INLINE RecorderHeader::
~RecorderHeader() {
}
