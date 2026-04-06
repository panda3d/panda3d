/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file openCVTexture.I
 * @author zacpavlov
 * @date 2005-08-19
 */

/**
 * Returns true if this stream is open and ready, false otherwise.
 */
INLINE bool OpenCVTexture::VideoStream::
is_valid() const {
  return (_capture != nullptr);
}

/**
 * Returns true if this stream takes its input from a video file, false
 * otherwise.
 */
INLINE bool OpenCVTexture::VideoStream::
is_from_file() const {
  return !_filename.empty();
}

/**
 *
 */
INLINE OpenCVTexture::VideoPage::
VideoPage() {
}

/**
 *
 */
INLINE OpenCVTexture::VideoPage::
VideoPage(const OpenCVTexture::VideoPage &copy) :
  _color(copy._color),
  _alpha(copy._alpha)
{
}

/**
 *
 */
INLINE OpenCVTexture::VideoPage::
~VideoPage() {
}
