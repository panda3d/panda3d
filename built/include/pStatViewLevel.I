/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatViewLevel.I
 * @author drose
 * @date 2000-07-19
 */

/**
 * Returns the Collector index associated with this level.
 */
INLINE int PStatViewLevel::
get_collector() const {
  return _collector;
}

/**
 * Returns the total level value (or elapsed time value) for this Collector,
 * not including any values accounted for by its child Collectors.
 */
INLINE double PStatViewLevel::
get_value_alone() const {
  return _value_alone;
}

/**
 * Returns the number of start/stop pairs for this collector.
 */
INLINE int PStatViewLevel::
get_count() const {
  return _count;
}
