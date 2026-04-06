/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTextureCollection.I
 * @author drose
 * @date 2000-02-16
 */

/**
 *
 */
INLINE EggTextureCollection::iterator EggTextureCollection::
begin() const {
  nassertr(_ordered_textures.size() == _textures.size(),
           _ordered_textures.begin());
  return _ordered_textures.begin();
}

/**
 *
 */
INLINE EggTextureCollection::iterator EggTextureCollection::
end() const {
  return _ordered_textures.end();
}

/**
 *
 */
INLINE bool EggTextureCollection::
empty() const {
  return _ordered_textures.empty();
}

/**
 * Returns the nth EggTexture in the collection.
 */
EggTexture *EggTextureCollection::
operator [] (size_type n) const {
  return get_texture(n);
}

/**
 * Returns the number of EggTextures in the collection.
 */
INLINE EggTextureCollection::size_type EggTextureCollection::
size() const {
  nassertr(_ordered_textures.size() == _textures.size(), 0);
  return _ordered_textures.size();
}
