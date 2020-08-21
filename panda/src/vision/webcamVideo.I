/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webcamVideo.I
 * @author jyelon
 * @date 2007-11-01
 */

/**
 * Returns the camera's size_x.
 */
INLINE int WebcamVideo::
get_size_x() const {
  return _size_x;
}

/**
 * Returns the camera's size_y.
 */
INLINE int WebcamVideo::
get_size_y() const {
  return _size_y;
}

/**
 * Returns the camera's framerate.  This is a maximum theoretical: the actual
 * performance will depend on the speed of the hardware.
 */
INLINE double WebcamVideo::
get_fps() const {
  return _fps;
}

/**
 * Returns the camera's pixel format, as a FourCC code, if known.
 */
INLINE const std::string &WebcamVideo::
get_pixel_format() const {
  return _pixel_format;
}

/**
 * Outputs the WebcamVideo.  This function simply writes the name, size and
 * FPS to the output stream.
 */
INLINE void WebcamVideo::
output(std::ostream &out) const {
  out << get_name() << ": " << get_size_x() << "x" << get_size_y();

  if (!_pixel_format.empty()) {
    out << " " << _pixel_format;
  }

  out << " @ " << get_fps() << "Hz";
}

INLINE std::ostream &operator << (std::ostream &out, const WebcamVideo &n) {
  n.output(out);
  return out;
}
