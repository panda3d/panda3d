/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texProjectorEffect.I
 * @author drose
 * @date 2004-07-25
 */

/**
 * Use TexProjectorEffect::make() to construct a new TexProjectorEffect
 * object.
 */
INLINE TexProjectorEffect::
TexProjectorEffect() {
}

/**
 * Use TexProjectorEffect::make() to construct a new TexProjectorEffect
 * object.
 */
INLINE TexProjectorEffect::
TexProjectorEffect(const TexProjectorEffect &copy) :
  _stages(copy._stages)
{
}

/**
 *
 */
INLINE TexProjectorEffect::StageDef::
StageDef() :
  _to_lens_node(nullptr)
{
}

/**
 *
 */
INLINE void TexProjectorEffect::StageDef::
set_from(const NodePath &from) {
  _from = from;
}

/**
 *
 */
INLINE void TexProjectorEffect::StageDef::
set_lens_index(int lens_index) {
  _lens_index = lens_index;
}

/**
 *
 */
INLINE int TexProjectorEffect::StageDef::
compare_to(const TexProjectorEffect::StageDef &other) const {
  int compare = _to.compare_to(other._to);
  if (compare != 0) {
    return compare;
  }

  if (_lens_index != other._lens_index) {
    return _lens_index - other._lens_index;
  }

  return _from.compare_to(other._from);
}
