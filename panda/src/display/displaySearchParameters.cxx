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
  _minimum_width = 1;
  _minimum_height = 1;
  _maximum_width = 6400;
  _maximum_height = 4800;
  _minimum_bits_per_pixel = 16;
  _maximum_bits_per_pixel = 32;
}

void displaySearchParameters::
 validate_window_size(int &width, int &height) {
  if (width < _minimum_width) width = _minimum_width;
  if (height < _minimum_height) height = _minimum_height;
}

/**
 *
 */
void DisplaySearchParameters::
set_minimum_width (int minimum_width) {
  _minimum_width = (minimum_width > 0) ? minimum_width : 1;
}
void DisplaySearchParameters::
set_window_size(int requested_width, int requested_height) {
  validate_window_size(requested_width, requested_height);
    _current_width = requested_width;
  _current_height = requested_height;
  
}
/**
 *
 */
void DisplaySearchParameters::
set_maximum_width (int maximum_width) {
   _maximum_width = (maximum_width > 0) ? maximum_width : 1;
}

/**
 *
 */
void DisplaySearchParameters::
set_minimum_height (int minimum_height) {
 _minimum_height = (minimum_height > 0) ? minimum_height : 1;
}

/**
 *
 */
void DisplaySearchParameters::
set_maximum_height (int maximum_height) {
   _maximum_height = (maximum_height > 0) ? maximum_height : 1;
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
