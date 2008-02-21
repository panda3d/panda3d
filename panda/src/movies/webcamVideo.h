// Filename: webcamVideo.h
// Created by: jyelon (01Nov2007)
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

#ifndef WEBCAMVIDEO_H
#define WEBCAMVIDEO_H

#include "movieVideo.h"

////////////////////////////////////////////////////////////////////
//       Class : WebcamVideo
// Description : Allows you to open a webcam or other video capture
//               device as a video stream.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES WebcamVideo : public MovieVideo {

PUBLISHED:
  virtual ~WebcamVideo();

  static int             get_num_options();
  static PT(WebcamVideo) get_option(int n);
  
  INLINE int get_size_x() const;
  INLINE int get_size_y() const;
  INLINE int get_fps() const;
  
  virtual PT(MovieVideoCursor) open() = 0;

public:
  static void find_all_webcams();

protected:
  int _size_x;
  int _size_y;
  int _fps;

  static pvector<PT(WebcamVideo)> _all_webcams;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieVideo::init_type();
    register_type(_type_handle, "WebcamVideo",
                  MovieVideo::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "webcamVideo.I"

#endif
