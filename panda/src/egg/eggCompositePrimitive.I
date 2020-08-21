/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCompositePrimitive.I
 * @author drose
 * @date 2005-03-13
 */

/**
 *
 */
INLINE EggCompositePrimitive::
EggCompositePrimitive(const std::string &name) : EggPrimitive(name) {
}

/**
 *
 */
INLINE EggCompositePrimitive::
EggCompositePrimitive(const EggCompositePrimitive &copy) : EggPrimitive(copy) {
}

/**
 *
 */
INLINE EggCompositePrimitive &EggCompositePrimitive::
operator = (const EggCompositePrimitive &copy) {
  EggPrimitive::operator = (copy);
  return *this;
}

/**
 * Returns the number of individual component triangles within the composite.
 * Each one of these might have a different set of attributes.
 */
INLINE size_t EggCompositePrimitive::
get_num_components() const {
  return _components.size();
}

/**
 * Returns the attributes for the nth component triangle.
 */
INLINE const EggAttributes *EggCompositePrimitive::
get_component(size_t i) const {
  nassertr(i < _components.size(), nullptr);
  return _components[i];
}

/**
 * Returns the attributes for the nth component triangle.
 */
INLINE EggAttributes *EggCompositePrimitive::
get_component(size_t i) {
  nassertr(i < _components.size(), nullptr);
  return _components[i];
}

/**
 * Changes the attributes for the nth component triangle.
 */
INLINE void EggCompositePrimitive::
set_component(size_t i, const EggAttributes *attrib) {
  nassertv(i < _components.size());
  _components[i] = new EggAttributes(*attrib);
}

/**
 * Subdivides the composite primitive into triangles and adds those triangles
 * to the indicated container.  Does not remove the primitive from its
 * existing parent or modify it in any way.
 *
 * Returns true if the triangulation is successful, or false if there was some
 * error (in which case the container may contain some partial triangulation).
 */
INLINE bool EggCompositePrimitive::
triangulate_into(EggGroupNode *container) const {
  return do_triangulate(container);
}
