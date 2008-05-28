// Filename: filterProperties.I
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


////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::clear
//      Access: Published
// Description: Removes all DSP postprocessing.
////////////////////////////////////////////////////////////////////
INLINE void FilterProperties::
clear() {
  _config.clear();
}

////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::apply_lowpass
//      Access: Published
// Description: Add a lowpass filter to the end of the DSP chain.
////////////////////////////////////////////////////////////////////
INLINE void FilterProperties::
add_lowpass(float cutoff_freq, float resonance_q) {
  add_filter(FT_lowpass, cutoff_freq, resonance_q);
}

////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::add_highpass
//      Access: Published
// Description: Add a highpass filter to the end of the DSP chain.
////////////////////////////////////////////////////////////////////
INLINE void FilterProperties::
add_highpass(float cutoff_freq, float resonance_q) {
  add_filter(FT_highpass, cutoff_freq, resonance_q);
}

////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::add_echo
//      Access: Published
// Description: Add a echo filter to the end of the DSP chain.
////////////////////////////////////////////////////////////////////
INLINE void FilterProperties::
add_echo(float drymix, float wetmix, float delay, float decayratio) {
  add_filter(FT_echo, drymix, wetmix, delay, decayratio);
}

////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::add_flange
//      Access: Published
// Description: Add a flange filter to the end of the DSP chain.
////////////////////////////////////////////////////////////////////
INLINE void FilterProperties::
add_flange(float drymix, float wetmix, float depth, float rate) {
  add_filter(FT_flange, drymix, wetmix, depth, rate);
}

////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::add_distort
//      Access: Published
// Description: Add a distort filter to the end of the DSP chain.
////////////////////////////////////////////////////////////////////
INLINE void FilterProperties::
add_distort(float level) {
  add_filter(FT_distort, level);
}

////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::add_normalize
//      Access: Published
// Description: Add a normalize filter to the end of the DSP chain.
////////////////////////////////////////////////////////////////////
INLINE void FilterProperties::
add_normalize(float fadetime, float threshold, float maxamp) {
  add_filter(FT_normalize, fadetime, threshold, maxamp);
}

////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::add_parameq
//      Access: Published
// Description: Add a parameq filter to the end of the DSP chain.
////////////////////////////////////////////////////////////////////
INLINE void FilterProperties::
add_parameq(float center_freq, float bandwidth, float gain) {
  add_filter(FT_parameq, center_freq, bandwidth, gain);
}

////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::add_pitchshift
//      Access: Published
// Description: Add a pitchshift filter to the end of the DSP chain.
////////////////////////////////////////////////////////////////////
INLINE void FilterProperties::
add_pitchshift(float pitch, float fftsize, float overlap) {
  add_filter(FT_pitchshift, pitch, fftsize, overlap);
}

////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::add_chorus
//      Access: Published
// Description: Add a chorus filter to the end of the DSP chain.
////////////////////////////////////////////////////////////////////
INLINE void FilterProperties::
add_chorus(float drymix, float wet1, float wet2, float wet3, float delay, float rate, float depth, float feedback) {
  add_filter(FT_chorus, drymix, wet1, wet2, wet3, delay, rate, depth, feedback);
}

////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::add_reverb
//      Access: Published
// Description: Add a reverb filter to the end of the DSP chain.
////////////////////////////////////////////////////////////////////
INLINE void FilterProperties::
add_reverb(float drymix, float wetmix, float roomsize, float damp, float width) {
  add_filter(FT_reverb, drymix, wetmix, roomsize, damp, width);
}

////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::add_compress
//      Access: Published
// Description: Add a compress filter to the end of the DSP chain.
////////////////////////////////////////////////////////////////////
INLINE void FilterProperties::
add_compress(float threshold, float attack, float release, float gainmakeup) {
  add_filter(FT_compress, threshold, attack, release, gainmakeup);
}

////////////////////////////////////////////////////////////////////
//    Function: FilterProperties::get_config
//      Access: Published
// Description: Intended for use by AudioManager and AudioSound
//              implementations: allows access to the config vector.
////////////////////////////////////////////////////////////////////
INLINE const FilterProperties::ConfigVector &FilterProperties::
get_config() {
  return _config;
}


