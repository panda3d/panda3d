// Filename: audio_null_traits.cxx
// Created by:  cary (25Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "audio_null_traits.h"

#ifdef AUDIO_USE_NULL

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

NullSound::~NullSound(void) {
}

float NullSound::length(void) const {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in sample length in Null audio driver" << endl;
  return -1.;
}

AudioTraits::PlayingClass* NullSound::get_state(void) const {
  return new NullPlaying((NullSound*)this);
}

AudioTraits::PlayerClass* NullSound::get_player(void) const {
  return new NullPlayer();
}

// REFCOUNT
/*
AudioTraits::DeleteSoundFunc* NullSound::get_destroy(void) const {
  return NullSound::destroy;
}
*/

AudioTraits::DeletePlayingFunc* NullSound::get_delstate(void) const {
  return NullPlaying::destroy;
}

// REFCOUNT
/*
void NullSound::destroy(AudioTraits::SoundClass* sound) {
  delete sound;
}
*/

NullPlaying::~NullPlaying(void) {
}

AudioTraits::PlayingClass::PlayingStatus NullPlaying::status(void) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in playing status in Null audio driver" << endl;
  return BAD;
}

void NullPlaying::destroy(AudioTraits::PlayingClass* play) {
  delete play;
}

NullPlayer::~NullPlayer(void) {
}

void NullPlayer::play_sound(AudioTraits::SoundClass*,
			    AudioTraits::PlayingClass*) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in play sound in Null audio driver" << endl;
}

void NullPlayer::stop_sound(AudioTraits::SoundClass*,
			    AudioTraits::PlayingClass*) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in stop sound in Null audio driver" << endl;
}

void NullPlayer::set_volume(AudioTraits::PlayingClass*, float) {
  if (audio_cat->is_debug())
    audio_cat->debug() << "in set volume in Null audio driver"
		       << endl;
}

#endif /* AUDIO_USE_NULL */
