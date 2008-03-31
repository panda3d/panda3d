// Filename: config_movies.cxx
// Created by:  jyelon (02Jul07)
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

#include "config_movies.h"
#include "dconfig.h"
#include "webcamVideo.h"

#ifdef HAVE_FFMPEG
#include "avcodec.h"
#endif

ConfigureDef(config_movies);
NotifyCategoryDef(movies, "");

ConfigureFn(config_movies) {
  init_libmovies();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libmovies
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libmovies() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  MovieVideo::init_type();
  MovieVideoCursor::init_type();
  MovieAudio::init_type();
  MovieAudioCursor::init_type();
  InkblotVideo::init_type();
  InkblotVideoCursor::init_type();
  UserDataAudio::init_type();
  UserDataAudioCursor::init_type();
  WebcamVideo::init_type();
#ifdef HAVE_FFMPEG
  FfmpegVideo::init_type();
  FfmpegVideoCursor::init_type();
  FfmpegAudio::init_type();
  FfmpegAudioCursor::init_type();
  av_register_all();
  FfmpegVirtualFile::register_protocol();
#endif
}

