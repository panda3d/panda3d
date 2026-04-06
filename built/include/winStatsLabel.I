/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsLabel.I
 * @author rdb
 * @date 2022-01-29
 */

/**
 * Returns the x position of the label on its parent.
 */
INLINE int WinStatsLabel::
get_x() const {
  return _x;
}

/**
 * Returns the y position of the label on its parent.
 */
INLINE int WinStatsLabel::
get_y() const {
  return _y;
}

/**
 * Returns the width of the label as we requested it.
 */
INLINE int WinStatsLabel::
get_width() const {
  return _width;
}

/**
 * Returns the height of the label as we requested it.
 */
INLINE int WinStatsLabel::
get_height() const {
  return _height;
}

/**
 * Returns the width the label would really prefer to be.
 */
INLINE int WinStatsLabel::
get_ideal_width() const {
  return _ideal_width;
}

/**
 * Returns the collector this label represents.
 */
INLINE int WinStatsLabel::
get_collector_index() const {
  return _collector_index;
}

/**
 * Returns true if the visual highlight for this label is enabled.
 */
INLINE bool WinStatsLabel::
get_highlight() const {
  return _highlight;
}
