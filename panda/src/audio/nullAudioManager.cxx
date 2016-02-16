/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nullAudioManager.cxx
 * @author skyler
 * @date 2001-06-06
 * Prior system by: cary
 */

#include "nullAudioManager.h"

TypeHandle NullAudioManager::_type_handle;

//namespace {
    //static const string blank="";
    //static PN_stdfloat no_listener_attributes [] = {0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f};
//}


////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::NullAudioManager
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
NullAudioManager::
NullAudioManager() {
  audio_info("NullAudioManager");
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::~NullAudioManager
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
NullAudioManager::
~NullAudioManager() {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::is_valid
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool NullAudioManager::
is_valid() {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::get_sound
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PT(AudioSound) NullAudioManager::
get_sound(const string&, bool positional, int mode) {
  return get_null_sound();
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::get_sound
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PT(AudioSound) NullAudioManager::
get_sound(MovieAudio *sound, bool positional, int mode) {
  return get_null_sound();
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::uncache_sound
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
uncache_sound(const string&) {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::uncache_all_sounds
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
clear_cache() {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::set_cache_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
set_cache_limit(unsigned int) {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::get_cache_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int NullAudioManager::
get_cache_limit() const {
  // intentionally blank.
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::set_volume
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
set_volume(PN_stdfloat) {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::get_volume
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat NullAudioManager::
get_volume() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::set_play_rate
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
set_play_rate(PN_stdfloat) {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::get_play_rate
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat NullAudioManager::
get_play_rate() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::set_active
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
set_active(bool) {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::get_active
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool NullAudioManager::
get_active() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::set_concurrent_sound_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
set_concurrent_sound_limit(unsigned int) {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::get_concurrent_sound_limit
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int NullAudioManager::
get_concurrent_sound_limit() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::reduce_sounds_playing_to
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
reduce_sounds_playing_to(unsigned int) {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::stop_all_sounds
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
stop_all_sounds() {
  // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::audio_3d_set_listener_attributes
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
audio_3d_set_listener_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz, PN_stdfloat fx, PN_stdfloat fy, PN_stdfloat fz, PN_stdfloat ux, PN_stdfloat uy, PN_stdfloat uz) {
    // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::audio_3d_get_listener_attributes
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
audio_3d_get_listener_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz, PN_stdfloat *fx, PN_stdfloat *fy, PN_stdfloat *fz, PN_stdfloat *ux, PN_stdfloat *uy, PN_stdfloat *uz) {
    // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::audio_3d_set_distance_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
audio_3d_set_distance_factor(PN_stdfloat factor) {
    // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::audio_3d_get_distance_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat NullAudioManager::
audio_3d_get_distance_factor() const {
    // intentionally blank.
    return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::audio_3d_set_doppler_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
audio_3d_set_doppler_factor(PN_stdfloat factor) {
    // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::audio_3d_get_doppler_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat NullAudioManager::
audio_3d_get_doppler_factor() const {
    // intentionally blank.
    return 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::audio_3d_set_drop_off_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void NullAudioManager::
audio_3d_set_drop_off_factor(PN_stdfloat factor) {
    // intentionally blank.
}

////////////////////////////////////////////////////////////////////
//     Function: NullAudioManager::audio_3d_get_drop_off_factor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PN_stdfloat NullAudioManager::
audio_3d_get_drop_off_factor() const {
    // intentionally blank.
    return 0.0f;
}

