/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsPipe.I
 * @author frang
 * @date 1999-03-07
 */

/**
 * Returns false if this pipe is known to be invalid, meaning that an attempt
 * to create a GraphicsWindow with the pipe will certainly fail.  Returns true
 * if the pipe is probably valid (is this case, an attempt to create a
 * GraphicsWindow should succeed, but might still fail).
 *
 * Use the GraphicsEngine class to create a GraphicsWindow on a particular
 * pipe.
 */
INLINE bool GraphicsPipe::
is_valid() const {
  return _is_valid;
}

/**
 * Returns the mask of bits that represents the kinds of GraphicsOutput
 * objects this pipe might be able to successfully create.  The return value
 * is the union of bits in GraphicsPipe::OutputTypes that represents the set
 * of GraphicsOutput types.
 *
 * A 1 bit in a particular position is not a guarantee of success, but a 0 bit
 * is a guarantee of failure.
 */
INLINE int GraphicsPipe::
get_supported_types() const {
  return _supported_types;
}

/**
 * A convenience function to ask if a particular type or types of
 * GraphicsObjects are supported.  The parameter is a union of one or more
 * bits defined in GrpahicsPipe::OutputTypes.
 *
 * Returns true if all of the requested types are listed in the
 * supported_types mask, false if any one of them is not.  This is not a
 * guarantee that the indicated output type will successfully be created when
 * it is attempted.
 */
INLINE bool GraphicsPipe::
supports_type(int flags) const {
  return (_supported_types & flags) == flags;
}

/**
 * Returns the width of the entire display, if it is known.  This may return
 * 0.  This is not a guarantee that windows (particularly fullscreen windows)
 * may not be created larger than this width, but it is intended to provide a
 * hint to the application.
 */
INLINE int GraphicsPipe::
get_display_width() const {
  return _display_width;
}

/**
 * Returns the height of the entire display, if it is known.  This may return
 * 0.  See the caveats for get_display_width().
 */
INLINE int GraphicsPipe::
get_display_height() const {
  return _display_height;
}

/**
 * Returns a pointer to device object
 */
INLINE GraphicsDevice *GraphicsPipe::
get_device() const {
  return _device;
}
