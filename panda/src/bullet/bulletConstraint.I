/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletConstraint.I
 * @author enn0x
 * @date 2010-03-01
 */

/**
 *
 */
INLINE BulletConstraint::
~BulletConstraint() {

}

/**
 * Sets the applied impulse limit for breaking the constraint.  If the limit
 * is exceeded the constraint will be disabled.  Disabled constraints are not
 * removed from the world, and can be re-enabled.
 */
INLINE void BulletConstraint::
set_breaking_threshold(PN_stdfloat threshold) {

  ptr()->setBreakingImpulseThreshold(threshold);
}

/**
 * Returns the applied impluse limit for breaking the constraint.
 */
INLINE PN_stdfloat BulletConstraint::
get_breaking_threshold() const {

  return (PN_stdfloat)ptr()->getBreakingImpulseThreshold();
}

/**
 *
 */
INLINE void BulletConstraint::
set_enabled(bool enable) {

  ptr()->setEnabled(enable);
}

/**
 * Returns TRUE if the constraint is enabled.
 */
INLINE bool BulletConstraint::
is_enabled() const {

  return ptr()->isEnabled();
}
