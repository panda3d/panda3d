// Filename: audio_sample.cxx
// Created by:  cary (23Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "audio_sample.h"
#include "config_audio.h"

TypeHandle AudioSample::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AudioSample::destructor
//       Access: Public
//  Description: deletes the sample data and then lets the system
//               destroy this structure
////////////////////////////////////////////////////////////////////
AudioSample::~AudioSample(void) {
  (*_destroy)(_sample);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSample::length
//       Access: Public
//  Description: return the length (in seconds) of the sample
////////////////////////////////////////////////////////////////////
float AudioSample::length(void) {
  return _sample->length();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSample::status
//       Access: Public
//  Description: return the current play status of this sample
////////////////////////////////////////////////////////////////////
AudioSample::SampleStatus AudioSample::status(void) {
  AudioTraits::SampleClass::SampleStatus stat = _sample->status();
  switch (stat) {
  case AudioTraits::SampleClass::READY:
    return READY;
  case AudioTraits::SampleClass::PLAYING:
    return PLAYING;
  }
  audio_cat->error() << "unknown status for sample" << endl;
  return READY;
}
