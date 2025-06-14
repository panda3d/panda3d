/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_audio.cxx
 * @author cary
 * @date 2000-09-22
 */

#include "config_audio.h"
#include "dconfig.h"
#include "filterProperties.h"
#include "audioLoadRequest.h"
#include "audioManager.h"
#include "audioSound.h"
#include "nullAudioManager.h"
#include "nullAudioSound.h"
#include "string_utils.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_AUDIO)
  #error Buildsystem error: BUILDING_PANDA_AUDIO not defined
#endif

using std::istream;
using std::ostream;
using std::string;

Configure(config_audio);
NotifyCategoryDef(audio, "");

ConfigVariableBool audio_active
("audio-active", true);

ConfigVariableInt audio_cache_limit
("audio-cache-limit", 15,
 PRC_DESC("The number of sounds in the cache."));

ConfigVariableString audio_library_name
("audio-library-name", "null");

ConfigVariableDouble audio_volume
("audio-volume", 1.0f);

// Config variables for OpenAL:

ConfigVariableDouble audio_doppler_factor
("audio-doppler-factor", 1.0f);

ConfigVariableDouble audio_distance_factor
("audio-distance-factor", 1.0f);

ConfigVariableDouble audio_drop_off_factor
("audio-drop-off-factor", 1.0f);

ConfigVariableDouble audio_buffering_seconds
("audio-buffering-seconds", 3.0f,
 PRC_DESC("Controls the amount of audio buffering when streaming audio. "
          "If you are playing a streaming sound, and any single frame "
          "takes longer than this, the audio will stutter.  Caution: "
          "buffering streaming audio takes a lot of memory.  For example, "
          "5 seconds of stereo audio at 44,100 samples/sec takes one "
          "megabyte.  The 3-second default is intentionally high, favoring "
          "correctness over efficiency, but for a commercial application "
          "you may wish to lower this."));

ConfigVariableInt audio_preload_threshold
("audio-preload-threshold", 1000000,
 PRC_DESC("If the decompressed size of a sound file exceeds this amount, "
          "then Panda3D will not attempt to store that sound file in RAM. "
          "Instead, it will stream the sound file from disk.  It is not "
          "practical to stream multiple sound-files from disk at the same "
          "time - the hard drive seek time makes it stutter."));

// Unknown

ConfigVariableInt audio_min_hw_channels
("audio-min-hw-channels", 15,
PRC_DESC("Guarantee this many channels on the local sound card, or just "
         "play EVERYTHING in software."));

// Config variables for Fmod:

ConfigVariableInt fmod_number_of_sound_channels
("fmod-number-of-sound-channels", 128,
 PRC_DESC("Guarantee this many channels you will have with FMOD.  AKA the max number of sounds you can play at one time.") );

ConfigVariableBool fmod_use_surround_sound
("fmod-use-surround-sound", false,
 PRC_DESC("Determines if an FMOD Flavor of PANDA use 5.1 Surround Sound or not.  "
          "This variable is deprecated and should not be used.  Use the enum "
          "variable fmod-speaker-mode instead."));

ConfigVariableEnum<FmodSpeakerMode> fmod_speaker_mode
("fmod-speaker-mode", FSM_unspecified,
 PRC_DESC("Sets the speaker configuration that the FMOD sound system will use. "
          "Options: raw, mono, stereo, quad, surround, 5.1 and 7.1. "));


ConfigVariableFilename audio_dls_file
("audio-dls-file", Filename(),
 PRC_DESC("Specifies a DLS file that defines an instrument set to load "
          "for MIDI file playback.  If this is not specified, the sound "
          "interface will try to use the system default DLS file, if "
          "one is available; the likely success of this depends on the "
          "operating system."));

ConfigureFn(config_audio) {
  FilterProperties::init_type();
  AudioLoadRequest::init_type();
  AudioManager::init_type();
  AudioSound::init_type();
  NullAudioManager::init_type();
  NullAudioSound::init_type();
}

ostream &
operator << (ostream &out, FmodSpeakerMode sm) {
  switch (sm) {
  case FSM_raw:
    return out << "raw";
  case FSM_mono:
    return out << "mono";
  case FSM_stereo:
    return out << "stereo";
  case FSM_quad:
    return out << "quad";
  case FSM_surround:
    return out << "surround";
  case FSM_5point1:
    return out << "5.1";
  case FSM_7point1:
    return out << "7.1";
  case FSM_unspecified:
    return out;
  }

  return out << "**invalid FmodSpeakerMode (" << (int)sm << ")**";
}

istream &
operator >> (istream &in, FmodSpeakerMode &sm) {
  string word;
  in >> word;

  if (word.size() == 0) {
    sm = FSM_unspecified;
  } else if (cmp_nocase(word, "raw") == 0) {
    sm = FSM_raw;
  } else if (cmp_nocase(word, "mono") == 0) {
    sm = FSM_mono;
  } else if (cmp_nocase(word, "stereo") == 0) {
    sm = FSM_stereo;
  } else if (cmp_nocase(word, "quad") == 0) {
    sm = FSM_quad;
  } else if (cmp_nocase(word, "surround") == 0) {
    sm = FSM_surround;
  } else if (cmp_nocase(word, "5point1") == 0 ||
             cmp_nocase(word, "5.1") == 0) {
    sm = FSM_5point1;
  } else if (cmp_nocase(word, "7point1") == 0 ||
             cmp_nocase(word, "7.1") == 0) {
    sm = FSM_7point1;

  } else {
    audio_cat->error() << "Invalid FmodSpeakerMode value: " << word << "\n";
    sm = FSM_unspecified;
  }

  return in;
}
