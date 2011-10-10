// Filename: audioSound.cxx
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
set_3d_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz) {
  // Intentionally blank.
}

void AudioSound::
get_3d_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz) {
  // Intentionally blank.
}

void AudioSound::
set_3d_min_distance(PN_stdfloat dist) {
  // Intentionally blank.
}

PN_stdfloat AudioSound::
get_3d_min_distance() const {
  // Intentionally blank.
  return 0.0f;
}

void AudioSound::
set_3d_max_distance(PN_stdfloat dist) {
  // Intentionally blank.
}

PN_stdfloat AudioSound::
get_3d_max_distance() const {
  // Intentionally blank.
  return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::get_speaker_mix
//       Access: Published
//  Description: For use only with FMOD.
////////////////////////////////////////////////////////////////////
PN_stdfloat AudioSound::
get_speaker_mix(int speaker) {
    // intentionally blank
    return 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::set_speaker_mix
//       Access: Published
//  Description: For use only with FMOD.
////////////////////////////////////////////////////////////////////
void AudioSound::
set_speaker_mix(PN_stdfloat frontleft, PN_stdfloat frontright, PN_stdfloat center, PN_stdfloat sub, PN_stdfloat backleft, PN_stdfloat backright, PN_stdfloat sideleft, PN_stdfloat  sideright) {
    // intentionally blank
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::get_speaker_level
//       Access: Published
//  Description: For use only with Miles.
////////////////////////////////////////////////////////////////////
PN_stdfloat AudioSound::
get_speaker_level(int index) {
  // intentionally blank
  return 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::set_speaker_levels
//       Access: Published
//  Description: For use only with Miles.
////////////////////////////////////////////////////////////////////
void AudioSound::
set_speaker_levels(PN_stdfloat level1, PN_stdfloat level2, PN_stdfloat level3, PN_stdfloat level4, PN_stdfloat level5, PN_stdfloat level6, PN_stdfloat level7, PN_stdfloat level8, PN_stdfloat level9) {
  // intentionally blank
}

////////////////////////////////////////////////////////////////////
//     Function: AudioSound::configure_filters
//       Access: Published
//  Description: Configure the local DSP filter chain.
//
//               There is no guarantee that any given configuration
//               will be supported by the implementation.  The only 
//               way to find out what's supported is to call 
//               configure_filters.  If it returns true, the
//               configuration is supported.
////////////////////////////////////////////////////////////////////
bool AudioSound::
configure_filters(FilterProperties *config) {
  const FilterProperties::ConfigVector &conf = config->get_config();
  if (conf.empty()) {
    return true;
  } else {
    return false;
  }
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
