// Filename: audio_trait.cxx
// Created by:  cary (23Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "audio_trait.h"
#include "config_audio.h"

AudioTraits::SampleClass::~SampleClass(void) {
}

float AudioTraits::SampleClass::length(void) {
  audio_cat->error() << "In abstract SampleClass::length!" << endl;
  return 0.;
}

AudioTraits::SampleClass::SampleStatus AudioTraits::SampleClass::status(void) {
  audio_cat->error() << "In abstract SampleClass::status!" << endl;
  return READY;
}

AudioTraits::MusicClass::~MusicClass(void) {
}

AudioTraits::MusicClass::MusicStatus AudioTraits::MusicClass::status(void) {
  audio_cat->error() << "In abstract MusicClass::status!" << endl;
  return READY;
}

AudioTraits::PlayerClass::~PlayerClass(void) {
}

void AudioTraits::PlayerClass::play_sample(AudioTraits::SampleClass*) {
  audio_cat->error() << "In abstract PlayerClass::play_sample!" << endl;
}

void AudioTraits::PlayerClass::play_music(AudioTraits::MusicClass*) {
  audio_cat->error() << "In abstract PlayerClass::play_music!" << endl;
}

void AudioTraits::PlayerClass::set_volume(AudioTraits::SampleClass*, int) {
  audio_cat->error() << "In abstract PlayerClass::set_volume (sample)!"
		     << endl;
}

void AudioTraits::PlayerClass::set_volume(AudioTraits::MusicClass*, int) {
  audio_cat->error() << "In abstract PlayerClass::set_volume (music)!" << endl;
}
