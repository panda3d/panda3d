// Filename: test_audio.cxx
// Created by:  cary (24Sep00)
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

#include "pandabase.h"
#include "audio.h"

#include "config_audio.h"
#include <ipc_traits.h>

int
main(int argc, char* argv[]) {

  AudioManager::spawn_update();
  {
    {
      PT(AudioSound) tester = AudioPool::load_sound(argv[1]);
      AudioManager::play(tester);
      AudioPool::release_all_sounds();
      cerr << "all sounds but 1 released" << endl;
    }
    cerr << "all sounds released" << endl;
  }

  /*
  if (! AudioPool::verify_sound("test.mp3")) {
    audio_cat->fatal() << "could not locate 'test.mp3'" << endl;
    exit(-1);
  }
  AudioSound* sample = AudioPool::load_sound("test.mp3");
  audio_cat->info() << "test.wav is " << sample->length() << " sec long"
                     << endl;
  audio_cat->info() << "Playing test.wav" << endl;
  AudioManager::play(sample);
  while (sample->status() == AudioSound::PLAYING) {
    AudioManager::update();
    ipc_traits::sleep(0, 1000000);
  }

  //   AudioMidi foo("test.midi");
  if (! AudioPool::verify_sound("test.midi")) {
    audio_cat->fatal() << "could not locate 'test.midi'" << endl;
    exit(-1);
  }
  AudioSound* music = AudioPool::load_sound("test.midi");
  audio_cat->info() << "Playing test.midi" << endl;
  AudioManager::play(music);
  while (music->status() == AudioSound::PLAYING) {
    AudioManager::update();
    ipc_traits::sleep(0, 1000000);
  }
  */
  return 0;
}
