/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamDirectEffect.cxx
 * @author Jackson Sutherland
 */

#include "pandabase.h"

#include "steamDirectEffect.h"
#include "steamAudioManager.h"

#include "PTA_float"

#include <phonon.h>

TypeHandle SteamAudioManager::_type_handle;

//vars


//functions

/**
*
**/
SteamDirectEffect::SteamDirectEffect() :
  _dist_atten = SAD_GENERATE,
  _dist_atten_amnt = 0.6f,

  _air_absorb = SAD_GENERATE,
  _air_eq[0] = 0.9f,
  _air_eq[1] = 0.7f,
  _air_eq[2] = 0.5f,

  _directivity = SAD_GENERATE,
  _directivity_amnt = 0.7f,
  _dipole_amnt = 0.5f,
  _dipole_pwr = 2.0f,

  _occlusion = SAD_USER,
  _occAmnt = 0.4f,

  _transmission = SAD_USER,
  _trans_amnt[0] = 0.3f,
  _trans_amnt[1] = 0.2f,
  _trans_amnt[2] = 0.1f
{

}

/**
*
**/
SteamDirectEffect::~SteamDirectEffect() {

}

/**
*returns a blank outBuffer. This shouldn't be called, though.
**/
SteamDirectEffect::apply_effect(SteamAudioSound::SteamGlobalHolder* globals, IPLAudioBuffer inBuffer) {
  IPLAudioBuffer outBuffer;
  iplAudioBufferAllocate(globals->_steam_context, globals->_channels, globals->_samples, &outBuffer);//Be sure to deallocate this in SteamAudioSound

  IPLDirectEffectSettings effectSettings{};
  effectSettings.numChannels = globals->_channels;

  IPLDirectEffect effect = nullptr;
  iplDirectEffectCreate(globals->_steam_context, &(globals->_audio_settings), &effectSettings, &effect);

  IPLDirectEffectParams params{};

  iplDirectEffectApply(effect, &params, inBuffer, outBuffer);

  return outBuffer;
}

//Distance Attenuation

/**
*
**/
SteamDirectEffect::_transmission;

/**
*SAD_DISABLED,//don't use this sub-effect
*SAD_GENERATE,//automatically generate values
*SAD_USER//use user-provided values
**/
void SteamDirectEffect::set_distance_attenuation(unsigned short state) :
  _dist_atten = state
{}

/**
*
**/
unsigned short SteamDirectEffect::get_distance_attenuation() {
  return _dist_atten;
}

/**
*Mannualy sets the ammount of distance attenuation.
**/
void SteamDirectEffect::set_distance_attenuation_amount(float val) :
  _dist_atten_amnt = val,
  _dist_atten = SAD_USER
{}

/**
*TODO:: This function doesn't actually give the generated value if we are in SAD_GENERATE mode.
* That could be usefull for people looking to bake information or make ai react to sounds and such.
**/
float SteamDirectEffect::get_distance_attenuation_amount() {
  return _dist_atten_amnt;
}

//Air Absorption

/**
*
**/
void SteamDirectEffect::set_air_absorption(unsigned short state) :
  _air_absorb = state
{}

/**
*
**/
unsigned short SteamDirectEffect::get_air_absorption() {
  return _air_absorb;
}

/**
*
**/
void SteamDirectEffect::set_air_eq(float val1, float val2, float val3) :
  _air_eq[0] = val1,
  _air_eq[1] = val2,
  _air_eq[2] = val3,
  _air_absorb = SAD_USER
{}

/**
*
**/
PTA_float SteamDirectEffect::get_air_eq() {
  PTA_float pta;
  pta.reserve(3);
  pta.push_back(_air_eq[0]);
  pta.push_back(_air_eq[1]);
  pta.push_back(_air_eq[2]);
}

//Directivity

void SteamDirectEffect::set_directivity(unsigned short state) :
  _dist_atten = state
{}

/**
*
**/
unsigned short SteamDirectEffect::get_directivity() {
  return _dist_atten;
}

/**
*
**/
void SteamDirectEffect::configure_directivity(float directivity_amnt, float dipole_amnt, float dipole_pwr) :
  _directivity_amnt = 0.7f,
  _dipole_amnt = 0.5f,
  _dipole_pwr = 2.0f,
  _directivity = SAD_USER
{}

/**
*
**/
float SteamDirectEffect::get_directivity_amnt() {
  return _directivity_amnt;
}

/**
*
**/
float SteamDirectEffect::get_dipole_amnt() {
  return _dipole_amnt;
}

/**
*
**/
float SteamDirectEffect::get_dipole_pwr() {
  return _dipole_pwr;
}

//Occlusion

/**
*SAD_DISABLED,//don't use this sub-effect
*SAD_GENERATE,//automatically generate values
*SAD_USER//use user-provided values
**/
void SteamDirectEffect::set_occlusion(unsigned short state) {
  if (state == SAD_GENERATE) {//generating occlusion data requires a scene simulation. When a scene class is implemented, make this check more intellegent.
    _occlusion = SAD_DISABLED
  }
  else { _occlusion = state }
}

/**
*
**/
unsigned short SteamDirectEffect::get_occlusion() {
  return _occlusion;
}

/**
*
**/
void SteamDirectEffect::set_occlusion_amount(float val) :
  _occAmnt = val,
  _occlusion = SAD_USER
{}

/**
*
**/
float SteamDirectEffect::get_occlusion_ammount() {
  return _occAmnt;
}

//transmission

/**
*
**/
void SteamDirectEffect::set_transmission(unsigned short state) {
  if (state == SAD_GENERATE) {//generating transmission data requires a scene simulation. When a scene class is implemented, make this check more intellegent.
    _transmission = SAD_DISABLED
  }
  else { _transmission = state }
}

/**
*
**/
unsigned short SteamDirectEffect::get_transmission() {
  return _transmission;
}

/**
*
**/
void SteamDirectEffect::set_transmission_eq(float val1, float val2, float val3) :
  _trans_amnt[0] = val1,
  _trans_amnt[1] = val2,
  _trans_amnt[2] = val3,
  _transmission = SAD_USER
{}

/**
*
**/
PTA_float SteamDirectEffect::get_transmission_eq() {
  PTA_float pta;
  pta.reserve(3);
  pta.push_back(_trans_amnt[0]);
  pta.push_back(_trans_amnt[1]);
  pta.push_back(_trans_amnt[2]);
}
