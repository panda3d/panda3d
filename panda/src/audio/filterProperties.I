/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file filterProperties.I
 * @author jyelon
 * @date 2007-08-01
 */

/**
 * Removes all DSP postprocessing.
 */
INLINE void FilterProperties::
clear() {
  _config.clear();
}

/**
 * Add a lowpass filter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_lowpass(PN_stdfloat cutoff_freq, PN_stdfloat resonance_q) {
  add_filter(FT_lowpass, cutoff_freq, resonance_q);
}

/**
 * Add a highpass filter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_highpass(PN_stdfloat cutoff_freq, PN_stdfloat resonance_q) {
  add_filter(FT_highpass, cutoff_freq, resonance_q);
}

/**
 * Add a echo filter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_echo(PN_stdfloat drymix, PN_stdfloat wetmix, PN_stdfloat delay, PN_stdfloat decayratio) {
  add_filter(FT_echo, drymix, wetmix, delay, decayratio);
}

/**
 * Add a flange filter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_flange(PN_stdfloat drymix, PN_stdfloat wetmix, PN_stdfloat depth, PN_stdfloat rate) {
  add_filter(FT_flange, drymix, wetmix, depth, rate);
}

/**
 * Add a distort filter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_distort(PN_stdfloat level) {
  add_filter(FT_distort, level);
}

/**
 * Add a normalize filter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_normalize(PN_stdfloat fadetime, PN_stdfloat threshold, PN_stdfloat maxamp) {
  add_filter(FT_normalize, fadetime, threshold, maxamp);
}

/**
 * Add a parameq filter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_parameq(PN_stdfloat center_freq, PN_stdfloat bandwidth, PN_stdfloat gain) {
  add_filter(FT_parameq, center_freq, bandwidth, gain);
}

/**
 * Add a pitchshift filter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_pitchshift(PN_stdfloat pitch, PN_stdfloat fftsize, PN_stdfloat overlap) {
  add_filter(FT_pitchshift, pitch, fftsize, overlap);
}

/**
 * Add a chorus filter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_chorus(PN_stdfloat drymix, PN_stdfloat wet1, PN_stdfloat wet2, PN_stdfloat wet3, PN_stdfloat delay, PN_stdfloat rate, PN_stdfloat depth) {
  add_filter(FT_chorus, drymix, wet1, wet2, wet3, delay, rate, depth);
}

/**
 * Add a reverb filter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_sfxreverb(PN_stdfloat drylevel, PN_stdfloat room, PN_stdfloat roomhf, PN_stdfloat decaytime,
              PN_stdfloat decayhfratio, PN_stdfloat reflectionslevel, PN_stdfloat reflectionsdelay,
              PN_stdfloat reverblevel, PN_stdfloat reverbdelay, PN_stdfloat diffusion,
              PN_stdfloat density, PN_stdfloat hfreference, PN_stdfloat roomlf, PN_stdfloat lfreference) {
  add_filter(FT_sfxreverb, drylevel, room, roomhf, decaytime, decayhfratio, reflectionslevel, reflectionsdelay,
                           reverblevel, reverbdelay, diffusion, density, hfreference, roomlf, lfreference);
}

/**
 * Add a compress filter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_compress(PN_stdfloat threshold, PN_stdfloat attack, PN_stdfloat release, PN_stdfloat gainmakeup) {
  add_filter(FT_compress, threshold, attack, release, gainmakeup);
}

/**
 * Add a fader to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_fader(PN_stdfloat gain) {
  add_filter(FT_fader, gain);
}

/**
 * Add a limiter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_limiter(PN_stdfloat releasetime, PN_stdfloat ceiling,
            PN_stdfloat maximizergain, PN_stdfloat mode) {
  add_filter(FT_limiter, releasetime, ceiling, maximizergain, mode);
}

/**
 * Add a pan filter to the end of the DSP chain.
 *
 * mode:                0=mono, 1=stereo, 2=surround (default: surround)
 * stereo_position:     -100 (full left) to 100 (full right)
 * direction:           surround pan direction in degrees [-180, 180]
 * extent_2d:           surround pan extent in degrees [0, 360]
 * rotation:            surround pan rotation in degrees [-180, 180]
 * lfe_level:           LFE / subwoofer level in dB [-80, 20]
 * stereo_mode:         0=distributed, 1=discrete
 * stereo_separation:   stereo separation in degrees [-180, 180] (discrete mode)
 * stereo_axis:         stereo axis in degrees [-180, 180] (discrete mode)
 * enabled_speakers:    bitmask of active speakers [0, 0xFFF]; default 0xFFF (all)
 * pos_3d_x/y/z:        relative 3D position of the sound source
 * rolloff:             0=linearsquared, 1=linear, 2=inverse, 3=inversetapered, 4=custom
 * min_distance:        3D min attenuation distance (when override_range=1)
 * max_distance:        3D max attenuation distance (when override_range=1)
 * extent_mode:         0=auto, 1=user, 2=off
 * sound_size:          extent of the sound source in distance units
 * min_extent:          minimum extent in degrees [0, 360]
 * pan_blend:           2D/3D pan blend [0, 1]; 0=full 3D, 1=full 2D
 * lfe_upmix_enabled:   0=off, 1=on; upmix non-LFE signal to LFE channel
 * surround_speaker_mode: FMOD speaker mode for surround (0=default system mode)
 * height_blend:        blend between ground and top speakers [-1, 1]
 * override_range:      1=use min/max_distance, 0=use FMOD attenuation range
 */
INLINE void FilterProperties::
add_pan(int mode, PN_stdfloat stereo_position,
        PN_stdfloat direction, PN_stdfloat extent_2d,
        PN_stdfloat rotation, PN_stdfloat lfe_level,
        int stereo_mode, PN_stdfloat stereo_separation,
        PN_stdfloat stereo_axis, int enabled_speakers,
        PN_stdfloat pos_3d_x, PN_stdfloat pos_3d_y, PN_stdfloat pos_3d_z,
        int rolloff, PN_stdfloat min_distance, PN_stdfloat max_distance,
        int extent_mode, PN_stdfloat sound_size, PN_stdfloat min_extent,
        PN_stdfloat pan_blend, bool lfe_upmix_enabled,
        int surround_speaker_mode, PN_stdfloat height_blend,
        bool override_range) {
  add_filter(FT_pan,
             mode, stereo_position, direction, extent_2d,
             rotation, lfe_level, stereo_mode, stereo_separation,
             stereo_axis, enabled_speakers, pos_3d_x, pos_3d_y,
             pos_3d_z, rolloff, min_distance, max_distance,
             extent_mode, sound_size, min_extent, pan_blend,
             lfe_upmix_enabled, surround_speaker_mode, height_blend,
             override_range);
}

/**
 * Add a tremolo filter to the end of the DSP chain.
 */
INLINE void FilterProperties::
add_tremolo(PN_stdfloat frequency, PN_stdfloat depth, PN_stdfloat shape,
            PN_stdfloat skew, PN_stdfloat duty, PN_stdfloat square,
            PN_stdfloat phase, PN_stdfloat spread) {
  add_filter(FT_tremolo, frequency, depth, shape, skew, duty, square, phase, spread);
}

/**
 * Add a delay filter to the end of the DSP chain.
 * ch0 and ch1 are the left and right channel delays in milliseconds.
 * maxdelay sets the maximum allowable delay and must be >= ch0 and ch1.
 */
INLINE void FilterProperties::
add_delay(PN_stdfloat ch0, PN_stdfloat ch1, PN_stdfloat maxdelay) {
  add_filter(FT_delay, ch0, ch1, maxdelay);
}

/**
 * Intended for use by AudioManager and AudioSound implementations: allows
 * access to the config vector.
 */
INLINE const FilterProperties::ConfigVector &FilterProperties::
get_config() {
  return _config;
}