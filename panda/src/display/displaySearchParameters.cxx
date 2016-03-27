/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file displaySearchParameters.cxx
 * @author aignacio
 * @date 2007-01-17
 */

#include "displaySearchParameters.h"

/**
 *
 */
DisplaySearchParameters::
~DisplaySearchParameters() {

}

/**
 *
 */
DisplaySearchParameters::
DisplaySearchParameters() {
  _minimum_width = 640;
  _minimum_height = 480;
  _maximum_width = 6400;
  _maximum_height = 4800;
  _minimum_bits_per_pixel = 16;
  _maximum_bits_per_pixel = 32;
}

/**
 *
 */
void DisplaySearchParameters::
set_minimum_width (int minimum_width) {
  _minimum_width = minimum_width;
}

/**
 *
 */
void DisplaySearchParameters::
set_maximum_width (int maximum_width) {
  _maximum_width = maximum_width;
}

/**
 *
 */
void DisplaySearchParameters::
set_minimum_height (int minimum_height) {
  _minimum_height = minimum_height;
}

/**
 *
 */
void DisplaySearchParameters::
set_maximum_height (int maximum_height) {
  _maximum_height = maximum_height;
}

/**
 *
 */
void DisplaySearchParameters::
set_minimum_bits_per_pixel (int minimum_bits_per_pixel) {
  _minimum_bits_per_pixel = minimum_bits_per_pixel;
}

/**
 *
 */
void DisplaySearchParameters::
set_maximum_bits_per_pixel (int maximum_bits_per_pixel) {
   _maximum_bits_per_pixel = maximum_bits_per_pixel;
}
