// Filename: audio_trait.cxx
// Created by:  cary (23Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "audio_trait.h"
#include "config_audio.h"

AudioTraits::SoundClass::~SoundClass(void) {
}

float AudioTraits::SoundClass::length(void) const {
  audio_cat->error() << "In abstract SoundClass::length!" << endl;
  return -1.;
}

AudioTraits::PlayingClass* AudioTraits::SoundClass::get_state(void) const {
  audio_cat->error() << "In abstract SoundClass::get_state!" << endl;
  return (AudioTraits::PlayingClass*)0L;
}

AudioTraits::PlayerClass* AudioTraits::SoundClass::get_player(void) const {
  audio_cat->error() << "In abstract SoundClass::get_player!" << endl;
  return (AudioTraits::PlayerClass*)0L;
}

AudioTraits::DeletePlayingFunc*
AudioTraits::SoundClass::get_delstate(void) const {
  audio_cat->error() << "In abstract SoundClass::get_delstate!" << endl;
  return (AudioTraits::DeletePlayingFunc*)0L;
}

AudioTraits::PlayingClass::~PlayingClass(void) {
}

AudioTraits::PlayingClass::PlayingStatus
AudioTraits::PlayingClass::status(void) {
  audio_cat->error() << "In abstract PlayingClass::status!" << endl;
  return BAD;
}

AudioTraits::PlayerClass::~PlayerClass(void) {
}

void AudioTraits::PlayerClass::play_sound(AudioTraits::SoundClass*,
					  AudioTraits::PlayingClass*, float) {
  audio_cat->error() << "In abstract PlayerClass::play_sound!" << endl;
}

void AudioTraits::PlayerClass::stop_sound(AudioTraits::SoundClass*,
					  AudioTraits::PlayingClass*) {
  audio_cat->error() << "In abstract PlayerClass::stop_sound!" << endl;
}

void AudioTraits::PlayerClass::set_volume(AudioTraits::PlayingClass*, float) {
  audio_cat->error() << "In abstract PlayerClass::set_volume!" << endl;
}

void AudioTraits::PlayerClass::adjust_volume(AudioTraits::PlayingClass*) {
  audio_cat->error() << "In abstract PlayerClass::adjust_volume!" << endl;
}
