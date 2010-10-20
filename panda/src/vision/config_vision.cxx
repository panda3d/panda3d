// Filename: config_vision.cxx
// Created by:  pro-rsoft (07Nov09)
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

#include "config_vision.h"
#include "openCVTexture.h"
#include "webcamVideo.h"
#include "webcamVideoCursorOpenCV.h"
#include "webcamVideoOpenCV.h"
#include "webcamVideoCursorV4L.h"
#include "webcamVideoV4L.h"

#include "pandaSystem.h"
#include "texturePool.h"
#include "dconfig.h"

Configure(config_vision);
NotifyCategoryDef(vision, "");

ConfigureFn(config_vision) {
  init_libvision();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libvision
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libvision() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  WebcamVideo::init_type();
#ifdef HAVE_VIDEO4LINUX
  WebcamVideoCursorV4L::init_type();
  WebcamVideoV4L::init_type();
#endif

#ifdef HAVE_OPENCV
  WebcamVideoCursorOpenCV::init_type();
  WebcamVideoOpenCV::init_type();

  OpenCVTexture::init_type();
  OpenCVTexture::register_with_read_factory();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("OpenCV");

  TexturePool *ts = TexturePool::get_global_ptr();
#ifndef HAVE_FFMPEG
  ts->register_texture_type(OpenCVTexture::make_texture, "avi");
#endif
#endif
}

