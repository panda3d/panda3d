// Filename: audio_load_wav.cxx
// Created by:  cary (23Sep00)
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

Configure(audio_load_wav);

#include "audio_trait.h"

#ifdef AUDIO_USE_MIKMOD

#include "audio_mikmod_traits.h"

AudioTraits::SoundClass* AudioLoadWav(Filename filename) {
  return MikModSample::load_wav(filename);
}

#elif defined(AUDIO_USE_WIN32)

#include "audio_win_traits.h"

EXPCL_MISC AudioTraits::SoundClass* AudioLoadWav(Filename filename) {
  return WinSample::load_wav(filename);
}

#elif defined(AUDIO_USE_LINUX)

#include "audio_linux_traits.h"

AudioTraits::SoundClass* AudioLoadWav(Filename) {
  audio_cat->error() << "Linux driver does not natively support WAV."
                     << "  Try the 'st' loader." << endl;
  return (AudioTraits::SoundClass*)0L;
}

#elif defined(AUDIO_USE_NULL)

// Null driver
#include "audio_null_traits.h"

AudioTraits::SoundClass* AudioLoadWav(Filename) {
  return new NullSound();
}

#else /* AUDIO_USE_NULL */

#error "unknown implementation driver"

#endif /* AUDIO_USE_NULL */

ConfigureFn(audio_load_wav) {
  AudioPool::register_sound_loader("wav", AudioLoadWav);
}
