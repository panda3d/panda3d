/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webcamVideoOpenCV.h
 * @author drose
 * @date 2010-10-20
 */

#ifndef WEBCAMVIDEOOPENCV_H
#define WEBCAMVIDEOOPENCV_H

#include "pandabase.h"

#ifdef HAVE_OPENCV

#include "webcamVideo.h"

class WebcamVideoCursorOpenCV;

/**
 * The OpenCV implementation of webcams.  Probably won't be needed once we
 * have a native webcam implementation for each Panda3D-supported platform.
 * (So far, we're 2 for 3.)
 */
class WebcamVideoOpenCV : public WebcamVideo {
private:
  WebcamVideoOpenCV(int camera_index);
  virtual PT(MovieVideoCursor) open();

private:
  int _camera_index;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WebcamVideo::init_type();
    register_type(_type_handle, "WebcamVideoOpenCV",
                  WebcamVideo::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class WebcamVideoCursorOpenCV;
  friend void find_all_webcams_opencv();
};

#endif // HAVE_OPENCV

#endif
