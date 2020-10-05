/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file oscillatorDSP.h
 * @author lachbr
 * @date 2020-10-04
 */

#ifndef OSCILLATORDSP_H
#define OSCILLATORDSP_H

#include "dsp.h"

/**
 * DSP filter that generates sine/square/saw/triangle or noise tones.
 */
class EXPCL_PANDA_AUDIO OscillatorDSP : public DSP {
PUBLISHED:
  enum OscillatorType {
    OT_sine,
    OT_square,
    OT_sawup,
    OT_sawdown,
    OT_triangle,
    OT_noise,
  };

  INLINE OscillatorDSP(OscillatorType type = OT_sine, float rate = 220);

  INLINE void set_oscillator_type(OscillatorType type);
  INLINE OscillatorType get_oscillator_type() const;
  MAKE_PROPERTY(oscillator_type, get_oscillator_type, set_oscillator_type);

  INLINE void set_rate(float rate);
  INLINE float get_rate() const;
  MAKE_PROPERTY(rate, get_rate, set_rate);

private:
  OscillatorType _osc_type;
  float _rate;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DSP::init_type();
    register_type(_type_handle, "OscillatorDSP",
                  DSP::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "oscillatorDSP.I"

#endif // OSCILLATORDSP_H
