// Filename: audio_null_traits.cxx
// Created by:  cary (25Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "audio_null_traits.h"
#include "audio_manager.h"
#include "config_audio.h"

static bool have_initialized = false;

static void update_null(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "Update in Null audio driver" << endl;
}

static void initialize(void) {
  if (have_initialized)
    return;
  AudioManager::set_update_func(update_null);
  have_initialized = true;
}

NullSample::~NullSample(void) {
}

float NullSample::length(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in sample length in Null audio driver" << endl;
  return 0.;
}

AudioTraits::SampleClass::SampleStatus NullSample::status(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in sample status in Null audio driver" << endl;
  return AudioTraits::SampleClass::READY;
}

NullMusic::~NullMusic(void) {
}

AudioTraits::MusicClass::MusicStatus NullMusic::status(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in music status in Null audio driver" << endl;
  return READY;
}

NullPlayer::~NullPlayer(void) {
}

void NullPlayer::play_sample(AudioTraits::SampleClass*) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in play sample in Null audio driver" << endl;
}

void NullPlayer::play_music(AudioTraits::MusicClass*) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in play music in Null audio driver" << endl;
}

void NullPlayer::set_volume(AudioTraits::SampleClass*, int) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in set volume (sample) in Null audio driver"
		       << endl;
}

void NullPlayer::set_volume(AudioTraits::MusicClass*, int) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in set volume (music) in Null audio driver" << endl;
}
