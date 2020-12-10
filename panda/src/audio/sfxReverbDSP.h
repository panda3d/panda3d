/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sfxReverbDSP.h
 * @author lachbr
 * @date 2020-10-06
 */

#ifndef SFXREVERBDSP_H
#define SFXREVERBDSP_H

#include "dsp.h"
#include "pointerTo.h"

/**
 * SFX reverb DSP filter.
 */
class EXPCL_PANDA_AUDIO SFXReverbDSP : public DSP {
PUBLISHED:
  INLINE SFXReverbDSP(float decay_time = 1500, float early_delay = 20,
                      float late_delay = 40, float hf_reference = 5000,
                      float hf_decay_ratio = 50, float diffusion = 50,
                      float density = 50, float low_shelf_frequency = 250,
                      float low_shelf_gain = 0, float highcut = 20000,
                      float early_late_max = 50, float wetlevel = -6,
                      float drylevel = 0);

  INLINE void set_decay_time(float time);
  INLINE float get_decay_time() const;
  MAKE_PROPERTY(decay_time, get_decay_time, set_decay_time);

  INLINE void set_early_delay(float time);
  INLINE float get_early_delay() const;
  MAKE_PROPERTY(early_delay, get_early_delay, set_early_delay);

  INLINE void set_late_delay(float time);
  INLINE float get_late_delay() const;
  MAKE_PROPERTY(late_delay, get_late_delay, set_late_delay);

  INLINE void set_hf_reference(float ref);
  INLINE float get_hf_reference() const;
  MAKE_PROPERTY(hf_reference, get_hf_reference, set_hf_reference);

  INLINE void set_hf_decay_ratio(float ratio);
  INLINE float get_hf_decay_ratio() const;
  MAKE_PROPERTY(hf_decay_ratio, get_hf_decay_ratio, set_hf_decay_ratio);

  INLINE void set_diffusion(float diffusion);
  INLINE float get_diffusion() const;
  MAKE_PROPERTY(diffusion, get_diffusion, set_diffusion);

  INLINE void set_density(float density);
  INLINE float get_density() const;
  MAKE_PROPERTY(density, get_density, set_density);

  INLINE void set_low_shelf_frequency(float freq);
  INLINE float get_low_shelf_frequency() const;
  MAKE_PROPERTY(low_shelf_frequency, get_low_shelf_frequency,
                set_low_shelf_frequency);

  INLINE void set_low_shelf_gain(float gain);
  INLINE float get_low_shelf_gain() const;
  MAKE_PROPERTY(low_shelf_gain, get_low_shelf_gain, set_low_shelf_gain);

  INLINE void set_highcut(float highcut);
  INLINE float get_highcut() const;
  MAKE_PROPERTY(highcut, get_highcut, set_highcut);

  INLINE void set_early_late_mix(float mix);
  INLINE float get_early_late_mix() const;
  MAKE_PROPERTY(early_late_mix, get_early_late_mix, set_early_late_mix);

  INLINE void set_wetlevel(float wetlevel);
  INLINE float get_wetlevel() const;
  MAKE_PROPERTY(wetlevel, get_wetlevel, set_wetlevel);

  INLINE void set_drylevel(float drylevel);
  INLINE float get_drylevel() const;
  MAKE_PROPERTY(drylevel, get_drylevel, set_drylevel);

PUBLISHED:
  // Helper methods to return preset reverb configs for different environments.
  INLINE static PT(SFXReverbDSP) get_generic();
  INLINE static PT(SFXReverbDSP) get_padded_cell();
  INLINE static PT(SFXReverbDSP) get_room();
  INLINE static PT(SFXReverbDSP) get_bathroom();
  INLINE static PT(SFXReverbDSP) get_living_room();
  INLINE static PT(SFXReverbDSP) get_stone_room();
  INLINE static PT(SFXReverbDSP) get_auditorium();
  INLINE static PT(SFXReverbDSP) get_concert_hall();
  INLINE static PT(SFXReverbDSP) get_cave();
  INLINE static PT(SFXReverbDSP) get_arena();
  INLINE static PT(SFXReverbDSP) get_hangar();
  INLINE static PT(SFXReverbDSP) get_carpetted_hallway();
  INLINE static PT(SFXReverbDSP) get_hallway();
  INLINE static PT(SFXReverbDSP) get_stone_corridor();
  INLINE static PT(SFXReverbDSP) get_alley();
  INLINE static PT(SFXReverbDSP) get_forest();
  INLINE static PT(SFXReverbDSP) get_city();
  INLINE static PT(SFXReverbDSP) get_mountains();
  INLINE static PT(SFXReverbDSP) get_quarry();
  INLINE static PT(SFXReverbDSP) get_plain();
  INLINE static PT(SFXReverbDSP) get_parking_lot();
  INLINE static PT(SFXReverbDSP) get_sewer_pipe();
  INLINE static PT(SFXReverbDSP) get_underwater();

private:
  float _decay_time;
  float _early_delay;
  float _late_delay;
  float _hf_reference;
  float _hf_decay_ratio;
  float _diffusion;
  float _density;
  float _low_shelf_freq;
  float _low_shelf_gain;
  float _highcut;
  float _early_late_mix;
  float _wetlevel;
  float _drylevel;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "SFXReverbDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sfxReverbDSP.I"

#endif // SFXREVERBDSP_H
