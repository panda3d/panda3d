/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sfxReverbDSP.I
 * @author lachbr
 * @date 2020-10-06
 */

/**
 *
 */
INLINE SFXReverbDSP::
SFXReverbDSP(float decay_time, float early_delay, float late_delay,
             float hf_reference, float hf_decay_ratio, float diffusion,
             float density, float low_shelf_frequency, float low_shelf_gain,
             float highcut, float early_late_mix, float wetlevel,
             float drylevel) :
  DSP(DSP::DT_sfxreverb),
  _decay_time(decay_time),
  _early_delay(early_delay),
  _late_delay(late_delay),
  _hf_reference(hf_reference),
  _hf_decay_ratio(hf_decay_ratio),
  _diffusion(diffusion),
  _density(density),
  _low_shelf_freq(low_shelf_frequency),
  _low_shelf_gain(low_shelf_gain),
  _highcut(highcut),
  _early_late_mix(early_late_mix),
  _wetlevel(wetlevel),
  _drylevel(drylevel)
{
}

/**
 * Sets the reverberation decay time at low-frequencies.
 * Unit: milliseconds
 */
INLINE void SFXReverbDSP::
set_decay_time(float time) {
  _decay_time = time;
  on_values_changed();
}

/**
 * Returns the reverberation decay time at low-frequencies.
 * Unit: milliseconds
 */
INLINE float SFXReverbDSP::
get_decay_time() const {
  return _decay_time;
}

/**
 * Sets the delay time of first reflection.
 * Unit: milliseconds
 */
INLINE void SFXReverbDSP::
set_early_delay(float delay) {
  _early_delay = delay;
  on_values_changed();
}

/**
 * Returns the delay time of first reflection.
 * Unit: milliseconds
 */
INLINE float SFXReverbDSP::
get_early_delay() const {
  return _early_delay;
}

/**
 * Sets the late reverberation delay time relative to first
 * reflection.
 * Unit: milliseconds
 */
INLINE void SFXReverbDSP::
set_late_delay(float delay) {
  _late_delay = delay;
  on_values_changed();
}

/**
 * Returns the late reverberation delay time relative to first
 * reflection.
 * Unit: milliseconds
 */
INLINE float SFXReverbDSP::
get_late_delay() const {
  return _late_delay;
}

/**
 * Sets the reference frequency for high-frequency delay.
 * Unit: Hertz
 */
INLINE void SFXReverbDSP::
set_hf_reference(float ref) {
  _hf_reference = ref;
  on_values_changed();
}

/**
 * Returns the reference frequency for high-frequency delay.
 * Unit: Hertz
 */
INLINE float SFXReverbDSP::
get_hf_reference() const {
  return _hf_reference;
}

/**
 * Sets the high-frequency decay time relative to decay time.
 * Unit: 0-100 percent
 */
INLINE void SFXReverbDSP::
set_hf_decay_ratio(float ratio) {
  _hf_decay_ratio = ratio;
  on_values_changed();
}

/**
 * Returns the high-frequency decay time relative to decay time.
 * Unit: 0-100 percent
 */
INLINE float SFXReverbDSP::
get_hf_decay_ratio() const {
  return _hf_decay_ratio;
}

/**
 * Sets the reverberation diffusion (echo density).
 * Unit: 0-100 percent
 */
INLINE void SFXReverbDSP::
set_diffusion(float diffusion) {
  _diffusion = diffusion;
  on_values_changed();
}

/**
 * Returns the reverberation diffusion (echo density).
 * Unit: 0-100 percent
 */
INLINE float SFXReverbDSP::
get_diffusion() const {
  return _diffusion;
}

/**
 * Sets the reverberation density (modal density).
 * Unit: 0-100 percent
 */
INLINE void SFXReverbDSP::
set_density(float density) {
  _density = density;
  on_values_changed();
}

/**
 * Returns the reverberation density (modal density).
 * Unit: 0-100 percent
 */
INLINE float SFXReverbDSP::
get_density() const {
  return _density;
}

/**
 * Sets the transition frequency of low-shelf filter.
 * Unit: Hertz
 */
INLINE void SFXReverbDSP::
set_low_shelf_frequency(float freq) {
  _low_shelf_freq = freq;
  on_values_changed();
}

/**
 * Returns the transition frequency of low-shelf filter.
 * Unit: Hertz
 */
INLINE float SFXReverbDSP::
get_low_shelf_frequency() const {
  return _low_shelf_freq;
}

/**
 * Sets the gain of low-shelf filter.
 * Unit: Decibels
 */
INLINE void SFXReverbDSP::
set_low_shelf_gain(float gain) {
  _low_shelf_gain = gain;
  on_values_changed();
}

/**
 * Returns the gain of low-shelf filter.
 * Unit: Decibels
 */
INLINE float SFXReverbDSP::
get_low_shelf_gain() const {
  return _low_shelf_gain;
}

/**
 * Sets the cutoff frequency of low-pass filter.
 * Unit: Hertz
 */
INLINE void SFXReverbDSP::
set_highcut(float highcut) {
  _highcut = highcut;
  on_values_changed();
}

/**
 * Returns the cutoff frequency of low-pass filter.
 * Unit: Hertz
 */
INLINE float SFXReverbDSP::
get_highcut() const {
  return _highcut;
}

/**
 * Sets the blend ratio of late reverb to early reflections.
 * Unit: 0-100 percent
 */
INLINE void SFXReverbDSP::
set_early_late_mix(float mix) {
  _early_late_mix = mix;
  on_values_changed();
}

/**
 * Returns the blend ratio of late reverb to early reflections.
 * Unit: 0-100 percent
 */
INLINE float SFXReverbDSP::
get_early_late_mix() const {
  return _early_late_mix;
}

/**
 * Sets the reverb signal level.
 * Unit: Decibels
 */
INLINE void SFXReverbDSP::
set_wetlevel(float level) {
  _wetlevel = level;
  on_values_changed();
}

/**
 * Returns the reverb signal level.
 * Unit: Decibels
 */
INLINE float SFXReverbDSP::
get_wetlevel() const {
  return _wetlevel;
}

/**
 * Sets the dry signal level.
 * Unit: Decibels
 */
INLINE void SFXReverbDSP::
set_drylevel(float level) {
  _drylevel = level;
  on_values_changed();
}

/**
 * Returns the dry signal level.
 * Unit: Decibels
 */
INLINE float SFXReverbDSP::
get_drylevel() const {
  return _drylevel;
}

/**
 * Returns a generic SFX reverb preset.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_generic() {
  return new SFXReverbDSP(1500, 7, 11, 5000, 83, 100, 100, 250, 0, 14500, 96, -8.0f);
}

/**
 * Returns an SFX reverb configured for a padded cell environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_padded_cell() {
  return new SFXReverbDSP(170, 1, 2, 5000, 10, 100, 100, 250, 0, 160, 84, -7.8f);
}

/**
 * Returns an SFX reverb configured for a room environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_room() {
  return new SFXReverbDSP(400, 2, 3, 5000, 83, 100, 100, 250, 0, 6050, 88, -9.4f);
}

/**
 * Returns an SFX reverb configured for a bathroom environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_bathroom() {
  return new SFXReverbDSP(1500, 7, 11, 5000, 54, 100, 60, 250, 0, 2900, 83, 0.5f);
}

/**
 * Returns an SFX reverb configured for a living room environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_living_room() {
  return new SFXReverbDSP(500, 3, 4, 5000, 10, 100, 100, 250, 0, 160, 58, -19.0f);
}

/**
 * Returns an SFX reverb configured for a stone room environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_stone_room() {
  return new SFXReverbDSP(2300, 12, 17, 5000, 64, 100, 100, 250, 0, 7800, 71, -8.5f);
}

/**
 * Returns an SFX reverb configured for an auditorium environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_auditorium() {
  return new SFXReverbDSP(4300, 20, 30, 5000, 59, 100, 100, 250, 0, 5850, 64, -11.7f);
}

/**
 * Returns an SFX reverb configured for a concert hall environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_concert_hall() {
  return new SFXReverbDSP(3900, 20, 29, 5000, 70, 100, 100, 250, 0, 5650, 80, -9.8f);
}

/**
 * Return an SFX reverb configured for a cave environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_cave() {
  return new SFXReverbDSP(2900, 15, 22, 5000, 100, 100, 100, 250, 0, 20000, 59, -11.3f);
}

/**
 * Return an SFX reverb configured for an arena environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_arena() {
  return new SFXReverbDSP(7200, 20, 30, 5000, 33, 100, 100, 250, 0, 4500, 80, -9.6f);
}

/**
 * Return an SFX reverb configured for a hangar environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_hangar() {
  return new SFXReverbDSP(10000, 20, 30, 5000, 23, 100, 100, 250, 0, 3400, 72, -7.4f);
}

/**
 * Returns an SFX reverb configured for a carpetted-hallway environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_carpetted_hallway() {
  return new SFXReverbDSP(300, 2, 30, 5000, 10, 100, 100, 250, 0, 500, 56, -24.0f);
}

/**
 * Returns an SFX reverb configured for a hallway environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_hallway() {
  return new SFXReverbDSP(1500, 7, 11, 5000, 59, 100, 100, 250, 0, 7800, 87, -5.5f);
}

/**
 * Returns an SFX reverb configured for a stone corridor environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_stone_corridor() {
  return new SFXReverbDSP(270, 13, 20, 5000, 79, 100, 100, 250, 0, 9000, 86, -6.0f);
}

/**
 * Returns an SFX reverb configured for an alley environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_alley() {
  return new SFXReverbDSP(1500, 7, 11, 5000, 86, 100, 100, 250, 0, 8300, 80, -9.8f);
}

/**
 * Returns an SFX reverb configured for a forest environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_forest() {
  return new SFXReverbDSP(1500, 162, 88, 5000, 54, 79, 100, 250, 0, 760, 94, -12.3f);
}

/**
 * Returns an SFX reverb configured for a city environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_city() {
  return new SFXReverbDSP(1500, 7, 11, 5000, 67, 50, 100, 250, 0, 4050, 66, -26.0f);
}

/**
 * Returns an SFX reverb configured for a mountainous environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_mountains() {
  return new SFXReverbDSP(1500, 300, 100, 5000, 21, 27, 100, 250, 0, 1220, 82, -24.0f);
}

/**
 * Returns an SFX reverb configured for a quarry environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_quarry() {
  return new SFXReverbDSP(1500, 61, 25, 5000, 83, 100, 100, 250, 0, 3400, 100, -5.0f);
}

/**
 * Returns an SFX reverb configured for a plain environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_plain() {
  return new SFXReverbDSP(1500, 179, 100, 5000, 50, 21, 100, 250, 0, 1670, 65, -28.0f);
}

/**
 * Returns an SFX reverb configured for a parking lot environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_parking_lot() {
  return new SFXReverbDSP(1700, 8, 12, 5000, 100, 100, 100, 250, 0, 20000, 56, -19.5f);
}

/**
 * Returns an SFX reverb configured for a sewer pipe environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_sewer_pipe() {
  return new SFXReverbDSP(2800, 14, 21, 5000, 14, 80, 60, 250, 0, 3400, 66, 1.2f);
}

/**
 * Returns an SFX reverb configured for an underwater environment.
 */
INLINE PT(SFXReverbDSP) SFXReverbDSP::
get_underwater() {
  return new SFXReverbDSP(1500, 7, 11, 5000, 10, 100, 100, 250, 0, 500, 92, 7.0f);
}
