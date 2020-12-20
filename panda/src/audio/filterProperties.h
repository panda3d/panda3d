/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file filterProperties.h
 * @author jyelon
 * @date 2007-08-01
 */

#ifndef FILTERPROPERTIES_H
#define FILTERPROPERTIES_H

#include "config_audio.h"
#include "typedReferenceCount.h"
#include "dsp.h"
#include "pointerTo.h"

/**
 * Stores a configuration for a set of audio DSP filters.
 */
class EXPCL_PANDA_AUDIO FilterProperties : public TypedReferenceCount {
PUBLISHED:
  FilterProperties();
  ~FilterProperties();
  INLINE void clear();
  INLINE void add_filter(DSP *filter);

  void add_lowpass(PN_stdfloat cutoff_freq, PN_stdfloat resonance_q);
  void add_highpass(PN_stdfloat cutoff_freq, PN_stdfloat resonance_q);
  void add_echo(PN_stdfloat drymix, PN_stdfloat wetmix, PN_stdfloat delay, PN_stdfloat decayratio);
  void add_flange(PN_stdfloat drymix, PN_stdfloat wetmix, PN_stdfloat depth, PN_stdfloat rate);
  void add_distort(PN_stdfloat level);
  void add_normalize(PN_stdfloat fadetime, PN_stdfloat threshold, PN_stdfloat maxamp);
  void add_parameq(PN_stdfloat center_freq, PN_stdfloat bandwidth, PN_stdfloat gain);
  void add_pitchshift(PN_stdfloat pitch, PN_stdfloat fftsize, PN_stdfloat overlap);
  void add_chorus(PN_stdfloat drymix, PN_stdfloat wet1, PN_stdfloat wet2, PN_stdfloat wet3, PN_stdfloat delay, PN_stdfloat rate, PN_stdfloat depth);
  void add_sfxreverb(PN_stdfloat drylevel=0, PN_stdfloat room=-10000, PN_stdfloat roomhf=0, PN_stdfloat decaytime=1,
                     PN_stdfloat decayhfratio=0.5, PN_stdfloat reflectionslevel=-10000, PN_stdfloat reflectionsdelay=0.02,
                     PN_stdfloat reverblevel=0, PN_stdfloat reverbdelay=0.04, PN_stdfloat diffusion=100,
                     PN_stdfloat density=100, PN_stdfloat hfreference=5000, PN_stdfloat roomlf=0, PN_stdfloat lfreference=250);
  void add_compress(PN_stdfloat threshold, PN_stdfloat attack, PN_stdfloat release, PN_stdfloat gainmakeup);

public:
  typedef pvector<PT(DSP)> ConfigVector;

private:
  ConfigVector _config;

public:
  INLINE const ConfigVector &get_config();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "FilterProperties",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "filterProperties.I"

#endif // FILTERPROPERTIES_H
