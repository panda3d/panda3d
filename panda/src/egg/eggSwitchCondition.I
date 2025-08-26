/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggSwitchCondition.I
 * @author Derzsi Daniel
 * @date 2021-01-01
 */

/**
 * Checks if these two switch conditions are equivalent.
 * If so, returns true.
 */
bool EggSwitchCondition::
operator == (const EggSwitchCondition &other) const {
  // These are equivalent only if both of them are castable to distance
  // conditions and if both of these distance conditions are equivalent.

  // Right now, only distance switch conditions exist.
  const EggSwitchConditionDistance *distance = as_distance();
  const EggSwitchConditionDistance *other_distance = other.as_distance();

  return distance != nullptr && other_distance != nullptr
    && (*distance) == (*other_distance);
}

/**
 * Checks if these two switch conditions are NOT equivalent. If so, returns
 * true.
 */
bool EggSwitchCondition::
operator != (const EggSwitchCondition &other) const {
  return !operator==(other);
}

/**
 * Checks if these two distance switch conditions are equivalent. If so, returns
 * true.
 */
bool EggSwitchConditionDistance::
operator == (const EggSwitchConditionDistance &other) const {
  return _switch_in == other._switch_in && _switch_out == other._switch_out
    && _center.almost_equal(other._center) && _fade == other._fade;
}

/**
 * Checks if these two distance switch conditions are NOT equivalent. If so,
 * returns true.
 */
bool EggSwitchConditionDistance::
operator != (const EggSwitchConditionDistance &other) const {
  return !operator==(other);
}
