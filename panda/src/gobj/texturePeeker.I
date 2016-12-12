/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texturePeeker.I
 * @author drose
 * @date 2008-08-26
 */

/**
 * Returns true if the TexturePeeker was able to initialize itself and is
 * ready to return texel colors.
 */
INLINE bool TexturePeeker::
is_valid() const {
  return !_image.is_null();
}

/**
 * Returns the width of the texture image that is contributing to the
 * TexturePeeker's information.  This may be either the Texture's full width,
 * or its simple ram image's width.
 */
INLINE int TexturePeeker::
get_x_size() const {
  return _x_size;
}

/**
 * Returns the height of the texture image that is contributing to the
 * TexturePeeker's information.  This may be either the Texture's full height,
 * or its simple ram image's height.
 */
INLINE int TexturePeeker::
get_y_size() const {
  return _y_size;
}

/**
 * Returns the depth of the texture image that is contributing to the
 * TexturePeeker's information.
 */
INLINE int TexturePeeker::
get_z_size() const {
  return _z_size;
}

/**
 * Returns whether a given coordinate is inside of the texture dimensions.
 */
INLINE bool TexturePeeker::
has_pixel(int x, int y) const {
  return x >= 0 && y >= 0 && x < _x_size && y < _y_size;
}
