/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTaskSequence.I
 * @author drose
 * @date 2008-10-04
 */

/**
 * Sets the repeat count of the sequence.  If the count is 0 or 1, the
 * sequence will run exactly once.  If it is greater than 0, it will run that
 * number of times.  If it is negative, it will run forever until it is
 * explicitly removed.
 */
INLINE void AsyncTaskSequence::
set_repeat_count(int repeat_count) {
  _repeat_count = repeat_count;
}

/**
 * Returns the repeat count of the sequence.  See set_repeat_count().
 */
INLINE int AsyncTaskSequence::
get_repeat_count() const {
  return _repeat_count;
}

/**
 * Returns the index of the task within the sequence that is currently being
 * executed (or that will be executed at the next epoch).
 */
INLINE size_t AsyncTaskSequence::
get_current_task_index() const {
  return _task_index;
}
