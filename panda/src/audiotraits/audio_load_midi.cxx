// Filename: audio_load_midi.cxx
// Created by:  cary (26Sep00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include <dconfig.h>
#include "audio_pool.h"
#include "config_audio.h"
#include "audio_trait.h"

Configure(audio_load_midi);

#ifdef AUDIO_USE_MIKMOD

#include "audio_mikmod_traits.h"

AudioTraits::SoundClass* AudioLoadMidi(Filename filename) {
  return MikModMidi::load_midi(filename);
}

#elif defined(AUDIO_USE_WIN32)

#include "audio_win_traits.h"

EXPCL_MISC AudioTraits::SoundClass* AudioLoadMidi(Filename filename) {
  return WinMusic::load_midi(filename);
}

#elif defined(AUDIO_USE_LINUX)

#include "audio_linux_traits.h"

AudioTraits::SoundClass* AudioLoadMidi(Filename) {
  audio_cat->warning() << "linux doesn't support reading midi data yet"
                       << endl;
  return (AudioTraits::SoundClass*)0L;
}

#elif defined(AUDIO_USE_NULL)

// Null driver
#include "audio_null_traits.h"

AudioTraits::SoundClass* AudioLoadMidi(Filename) {
  return new NullSound();
}

#else /* AUDIO_USE_NULL */

#error "unknown driver type"

#endif /* AUDIO_USE_NULL */

ConfigureFn(audio_load_midi) {
  AudioPool::register_sound_loader("midi", AudioLoadMidi);
  AudioPool::register_sound_loader("mid", AudioLoadMidi);
}
