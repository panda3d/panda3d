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
  INLINE void add_lowpass(PN_stdfloat cutoff_freq, PN_stdfloat resonance_q);
  INLINE void add_highpass(PN_stdfloat cutoff_freq, PN_stdfloat resonance_q);
  INLINE void add_echo(PN_stdfloat drymix, PN_stdfloat wetmix, PN_stdfloat delay, PN_stdfloat decayratio);
  INLINE void add_flange(PN_stdfloat drymix, PN_stdfloat wetmix, PN_stdfloat depth, PN_stdfloat rate);
  INLINE void add_distort(PN_stdfloat level);
  INLINE void add_normalize(PN_stdfloat fadetime, PN_stdfloat threshold, PN_stdfloat maxamp);
  INLINE void add_parameq(PN_stdfloat center_freq, PN_stdfloat bandwidth, PN_stdfloat gain);
  INLINE void add_pitchshift(PN_stdfloat pitch, PN_stdfloat fftsize, PN_stdfloat overlap);
  INLINE void add_chorus(PN_stdfloat drymix, PN_stdfloat wet1, PN_stdfloat wet2, PN_stdfloat wet3, PN_stdfloat delay, PN_stdfloat rate, PN_stdfloat depth);
  INLINE void add_sfxreverb(PN_stdfloat drylevel=0, PN_stdfloat room=-10000, PN_stdfloat roomhf=0, PN_stdfloat decaytime=1,
  	                        PN_stdfloat decayhfratio=0.5, PN_stdfloat reflectionslevel=-10000, PN_stdfloat reflectionsdelay=0.02,
  	                        PN_stdfloat reverblevel=0, PN_stdfloat reverbdelay=0.04, PN_stdfloat diffusion=100,
  	                        PN_stdfloat density=100, PN_stdfloat hfreference=5000, PN_stdfloat roomlf=0, PN_stdfloat lfreference=250);
  INLINE void add_compress(PN_stdfloat threshold, PN_stdfloat attack, PN_stdfloat release, PN_stdfloat gainmakeup);

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
    FT_sfxreverb,
    FT_compress,
  };

  struct FilterConfig {
    FilterType  _type;
    PN_stdfloat       _a,_b,_c,_d;
    PN_stdfloat       _e,_f,_g,_h;
    PN_stdfloat       _i,_j,_k,_l;
    PN_stdfloat       _m,_n;
  };
  
  typedef pvector<FilterConfig> ConfigVector;
  
 private:  
  void add_filter(FilterType t, PN_stdfloat a=0, PN_stdfloat b=0, PN_stdfloat c=0, PN_stdfloat d=0,
  	                            PN_stdfloat e=0, PN_stdfloat f=0, PN_stdfloat g=0, PN_stdfloat h=0,
  	                            PN_stdfloat i=0, PN_stdfloat j=0, PN_stdfloat k=0, PN_stdfloat l=0,
  	                            PN_stdfloat m=0, PN_stdfloat n=0);
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
