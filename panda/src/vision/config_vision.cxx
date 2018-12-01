/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_vision.cxx
 * @author rdb
 * @date 2009-11-07
 */

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

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_VISION)
  #error Buildsystem error: BUILDING_VISION not defined
#endif

Configure(config_vision);
NotifyCategoryDef(vision, "");

ConfigVariableBool v4l_blocking
("v4l-blocking", false,
 PRC_DESC("Set this to true if you want to block waiting for webcam frames."));

ConfigureFn(config_vision) {
  init_libvision();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
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

#ifndef HAVE_FFMPEG
  TexturePool *ts = TexturePool::get_global_ptr();
  ts->register_texture_type(OpenCVTexture::make_texture, "avi");
#endif
#endif
}
