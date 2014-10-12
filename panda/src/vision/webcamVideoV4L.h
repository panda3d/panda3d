// Filename: webcamVideoV4L.h
// Created by: rdb (11Jun2010)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef WEBCAMVIDEOV4L_H
#define WEBCAMVIDEOV4L_H

#include "pandabase.h"

#ifdef HAVE_VIDEO4LINUX

#include "webcamVideo.h"

class WebcamVideoCursorV4L;

////////////////////////////////////////////////////////////////////
//       Class : WebcamVideoV4L
// Description : The Video4Linux implementation of webcams.
////////////////////////////////////////////////////////////////////
class WebcamVideoV4L : public WebcamVideo {
private:
  virtual PT(MovieVideoCursor) open();

  friend class WebcamVideoCursorV4L;
  friend void find_all_webcams_v4l();
  static void add_options_for_size(int fd, const string &dev, const char *name,
                                   unsigned width, unsigned height,
                                   unsigned pixelformat);

  string _device;
  uint32_t _pformat;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    WebcamVideo::init_type();
    register_type(_type_handle, "WebcamVideoV4L",
                  WebcamVideo::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // HAVE_VIDEO4LINUX

#endif
