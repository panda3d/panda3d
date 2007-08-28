// Filename: ffmpegVideo.h
// Created by: jyelon (01Aug2007)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
