/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderEffect.I
 * @author drose
 * @date 2002-03-14
 */

/**
 * Provides an arbitrary ordering among all unique RenderEffects, so we can
 * store the essentially different ones in a big set and throw away the rest.
 *
 * This method is not needed outside of the RenderEffect class because all
 * equivalent RenderEffect objects are guaranteed to share the same pointer;
 * thus, a pointer comparison is always sufficient.
 */
INLINE int RenderEffect::
compare_to(const RenderEffect &other) const {
  // First, we compare the types; if they are of different types then they
  // sort differently.
  TypeHandle type = get_type();
  TypeHandle other_type = other.get_type();
  if (type != other_type) {
    return type.get_index() - other_type.get_index();
  }

  // We only call compare_to_impl() if they have the same type.
  return compare_to_impl(&other);
}
