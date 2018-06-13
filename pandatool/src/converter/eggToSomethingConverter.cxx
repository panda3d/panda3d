/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToSomethingConverter.cxx
 * @author drose
 * @date 2001-04-26
 */

#include "eggToSomethingConverter.h"

#include "eggData.h"

/**
 *
 */
EggToSomethingConverter::
EggToSomethingConverter() {
  _egg_data = nullptr;
  _error = false;
}

/**
 *
 */
EggToSomethingConverter::
EggToSomethingConverter(const EggToSomethingConverter &copy) {
  _egg_data = nullptr;
  _error = false;
}

/**
 *
 */
EggToSomethingConverter::
~EggToSomethingConverter() {
  clear_egg_data();
}

/**
 * Sets the egg data that will be filled in when convert_file() is called.
 * This must be called before convert_file().
 */
void EggToSomethingConverter::
set_egg_data(EggData *egg_data) {
  _egg_data = egg_data;
}

/**
 * Returns a space-separated list of extension, in addition to the one
 * returned by get_extension(), that are recognized by this converter.
 */
std::string EggToSomethingConverter::
get_additional_extensions() const {
  return std::string();
}

/**
 * Returns true if this file type can transparently save compressed files
 * (with a .pz extension), false otherwise.
 */
bool EggToSomethingConverter::
supports_compressed() const {
  return false;
}
