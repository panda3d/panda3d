// Filename: audio_load_wav.cxx
// Created by:  cary (23Sep00)
// 
////////////////////////////////////////////////////////////////////

#include <dconfig.h>
#include "audio_pool.h"

Configure(audio_load_wav);

#ifdef USE_MIKMOD

#include "audio_mikmod_traits.h"

void AudioDestroyWav(AudioTraits::SampleClass* sample) {
  MikModSample::destroy(sample);
}

void AudioLoadWav(AudioTraits::SampleClass** sample,
		  AudioTraits::PlayerClass** player,
		  AudioTraits::DeleteSampleFunc** destroy, Filename filename) {
  *sample = MikModSample::load_wav(filename);
  if (*sample == (AudioTraits::SampleClass*)0L)
    return;
  *player = MikModSamplePlayer::get_instance();
  *destroy = AudioDestroyWav;
}

#else /* no MikMod */

#ifdef PENV_WIN32

#include "audio_win_traits.h"

void AudioDestroyWav(AudioTraits::SampleClass* sample) {
  WinSample::destroy(sample);
}

void AudioLoadWav(AudioTraits::SampleClass** sample,
		  AudioTraits::PlayerClass** player,
		  AudioTraits::DeleteSampleFunc** destroy, Filename filename) {
  *sample = WinSample::load_wav(filename);
  if (*sample == (AudioTraits::SampleClass*)0L)
    return;
  *player = WinPlayer::get_instance();
  *destroy = AudioDestroyWav;
}

#else /* no win32 */

// Null driver
#include "audio_null_traits.h"

void AudioDestroyWav(AudioTraits::SampleClass* sample) {
  delete sample;
}

void AudioLoadWav(AudioTraits::SampleClass** sample,
		  AudioTraits::PlayerClass** player,
		  AudioTraits::DeleteSampleFunc** destroy, Filename) {
  *sample = new NullSample();
  *player = new NullPlayer();
  *destroy = AudioDestroyWav;
}

#endif /* win32 */
#endif /* MikMod */

ConfigureFn(audio_load_wav) {
  AudioPool::register_sample_loader("wav", AudioLoadWav);
}
