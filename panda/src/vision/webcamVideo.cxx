/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webcamVideo.cxx
 * @author jyelon
 * @date 2007-11-01
 */

#include "webcamVideo.h"
#include "pandabase.h"
#include "movieVideoCursor.h"

pvector<PT(WebcamVideo)> WebcamVideo::_all_webcams;
TypeHandle WebcamVideo::_type_handle;


/**
 *
 */
WebcamVideo::
~WebcamVideo() {
}

/**
 * Scans the hardware for webcams, and pushes them onto the global list of all
 * webcams.
 *
 * There are several implementations of WebcamVideo, including one based on
 * DirectShow, one based on Video4Linux, and so forth.  These implementations
 * are contained in one C++ file each, and they export nothing at all except a
 * single "find_all" function.  Otherwise, they can only be accessed through
 * the virtual methods of the WebcamVideo objects they create.
 */
void WebcamVideo::
find_all_webcams() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

#ifdef HAVE_DIRECTCAM
  extern void find_all_webcams_ds();
  find_all_webcams_ds();
#endif

#ifdef HAVE_VIDEO4LINUX
  extern void find_all_webcams_v4l();
  find_all_webcams_v4l();
#endif

#if defined(HAVE_OPENCV) && !defined(HAVE_DIRECTCAM) && !defined(HAVE_VIDEO4LINUX)
  extern void find_all_webcams_opencv();
  find_all_webcams_opencv();
#endif
}

/**
 * Returns the number of webcam options.  An "option" consists of a device
 * plus a set of configuration parameters.  For example, "Creative Webcam Live
 * at 640x480, 30 fps" is an option.
 */
int WebcamVideo::
get_num_options() {
  find_all_webcams();
  return _all_webcams.size();
}

/**
 * Returns the nth webcam option.
 */
PT(WebcamVideo) WebcamVideo::
get_option(int n) {
  find_all_webcams();
  nassertr((n >= 0) && (n < (int)_all_webcams.size()), nullptr);
  return _all_webcams[n];
}
