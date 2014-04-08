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
#include "inkblotVideo.h"
#include "inkblotVideoCursor.h"
#include "microphoneAudio.h"
#include "movieAudio.h"
#include "movieAudioCursor.h"
#include "movieTypeRegistry.h"
#include "movieVideo.h"
#include "movieVideoCursor.h"
#include "userDataAudio.h"
#include "userDataAudioCursor.h"
#include "vorbisAudio.h"
#include "vorbisAudioCursor.h"
#include "wavAudio.h"
#include "wavAudioCursor.h"

ConfigureDef(config_movies);
NotifyCategoryDef(movies, "");

ConfigureFn(config_movies) {
  init_libmovies();
}

ConfigVariableList load_audio_type
("load-audio-type",
 PRC_DESC("List the audio loader modules that Panda will automatically "
          "import when a new, unknown audio type is loaded.  This may be "
          "either the name of a module, or a space-separate list of filename "
          "extensions, followed by the name of the module."));

ConfigVariableList load_video_type
("load-video-type",
 PRC_DESC("List the video loader modules that Panda will automatically "
          "import when a new, unknown video type is loaded.  This may be "
          "either the name of a module, or a space-separate list of filename "
          "extensions, followed by the name of the module."));

ConfigVariableBool vorbis_enable_seek
("vorbis-enable-seek", true,
 PRC_DESC("Set this to false if you're having trouble with seeking while "
          "using the Ogg Vorbis decoder."));

ConfigVariableBool vorbis_seek_lap
("vorbis-seek-lap", true,
 PRC_DESC("If this is set to true, the Ogg Vorbis decoder will automatically "
          "crosslap the transition from the previous playback position into "
          "the new playback position when seeking in order to eliminate "
          "clicking and boundary discontinuities."));

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

  InkblotVideo::init_type();
  InkblotVideoCursor::init_type();
  MicrophoneAudio::init_type();
  MovieAudio::init_type();
  MovieAudioCursor::init_type();
  MovieVideo::init_type();
  MovieVideoCursor::init_type();
  UserDataAudio::init_type();
  UserDataAudioCursor::init_type();
  WavAudio::init_type();
  WavAudioCursor::init_type();

#ifdef HAVE_VORBIS
  VorbisAudio::init_type();
  VorbisAudioCursor::init_type();
#endif

  MovieTypeRegistry *reg = MovieTypeRegistry::get_global_ptr();
  reg->register_audio_type(&WavAudio::make, "wav wave");

#ifdef HAVE_VORBIS
  reg->register_audio_type(&VorbisAudio::make, "ogg oga");
#endif
}
