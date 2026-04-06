/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToSomethingConverter.I
 * @author drose
 * @date 2012-09-26
 */

/**
 * Resets the error flag to the no-error state.  had_error() will return false
 * until a new error is generated.
 */
INLINE void EggToSomethingConverter::
clear_error() {
  _error = false;
}

/**
 * Returns true if an error was detected during the conversion process, false
 * otherwise.
 */
INLINE bool EggToSomethingConverter::
had_error() const {
  return _error;
}

/**
 * Sets the EggData to NULL and makes the converter invalid.
 */
INLINE void EggToSomethingConverter::
clear_egg_data() {
  set_egg_data(nullptr);
}

/**
 * Returns the EggData structure.
 */
INLINE EggData *EggToSomethingConverter::
get_egg_data() {
  return _egg_data;
}

/**
 * Specifies the units that the EggData has already been scaled to.  This is
 * informational only; if the target file format supports it, this information
 * will be written to the header.
 */
void EggToSomethingConverter::
set_output_units(DistanceUnit output_units) {
  _output_units = output_units;
}

/**
 * Returns the value supplied to set_output_units().
 */
DistanceUnit EggToSomethingConverter::
get_output_units() const {
  return _output_units;
}
