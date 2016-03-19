/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file recorderFrame.I
 * @author drose
 * @date 2004-01-28
 */

/**
 *
 */
INLINE RecorderFrame::
RecorderFrame() {
}

/**
 *
 */
INLINE RecorderFrame::
RecorderFrame(double timestamp, int frame,
              bool table_changed, RecorderTable *table) :
  _timestamp(timestamp),
  _frame(frame),
  _table_changed(table_changed),
  _table(table)
{
}

/**
 *
 */
INLINE RecorderFrame::
~RecorderFrame() {
}
