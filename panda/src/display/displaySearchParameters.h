// Filename: displaySearchParameters.h
// Created by:  aignacio (17Jan07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights 
// reserved.
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef DISPLAYSEARCHPARAMETERS_H
#define DISPLAYSEARCHPARAMETERS_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : DisplaySearchParameters
// Description : Parameters used for searching display capabilities.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DisplaySearchParameters {

PUBLISHED:
  DisplaySearchParameters::DisplaySearchParameters();
  DisplaySearchParameters::~DisplaySearchParameters();

  void DisplaySearchParameters::set_minimum_width(int minimum_width);
  void DisplaySearchParameters::set_maximum_width(int maximum_width);
  void DisplaySearchParameters::set_minimum_height(int minimum_height);
  void DisplaySearchParameters::set_maximum_height(int maximum_height);
  void DisplaySearchParameters::set_minimum_bits_per_pixel(int minimum_bits_per_pixel);
  void DisplaySearchParameters::set_maximum_bits_per_pixel(int maximum_bits_per_pixel);

public:
  int _minimum_width;
  int _maximum_width;
  int _minimum_height;
  int _maximum_height;
  int _minimum_bits_per_pixel;
  int _maximum_bits_per_pixel;
};

#endif