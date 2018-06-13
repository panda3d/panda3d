/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texturePoolFilter.cxx
 * @author drose
 * @date 2006-07-27
 */

#include "texturePoolFilter.h"

TypeHandle TexturePoolFilter::_type_handle;

/**
 *
 */
TexturePoolFilter::
~TexturePoolFilter() {
}

/**
 * This method is called before each texture is loaded from disk, via the
 * TexturePool, for the first time.  If this method returns NULL, then a new
 * Texture will be allocated and loaded from disk normally by the TexturePool;
 * otherwise, if it returns non-NULL, then that returned pointer will be used
 * as the Texture for this filename.
 */
PT(Texture) TexturePoolFilter::
pre_load(const Filename &, const Filename &, int, int, bool,
         const LoaderOptions &) {
  return nullptr;
}

/**
 * This method is called after each texture has been loaded from disk, via the
 * TexturePool, for the first time.  By the time this method is called, the
 * Texture has already been fully read from disk.  This method should return
 * the Texture pointer that the TexturePool should actually return (usually it
 * is the same as the pointer supplied).
 */
PT(Texture) TexturePoolFilter::
post_load(Texture *tex) {
  return tex;
}

/**
 *
 */
void TexturePoolFilter::
output(std::ostream &out) const {
  out << get_type();
}
