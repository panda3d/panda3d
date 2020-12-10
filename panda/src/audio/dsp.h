/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dsp.h
 * @author lachbr
 * @date 2020-10-04
 */

#ifndef DSP_H
#define DSP_H

#include "config_audio.h"
#include "typedReferenceCount.h"

/**
 * This is an abstract base class for a single DSP filter that can be inserted
 * into a DSP chain.
 */
class EXPCL_PANDA_AUDIO DSP : public TypedReferenceCount {
PUBLISHED:
  enum DSPType {
    DT_unknown = -1,
    DT_chorus,
    DT_compressor,
    DT_distortion,
    DT_echo,
    DT_fader,
    DT_flange,
    DT_highpass,
    DT_limiter,
    DT_lowpass,
    DT_normalize,
    DT_oscillator,
    DT_parameq,
    DT_pitchshift,
    DT_sfxreverb,
  };

  INLINE DSPType get_dsp_type() const;
  MAKE_PROPERTY(dsp_type, get_dsp_type);

protected:
  DSP(DSPType type);

  INLINE void on_values_changed();

public:
  virtual ~DSP();

  INLINE bool is_dirty() const;
  INLINE void clear_dirty();

protected:
  DSPType _dsp_type;
  bool _dirty;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "DSP",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dsp.I"

#endif // DSP_H
