/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureStageCollection.I
 * @author drose
 * @date 2004-07-23
 */

/**
 *
 */
INLINE TextureStageCollection::
~TextureStageCollection() {
}

/**
 * Appends the other list onto the end of this one.
 */
INLINE void TextureStageCollection::
operator += (const TextureStageCollection &other) {
  add_texture_stages_from(other);
}

/**
 * Returns a TextureStageCollection representing the concatenation of the two
 * lists.
 */
INLINE TextureStageCollection TextureStageCollection::
operator + (const TextureStageCollection &other) const {
  TextureStageCollection a(*this);
  a += other;
  return a;
}

/**
 * This STL function object is used to sort a list of texture stages in order
 * by sort.
 */
INLINE bool TextureStageCollection::CompareTextureStageSort::
operator () (const TextureStage *a, const TextureStage *b) const {
  return a->get_sort() < b->get_sort();
}
