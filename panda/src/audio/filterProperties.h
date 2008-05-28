// Filename: filterProperties.h
// Created by: jyelon (01Aug2007)
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

#ifndef FILTERPROPERTIES_H
#define FILTERPROPERTIES_H

#include "config_audio.h"
#include "typedReferenceCount.h"

////////////////////////////////////////////////////////////////////
//       Class : FilterProperties
// Description : Stores a configuration for a set of audio DSP filters.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA_AUDIO FilterProperties : public TypedReferenceCount {
 PUBLISHED:
  FilterProperties();
  ~FilterProperties();
  INLINE void clear();
  INLINE void add_lowpass(float cutoff_freq, float resonance_q);
  INLINE void add_highpass(float cutoff_freq, float resonance_q);
  INLINE void add_echo(float drymix, float wetmix, float delay, float decayratio);
  INLINE void add_flange(float drymix, float wetmix, float depth, float rate);
  INLINE void add_distort(float level);
  INLINE void add_normalize(float fadetime, float threshold, float maxamp);
  INLINE void add_parameq(float center_freq, float bandwidth, float gain);
  INLINE void add_pitchshift(float pitch, float fftsize, float overlap);
  INLINE void add_chorus(float drymix, float wet1, float wet2, float wet3, float delay, float rate, float depth, float feedback);
  INLINE void add_reverb(float drymix, float wetmix, float roomsize, float damp, float width);
  INLINE void add_compress(float threshold, float attack, float release, float gainmakeup);

 public:

  enum FilterType {
    FT_lowpass,
    FT_highpass,
    FT_echo,
    FT_flange,
    FT_distort,
    FT_normalize,
    FT_parameq,
    FT_pitchshift,
    FT_chorus,
    FT_reverb,
    FT_compress,
  };

  struct FilterConfig {
    FilterType  _type;
    float       _a,_b,_c,_d;
    float       _e,_f,_g,_h;
  };
  
  typedef pvector<FilterConfig> ConfigVector;
  
 private:  
  void add_filter(FilterType t, float a=0, float b=0, float c=0, float d=0, float e=0, float f=0, float g=0, float h=0);
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
