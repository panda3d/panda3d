/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieTexture.I
 * @author jyelon
 * @date 2007-08-01
 */

/**
 * Returns the length of the video.
 */
INLINE double MovieTexture::
get_video_length() const {
  CDReader cdata(_cycler);
  return cdata->_video_length;
}

/**
 * Returns the width in texels of the source video stream.  This is not
 * necessarily the width of the actual texture, since the texture may have
 * been expanded to raise it to a power of 2.
 */
INLINE int MovieTexture::
get_video_width() const {
  CDReader cdata(_cycler);
  return cdata->_video_width;
}

/**
 * Returns the height in texels of the source video stream.  This is not
 * necessarily the height of the actual texture, since the texture may have
 * been expanded to raise it to a power of 2.
 */
INLINE int MovieTexture::
get_video_height() const {
  CDReader cdata(_cycler);
  return cdata->_video_height;
}

/**
 * Returns the MovieVideoCursor that is feeding the color channels for the
 * indicated page, where 0 <= page < get_num_pages().
 */
INLINE MovieVideoCursor *MovieTexture::
get_color_cursor(int page) {
  CDReader cdata(_cycler);
  nassertr(page >= 0 && page < (int)cdata->_pages.size(), nullptr);
  return cdata->_pages[page]._color;
}

/**
 * Returns the MovieVideoCursor that is feeding the alpha channel for the
 * indicated page, where 0 <= page < get_num_pages().
 */
INLINE MovieVideoCursor *MovieTexture::
get_alpha_cursor(int page) {
  CDReader cdata(_cycler);
  nassertr(page >= 0 && page < (int)cdata->_pages.size(), nullptr);
  return cdata->_pages[page]._alpha;
}
