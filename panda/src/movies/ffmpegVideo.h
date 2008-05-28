// Filename: ffmpegVideo.h
// Created by: jyelon (01Aug2007)
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

#ifndef FFMPEGVIDEO_H
#define FFMPEGVIDEO_H
#ifdef HAVE_FFMPEG

#include "movieVideo.h"

class FfmpegVideoCursor;

////////////////////////////////////////////////////////////////////
//       Class : FfmpegVideo
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES FfmpegVideo : public MovieVideo {

 PUBLISHED:
  FfmpegVideo(const Filename &name);
  virtual ~FfmpegVideo();
  virtual PT(MovieVideoCursor) open();
  
 private:
  friend class FfmpegVideoCursor;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieVideo::init_type();
    register_type(_type_handle, "FfmpegVideo",
                  MovieVideo::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "ffmpegVideo.I"

#endif // HAVE_FFMPEG
#endif // FFMPEGVIDEO_H
