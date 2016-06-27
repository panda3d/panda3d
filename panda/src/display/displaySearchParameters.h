/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file displaySearchParameters.h
 * @author aignacio
 * @date 2007-01-17
 */

#ifndef DISPLAYSEARCHPARAMETERS_H
#define DISPLAYSEARCHPARAMETERS_H

#include "pandabase.h"

/**
 * Parameters used for searching display capabilities.
 */
class EXPCL_PANDA_DISPLAY DisplaySearchParameters {

PUBLISHED:
  DisplaySearchParameters();
  ~DisplaySearchParameters();

  void set_minimum_width(int minimum_width);
  void set_maximum_width(int maximum_width);
  void set_minimum_height(int minimum_height);
  void set_maximum_height(int maximum_height);
  void set_minimum_bits_per_pixel(int minimum_bits_per_pixel);
  void set_maximum_bits_per_pixel(int maximum_bits_per_pixel);

public:
  int _minimum_width;
  int _maximum_width;
  int _minimum_height;
  int _maximum_height;
  int _minimum_bits_per_pixel;
  int _maximum_bits_per_pixel;
};

#endif
