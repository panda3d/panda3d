/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureCollection.I
 * @author drose
 * @date 2002-03-16
 */

/**
 *
 */
INLINE TextureCollection::
~TextureCollection() {
}

/**
 * Appends the other list onto the end of this one.
 */
INLINE void TextureCollection::
operator += (const TextureCollection &other) {
  add_textures_from(other);
}

/**
 * Returns a TextureCollection representing the concatenation of the two
 * lists.
 */
INLINE TextureCollection TextureCollection::
operator + (const TextureCollection &other) const {
  TextureCollection a(*this);
  a += other;
  return a;
}

/**
 * Adds a new Texture to the collection.  This method duplicates the
 * add_texture() method; it is provided to satisfy Python's naming convention.
 */
void TextureCollection::
append(Texture *texture) {
  add_texture(texture);
}

/**
 * Appends the other list onto the end of this one.  This method duplicates
 * the += operator; it is provided to satisfy Python's naming convention.
 */
INLINE void TextureCollection::
extend(const TextureCollection &other) {
  operator += (other);
}
