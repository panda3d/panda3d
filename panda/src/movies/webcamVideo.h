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
  WebcamVideo(const string &dev, int x=640, int y=480, int fps=24);
  virtual ~WebcamVideo();

  static int    get_num_devices();
  static string get_device_name(int n);
  
  virtual PT(MovieVideoCursor) open();
  
 private:
  string _specified_device;
  int    _specified_x;
  int    _specified_y;
  int    _specified_fps;
  friend class WebcamVideoCursor;
  
public:
  static void init_cursor_type();

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
