/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_audio.cxx
 * @author cary
 * @date 2000-09-24
 */

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
      std::cerr << "all sounds but 1 released" << std::endl;
    }
    std::cerr << "all sounds released" << std::endl;
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

  // AudioMidi foo("test.midi");
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
