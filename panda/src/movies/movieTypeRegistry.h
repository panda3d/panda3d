/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieTypeRegistry.h
 * @author rdb
 * @date 2013-08-24
 */

#ifndef MOVIETYPEREGISTRY_H
#define MOVIETYPEREGISTRY_H

#include "pandabase.h"
#include "movieAudio.h"
#include "movieVideo.h"
#include "filename.h"
#include "pmap.h"
#include "reMutex.h"

/**
 * This class records the different types of MovieAudio and MovieVideo that
 * are available for loading.
 */
class EXPCL_PANDA_MOVIES MovieTypeRegistry {
public:
  typedef PT(MovieAudio) (*MakeAudioFunc)(const Filename&);
  PT(MovieAudio) make_audio(const Filename &name);
  void register_audio_type(MakeAudioFunc func, const std::string &extensions);
  void load_audio_types();

  typedef PT(MovieVideo) (*MakeVideoFunc)(const Filename&);
  PT(MovieVideo) make_video(const Filename &name);
  void register_video_type(MakeVideoFunc func, const std::string &extensions);
  void load_video_types();

  void load_movie_library(const std::string &name);

  INLINE static MovieTypeRegistry *get_global_ptr();

private:
  static MovieTypeRegistry *_global_ptr;

  ReMutex _audio_lock;
  pmap<std::string, MakeAudioFunc> _audio_type_registry;
  pmap<std::string, std::string> _deferred_audio_types;

  ReMutex _video_lock;
  pmap<std::string, MakeVideoFunc> _video_type_registry;
  pmap<std::string, std::string> _deferred_video_types;
};

#include "movieTypeRegistry.I"

#endif
