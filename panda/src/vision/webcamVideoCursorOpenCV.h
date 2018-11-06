/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webcamVideoCursorOpenCV.h
 * @author drose
 * @date 2010-10-20
 */

#ifndef WEBCAMVIDEOCURSOROPENCV_H
#define WEBCAMVIDEOCURSOROPENCV_H

#include "pandabase.h"

#ifdef HAVE_OPENCV

#include "webcamVideo.h"
#include "movieVideoCursor.h"

class WebcamVideoOpenCV;
struct CvCapture;

/**
 * The Video4Linux implementation of webcams.
 */
class WebcamVideoCursorOpenCV : public MovieVideoCursor {
public:
  WebcamVideoCursorOpenCV(WebcamVideoOpenCV *src);
  virtual ~WebcamVideoCursorOpenCV();
  virtual PT(Buffer) fetch_buffer();

private:
  bool get_frame_data(const unsigned char *&r,
                      const unsigned char *&g,
                      const unsigned char *&b,
                      int &x_pitch, int &y_pitch);

  CvCapture *_capture;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieVideoCursor::init_type();
    register_type(_type_handle, "WebcamVideoCursorOpenCV",
                  MovieVideoCursor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // HAVE_OPENCV

#endif
