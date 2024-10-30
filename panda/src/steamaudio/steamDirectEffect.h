/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamDirectEffect.h
 * @author Jackson Sutherland
 */

#ifndef STEAMDIRECTEFFECT_H
#define STEAMDIRECTEFFECT_H

#include "pandabase.h"
#include "typedObject.h"

#include "steamAudioSound.h"
#include "nodePath.h"
#include "movieAudioCursor.h"
#include "plist.h"//Don't know if I'll need these, but good idea to keep in hand
#include "PTA_float"
#include "steamAudioEffect.h"

#include <phonon.h>


class EXPCL_STEAMAUDIO SteamDirectEffect : public SteamAudioEffect {

friend class SteamAudioSound
PUBLISHED:
    SteamDirectEffect();
    ~SteamDirectEffect();

    void set_distance_attenuation(unsigned short state);
    unsigned short get_distance_attenuation();
    void set_distance_attenuation_amount(float val);
    float get_distance_attenuation_ammount();

    void set_air_absorption(unsigned short state);
    unsigned short get_air_absorption();
    void set_air_eq(float val1, float val2, float val3);
    PTA_float get_air_eq();

    void set_directivity(unsigned short state);
    unsigned short get_directivity();
    void configure_directivity(float directivity_amnt, float dipole_amnt, float dipole_pwr);
    float get_directivity_amnt();
    float get_dipole_amnt();
    float get_dipole_pwr();

    void set_occlusion(unsigned short state);
    unsigned short get_occlusion();
    void set_occlusion_amount(float val);
    float get_occlusion_ammount();

    void set_transmission(unsigned short state);
    unsigned short get_transmission();
    void set_transmission_eq(float val1, float val2, float val3);
    PTA_float get_transmission_eq();

    enum val_calc_mode {
      SAD_DISABLED,//don't use this sub-effect
      SAD_GENERATE,//automatically generate values
      SAD_USER//use user-provided values
    };

protected:
  virtual IPLAudioBuffer apply_effect(SteamAudioSound::SteamGlobalHolder* globals, IPLAudioBuffer inBuffer);

  unsigned short _dist_atten;//IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION
  float _dist_atten_amnt;

  unsigned short _air_absorb;//IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION
  float _air_eq[3];


  unsigned short _directivity;//IPL_DIRECTEFFECTFLAGS_APPLYDIRECTIVITY
  float _directivity_amnt;
  float _dipole_amnt;
  float _dipole_pwr;


  unsigned short _occlusion;//IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION
  float _occAmnt;


  unsigned short _transmission;//IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION
  float _trans_amnt[3];

  //TODO:: allow users to set different models for the sub-effects.
  //TODO:: allow this class to use a scene to run spatial simulations.

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "SteamDirectEffect", SteamAudioEffect::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }
}

#endif
