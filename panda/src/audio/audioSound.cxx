// Filename: audioSound.cxx
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
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

#include "audioSound.h"

TypeHandle AudioSound::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
AudioSound::
~AudioSound() {
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
AudioSound::
AudioSound() {
  // Intentionally blank.
}


void AudioSound::
set_3d_attributes(float px, float py, float pz, float vx, float vy, float vz) {
  // Intentionally blank.
}

void AudioSound::
get_3d_attributes(float *px, float *py, float *pz, float *vx, float *vy, float *vz) {
  // Intentionally blank.
}

void AudioSound::
set_3d_min_distance(float dist) {
  // Intentionally blank.
}

float AudioSound::
get_3d_min_distance() const {
  // Intentionally blank.
  return 0.0f;
}

void AudioSound::
set_3d_max_distance(float dist) {
  // Intentionally blank.
}

float AudioSound::
get_3d_max_distance() const {
  // Intentionally blank.
  return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::add_dsp
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
bool AudioSound::
add_dsp(PT(AudioDSP) x) {
	// intentionally blank
	return false;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::remove_dsp
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
bool AudioSound::
remove_dsp(PT(AudioDSP) x) {
	// intentionally blank
	return false;
}


////////////////////////////////////////////////////////////////////
//     Function: AudioSound::getSpeakerMix
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
float AudioSound::
get_speaker_mix(int speaker) {
	// intentionally blank
	return 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::setSpeakerMix
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioSound::
set_speaker_mix(float frontleft, float frontright, float center, float sub, float backleft, float backright, float sideleft, float  sideright) {
	// intentionally blank
	;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::get_priority
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
int AudioSound::
get_priority() {
	// intentionally blank
	return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::set_priority
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioSound::
set_priority(int priority) {
	// intentionally blank
	;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioSound::
output(ostream &out) const {
  out << get_type() << " " << get_name() << " " << status();
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioSound::
write(ostream &out) const {
  out << (*this) << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::SoundStatus::output operator
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, AudioSound::SoundStatus status) {
  switch (status) {
  case AudioSound::BAD:
    return out << "BAD";

  case AudioSound::READY:
    return out << "READY";

  case AudioSound::PLAYING:
    return out << "PLAYING";
  }

  return out << "**invalid AudioSound::SoundStatus(" << (int)status << ")**";
}
