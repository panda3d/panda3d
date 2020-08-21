/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webcamVideoOpenCV.cxx
 * @author drose
 * @date 2010-10-20
 */

#include "webcamVideoOpenCV.h"

#ifdef HAVE_OPENCV

#include "webcamVideoCursorOpenCV.h"
#include "configVariableInt.h"

TypeHandle WebcamVideoOpenCV::_type_handle;

/**
 * Finds all OpenCV webcams and adds them to the global list _all_webcams.
 */
void
find_all_webcams_opencv() {
  // OpenCV doesn't really provide a way to enumerate cameras.  We ask the
  // user to do this via a config variable.
  static ConfigVariableInt wemcam_opencv_camera_index
    ("webcam-opencv-camera-index", "0",
     PRC_DESC("Specify the space-separated list of integer camera index "
              "numbers that are assumed to be available via OpenCV to the "
              "WebcamVideo interface.  The default camera index is 0.  "
              "Specify empty string if there are no available cameras."));
  for (size_t i = 0; i < wemcam_opencv_camera_index.get_num_words(); ++i) {
    PT(WebcamVideo) wc = new WebcamVideoOpenCV(wemcam_opencv_camera_index[i]);
    WebcamVideoOpenCV::_all_webcams.push_back(wc);
  }
}

/**
 *
 */
WebcamVideoOpenCV::
WebcamVideoOpenCV(int camera_index) :
  _camera_index(camera_index)
{
  std::ostringstream strm;
  strm << "OpenCV webcam " << _camera_index;
  set_name(strm.str());
}

/**
 * Open this video, returning a MovieVideoCursor.
 */
PT(MovieVideoCursor) WebcamVideoOpenCV::
open() {
  return new WebcamVideoCursorOpenCV(this);
}

#endif
