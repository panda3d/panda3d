/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physicsObjectCollection.I
 * @author joswilso
 * @date 2006-07-12
 */

/**
 *
 */
INLINE PhysicsObjectCollection::
~PhysicsObjectCollection() {
}

/**
 * Appends the other list onto the end of this one.
 */
INLINE void PhysicsObjectCollection::
operator += (const PhysicsObjectCollection &other) {
  add_physics_objects_from(other);
}

/**
 * Returns a PhysicsObjectCollection representing the concatenation of the two
 * lists.
 */
INLINE PhysicsObjectCollection PhysicsObjectCollection::
operator + (const PhysicsObjectCollection &other) const {
  PhysicsObjectCollection a(*this);
  a += other;
  return a;
}
