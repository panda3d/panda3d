/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file filterProperties.cxx
 * @author jyelon
 * @date 2007-08-01
 */

#include "filterProperties.h"
#include "lowpassDSP.h"
#include "highpassDSP.h"
#include "echoDSP.h"
#include "flangeDSP.h"
#include "distortionDSP.h"
#include "normalizeDSP.h"
#include "paramEQDSP.h"
#include "pitchShiftDSP.h"
#include "chorusDSP.h"
#include "sfxReverbDSP.h"
#include "compressorDSP.h"

TypeHandle FilterProperties::_type_handle;

/**
 *
 */
FilterProperties::
FilterProperties()
{
}

/**
 *
 */
FilterProperties::
~FilterProperties() {
}

/**
 * Add a lowpass filter to the end of the DSP chain.
 */
void FilterProperties::
add_lowpass(PN_stdfloat cutoff_freq, PN_stdfloat resonance_q) {
  PT(LowpassDSP) filter = new LowpassDSP(cutoff_freq, resonance_q);
  add_filter(filter);
}

/**
 * Add a highpass filter to the end of the DSP chain.
 */
void FilterProperties::
add_highpass(PN_stdfloat cutoff_freq, PN_stdfloat resonance_q) {
  PT(HighpassDSP) filter = new HighpassDSP(cutoff_freq, resonance_q);
  add_filter(filter);
}

/**
 * Add a echo filter to the end of the DSP chain.
 */
void FilterProperties::
add_echo(PN_stdfloat drymix, PN_stdfloat wetmix, PN_stdfloat delay, PN_stdfloat decayratio) {
  PT(EchoDSP) filter = new EchoDSP(delay, decayratio * 100,
                                   log10(std::max((PN_stdfloat)0.0001, drymix)) * 20,
                                   log10(std::max((PN_stdfloat)0.0001, wetmix)) * 20);
  add_filter(filter);
}

/**
 * Add a flange filter to the end of the DSP chain.
 */
void FilterProperties::
add_flange(PN_stdfloat drymix, PN_stdfloat wetmix, PN_stdfloat depth, PN_stdfloat rate) {
  PT(FlangeDSP) filter = new FlangeDSP(wetmix * 100, depth, rate);
  add_filter(filter);
}

/**
 * Add a distort filter to the end of the DSP chain.
 */
void FilterProperties::
add_distort(PN_stdfloat level) {
  PT(DistortionDSP) filter = new DistortionDSP(level);
  add_filter(filter);
}

/**
 * Add a normalize filter to the end of the DSP chain.
 */
void FilterProperties::
add_normalize(PN_stdfloat fadetime, PN_stdfloat threshold, PN_stdfloat maxamp) {
  PT(NormalizeDSP) filter = new NormalizeDSP(fadetime, threshold, maxamp);
  add_filter(filter);
}

/**
 * Add a parameq filter to the end of the DSP chain.
 */
void FilterProperties::
add_parameq(PN_stdfloat center_freq, PN_stdfloat bandwidth, PN_stdfloat gain) {
  PT(ParamEQDSP) filter = new ParamEQDSP(center_freq, bandwidth, gain);
  add_filter(filter);
}

/**
 * Add a pitchshift filter to the end of the DSP chain.
 */
void FilterProperties::
add_pitchshift(PN_stdfloat pitch, PN_stdfloat fftsize, PN_stdfloat overlap) {
  PT(PitchShiftDSP) filter = new PitchShiftDSP(pitch, (int)fftsize);
  add_filter(filter);
}

/**
 * Add a chorus filter to the end of the DSP chain.
 */
void FilterProperties::
add_chorus(PN_stdfloat drymix, PN_stdfloat wet1, PN_stdfloat wet2, PN_stdfloat wet3, PN_stdfloat delay, PN_stdfloat rate, PN_stdfloat depth) {
  PT(ChorusDSP) filter = new ChorusDSP(drymix * 100, rate, depth);
  add_filter(filter);
}

/**
 * Add a reverb filter to the end of the DSP chain.
 */
void FilterProperties::
add_sfxreverb(PN_stdfloat drylevel, PN_stdfloat room, PN_stdfloat roomhf, PN_stdfloat decaytime,
              PN_stdfloat decayhfratio, PN_stdfloat reflectionslevel, PN_stdfloat reflectionsdelay,
              PN_stdfloat reverblevel, PN_stdfloat reverbdelay, PN_stdfloat diffusion,
              PN_stdfloat density, PN_stdfloat hfreference, PN_stdfloat roomlf, PN_stdfloat lfreference) {
  PN_stdfloat late_early_ratio = pow(10, (reverblevel - reflectionslevel) / 2000);
  PN_stdfloat early_and_late_power = pow(10, reflectionslevel / 1000) + pow(10, reverblevel / 1000);
  PN_stdfloat hf_gain = pow(10, roomhf / 2000);
  PN_stdfloat highcut = (roomhf < 0) ? (hfreference / sqrt((1 - hf_gain) / hf_gain)) : 20000;
  PN_stdfloat early_late_mix = (reflectionslevel > -10000) ? (late_early_ratio / (late_early_ratio + 1) * 100) : 100;
  PN_stdfloat wetlevel = 10 * log10(early_and_late_power) + room / 100;
  PT(SFXReverbDSP) filter = new SFXReverbDSP(decaytime * 1000, reflectionsdelay * 1000, reverbdelay * 1000,
                                             hfreference, decayhfratio * 100, diffusion, density,
                                             lfreference, roomlf / 100, highcut, early_late_mix,
                                             wetlevel, drylevel / 100);
  add_filter(filter);
}

/**
 * Add a compress filter to the end of the DSP chain.
 */
void FilterProperties::
add_compress(PN_stdfloat threshold, PN_stdfloat attack, PN_stdfloat release, PN_stdfloat gainmakeup) {
  PT(CompressorDSP) filter = new CompressorDSP(threshold, attack, release, gainmakeup);
  add_filter(filter);
}
