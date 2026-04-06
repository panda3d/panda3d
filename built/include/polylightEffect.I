/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file polylightEffect.I
 * @author sshodhan
 * @date 2004-06-02
 */

/**
 * Use PolylightEffect::make() to construct a new PolylightEffect object.
 */
INLINE PolylightEffect::
PolylightEffect() {
}

/**
 * Copy Constructor used by the const methods to modify data on a copy and
 * return a new one
 */
INLINE PolylightEffect::
PolylightEffect(const PolylightEffect &copy) :
  _contribution_type(copy._contribution_type),
  _weight(copy._weight),
  _lightgroup(copy._lightgroup),
  _effect_center(copy._effect_center)
{
}

/**
 * Get the weight value
 */
INLINE PN_stdfloat PolylightEffect::
get_weight() const {
  return _weight;
}

/**
 * Returns CT_all or CT_proximal
 */
INLINE PolylightEffect::ContribType PolylightEffect::
get_contrib() const {
  return _contribution_type;
}

/**
 * Return the value of the _effect_center
 */
INLINE LPoint3 PolylightEffect::
get_effect_center() const {
  return _effect_center;
}
