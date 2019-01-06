/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file somethingToEggConverter.I
 * @author drose
 * @date 2001-04-26
 */

/**
 * Resets the error flag to the no-error state.  had_error() will return false
 * until a new error is generated.
 */
INLINE void SomethingToEggConverter::
clear_error() {
  _error = false;
}

/**
 * Returns true if an error was detected during the conversion process (unless
 * _allow_errors is true), false otherwise.
 */
INLINE bool SomethingToEggConverter::
had_error() const {
  return !_allow_errors && (_error || _path_replace->had_error());
}

/**
 * Replaces the PathReplace object (which specifies how to mangle paths from
 * the source to the destination egg file) with a new one.
 */
INLINE void SomethingToEggConverter::
set_path_replace(PathReplace *path_replace) {
  _path_replace = path_replace;
}

/**
 * Returns a pointer to the PathReplace object associated with this converter.
 * If the converter is non-const, this returns a non-const pointer, which can
 * be adjusted.
 */
INLINE PathReplace *SomethingToEggConverter::
get_path_replace() {
  return _path_replace;
}

/**
 * Returns a pointer to the PathReplace object associated with this converter.
 * If the converter is non-const, this returns a non-const pointer, which can
 * be adjusted.
 */
INLINE const PathReplace *SomethingToEggConverter::
get_path_replace() const {
  return _path_replace;
}

/**
 * Specifies how source animation will be converted into egg structures.  The
 * default is AC_none, which means animation tables will be ignored.  This is
 * only meaningful for converters that understand animation.
 */
INLINE void SomethingToEggConverter::
set_animation_convert(AnimationConvert animation_convert) {
  _animation_convert = animation_convert;
}

/**
 * Returns how source animation will be converted into egg structures.
 */
INLINE AnimationConvert SomethingToEggConverter::
get_animation_convert() const {
  return _animation_convert;
}

/**
 * Specifies the name of the character generated.  This name should match
 * between all the model and channel egg files for a particular character and
 * its associated animations.
 */
INLINE void SomethingToEggConverter::
set_character_name(const std::string &character_name) {
  _character_name = character_name;
}

/**
 * Returns the name of the character generated.  See set_character_name().
 */
INLINE const std::string &SomethingToEggConverter::
get_character_name() const {
  return _character_name;
}

/**
 * Specifies the starting frame of the animation to convert, in the units
 * specified by set_input_frame_rate().  If this is unspecified, the starting
 * frame is taken from the source, for instance from the first frame of the
 * animation slider.
 */
INLINE void SomethingToEggConverter::
set_start_frame(double start_frame) {
  _start_frame = start_frame;
  _control_flags |= CF_start_frame;
}

/**
 * Returns true if the starting frame has been explicitly specified via
 * set_start_frame(), or false if the starting frame should be implicit based
 * on the source.
 */
INLINE bool SomethingToEggConverter::
has_start_frame() const {
  return (_control_flags & CF_start_frame) != 0;
}

/**
 * Returns the value set by a previous call to set_start_frame().  It is an
 * error to call this if has_start_frame() returns false.
 */
INLINE double SomethingToEggConverter::
get_start_frame() const {
  nassertr(has_start_frame(), 0.0);
  return _start_frame;
}

/**
 * Removes the value previously set by set_start_frame().
 */
INLINE void SomethingToEggConverter::
clear_start_frame() {
  _start_frame = 0.0;
  _control_flags &= ~CF_start_frame;
}

/**
 * Specifies the ending frame of the animation to convert, in the units
 * specified by set_input_frame_rate().  If this is unspecified, the ending
 * frame is taken from the source, for instance from the last frame of the
 * animation slider.
 */
INLINE void SomethingToEggConverter::
set_end_frame(double end_frame) {
  _end_frame = end_frame;
  _control_flags |= CF_end_frame;
}

/**
 * Returns true if the ending frame has been explicitly specified via
 * set_end_frame(), or false if the ending frame should be implicit based on
 * the source.
 */
INLINE bool SomethingToEggConverter::
has_end_frame() const {
  return (_control_flags & CF_end_frame) != 0;
}

/**
 * Returns the value set by a previous call to set_end_frame().  It is an
 * error to call this if has_end_frame() returns false.
 */
INLINE double SomethingToEggConverter::
get_end_frame() const {
  nassertr(has_end_frame(), 0.0);
  return _end_frame;
}

/**
 * Removes the value previously set by set_end_frame().
 */
INLINE void SomethingToEggConverter::
clear_end_frame() {
  _end_frame = 0.0;
  _control_flags &= ~CF_end_frame;
}

/**
 * Specifies the increment between frames to extract.  This is the amount to
 * increment the time slider (in units of internal_frame_rate) between
 * extracting each frame.  If this is not specified, the default is taken from
 * the animation package, or 1.0 if the animation package does not specified a
 * frame increment.
 */
INLINE void SomethingToEggConverter::
set_frame_inc(double frame_inc) {
  _frame_inc = frame_inc;
  _control_flags |= CF_frame_inc;
}

/**
 * Returns true if the frame increment has been explicitly specified via
 * set_frame_inc(), or false if the ending frame should be implicit based on
 * the source.
 */
INLINE bool SomethingToEggConverter::
has_frame_inc() const {
  return (_control_flags & CF_frame_inc) != 0;
}

/**
 * Returns the value set by a previous call to set_frame_inc().  It is an
 * error to call this if has_frame_inc() returns false.
 */
INLINE double SomethingToEggConverter::
get_frame_inc() const {
  nassertr(has_frame_inc(), 0.0);
  return _frame_inc;
}

/**
 * Removes the value previously set by set_frame_inc().
 */
INLINE void SomethingToEggConverter::
clear_frame_inc() {
  _frame_inc = 0.0;
  _control_flags &= ~CF_frame_inc;
}

/**
 * Specifies the frame of animation to represent the neutral pose of the
 * model.
 */
INLINE void SomethingToEggConverter::
set_neutral_frame(double neutral_frame) {
  _neutral_frame = neutral_frame;
  _control_flags |= CF_neutral_frame;
}

/**
 * Returns true if the neutral frame has been explicitly specified via
 * set_neutral_frame(), or false otherwise.
 */
INLINE bool SomethingToEggConverter::
has_neutral_frame() const {
  return (_control_flags & CF_neutral_frame) != 0;
}

/**
 * Returns the value set by a previous call to set_neutral_frame().  It is an
 * error to call this if has_neutral_frame() returns false.
 */
INLINE double SomethingToEggConverter::
get_neutral_frame() const {
  nassertr(has_neutral_frame(), 0.0);
  return _neutral_frame;
}

/**
 * Removes the value previously set by set_neutral_frame().
 */
INLINE void SomethingToEggConverter::
clear_neutral_frame() {
  _neutral_frame = 0.0;
  _control_flags &= ~CF_neutral_frame;
}

/**
 * Specifies the number of frames per second that is represented by the
 * "frame" unit in the animation package.  If this is omitted, it is taken
 * from whatever the file header indicates.  Some animation packages do not
 * encode a frame rate, in which case the default if this is omitted is the
 * same as the output frame rate.
 */
INLINE void SomethingToEggConverter::
set_input_frame_rate(double input_frame_rate) {
  _input_frame_rate = input_frame_rate;
  _control_flags |= CF_input_frame_rate;
}

/**
 * Returns true if the frame rate has been explicitly specified via
 * set_input_frame_rate(), or false otherwise.
 */
INLINE bool SomethingToEggConverter::
has_input_frame_rate() const {
  return (_control_flags & CF_input_frame_rate) != 0;
}

/**
 * Returns the value set by a previous call to set_input_frame_rate().  It is
 * an error to call this if has_input_frame_rate() returns false.
 */
INLINE double SomethingToEggConverter::
get_input_frame_rate() const {
  nassertr(has_input_frame_rate(), 0.0);
  return _input_frame_rate;
}

/**
 * Removes the value previously set by set_input_frame_rate().
 */
INLINE void SomethingToEggConverter::
clear_input_frame_rate() {
  _input_frame_rate = 0.0;
  _control_flags &= ~CF_input_frame_rate;
}

/**
 * Specifies the number of frames per second that the resulting animation
 * should be played at.  If this is omitted, it is taken to be the same as the
 * input frame rate.
 */
INLINE void SomethingToEggConverter::
set_output_frame_rate(double output_frame_rate) {
  _output_frame_rate = output_frame_rate;
  _control_flags |= CF_output_frame_rate;
}

/**
 * Returns true if the frame rate has been explicitly specified via
 * set_output_frame_rate(), or false otherwise.
 */
INLINE bool SomethingToEggConverter::
has_output_frame_rate() const {
  return (_control_flags & CF_output_frame_rate) != 0;
}

/**
 * Returns the value set by a previous call to set_output_frame_rate().  It is
 * an error to call this if has_output_frame_rate() returns false.
 */
INLINE double SomethingToEggConverter::
get_output_frame_rate() const {
  nassertr(has_output_frame_rate(), 0.0);
  return _output_frame_rate;
}

/**
 * Removes the value previously set by set_output_frame_rate().
 */
INLINE void SomethingToEggConverter::
clear_output_frame_rate() {
  _output_frame_rate = 0.0;
  _control_flags &= ~CF_output_frame_rate;
}

/**
 * Returns the default frame rate if nothing is specified for input_frame_rate
 * or output_frame_rate, and the animation package does not have an implicit
 * frame rate.
 */
INLINE double SomethingToEggConverter::
get_default_frame_rate() {
  return 24.0;
}

/**
 * Sets the merge_externals flag.  When this is true, external references
 * within the source file are read in and merged directly; otherwise, only a
 * reference to a similarly-named egg file is inserted.
 */
INLINE void SomethingToEggConverter::
set_merge_externals(bool merge_externals) {
  _merge_externals = merge_externals;
}

/**
 * Returns the current state of the merge_externals flag.  See
 * set_merge_externals().
 */
INLINE bool SomethingToEggConverter::
get_merge_externals() const {
  return _merge_externals;
}

/**
 * Sets the EggData to NULL and makes the converter invalid.
 */
INLINE void SomethingToEggConverter::
clear_egg_data() {
  set_egg_data(nullptr);
}

/**
 * Returns the EggData structure.
 */
INLINE EggData *SomethingToEggConverter::
get_egg_data() {
  return _egg_data;
}

/**
 * Converts the indicated model filename to a relative or absolute or whatever
 * filename, according to _path_replace.
 */
INLINE Filename SomethingToEggConverter::
convert_model_path(const Filename &orig_filename) {
  return _path_replace->convert_path(orig_filename);
}
