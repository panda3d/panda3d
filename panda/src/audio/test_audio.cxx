// Filename: test_audio.cxx
// Created by:  cary (24Sep00)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>
#include "audio.h"

#include "config_audio.h"
#include <ipc_traits.h>

int
main(int argc, char* argv[]) {
  //   if (! AudioPool::verify_sample("test.wav")) {
  //     audio_cat->fatal() << "could not locate 'test.wav'" << endl;
  //     exit(-1);
  //   }
  if (! AudioPool::verify_sound("test.mp3")) {
    audio_cat->fatal() << "could not locate 'test.mp3'" << endl;
    exit(-1);
  }
  //   AudioSample* sample = AudioPool::load_sample("test.wav");
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
  return 0;
}
