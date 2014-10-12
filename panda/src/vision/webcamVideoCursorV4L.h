// Filename: webcamVideoCursorV4L.h
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

#ifndef WEBCAMVIDEOCURSORV4L_H
#define WEBCAMVIDEOCURSORV4L_H

#include "pandabase.h"

#ifdef HAVE_VIDEO4LINUX

#include "webcamVideo.h"
#include "movieVideoCursor.h"

#include <linux/videodev2.h>

#ifdef HAVE_JPEG
extern "C" {
  #include <jpeglib.h>
}
#endif

class WebcamVideoV4L;

////////////////////////////////////////////////////////////////////
//       Class : WebcamVideoCursorV4L
// Description : The Video4Linux implementation of webcams.
////////////////////////////////////////////////////////////////////
class WebcamVideoCursorV4L : public MovieVideoCursor {
public:
  WebcamVideoCursorV4L(WebcamVideoV4L *src);
  virtual ~WebcamVideoCursorV4L();
  virtual PT(Buffer) fetch_buffer();

private:
  int _fd;
  void **_buffers;
  size_t *_buflens;
  size_t _bufcount;
  struct v4l2_format _format;
#ifdef HAVE_JPEG
  struct jpeg_decompress_struct _cinfo;
#endif

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieVideoCursor::init_type();
    register_type(_type_handle, "WebcamVideoCursorV4L",
                  MovieVideoCursor::get_class_type());
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
