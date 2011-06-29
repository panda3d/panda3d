// Filename: config_movies.cxx
// Created by:  jyelon (02Jul07)
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

#include "config_movies.h"
#include "dconfig.h"

#include "movieVideo.h"
#include "movieVideoCursor.h"

#include "movieAudio.h"
#include "movieAudioCursor.h"

#include "inkblotVideo.h"
#include "inkblotVideoCursor.h"

#include "ffmpegVideo.h"
#include "ffmpegVideoCursor.h"

#include "ffmpegAudio.h"
#include "ffmpegAudioCursor.h"

#ifdef HAVE_FFMPEG
extern "C" {
  #include "libavcodec/avcodec.h"
}
#endif

ConfigureDef(config_movies);
NotifyCategoryDef(movies, "");
NotifyCategoryDef(ffmpeg, "movies");

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
  MicrophoneAudio::init_type();

#ifdef HAVE_FFMPEG
  FfmpegVirtualFile::register_protocol();

  FfmpegVideo::init_type();
  FfmpegVideoCursor::init_type();
  FfmpegAudio::init_type();
  FfmpegAudioCursor::init_type();

  FfmpegVideo::register_with_read_factory();
  FfmpegVideoCursor::register_with_read_factory();
#endif
}

