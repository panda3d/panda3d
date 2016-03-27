/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageFixHiddenColor.h
 * @author drose
 * @date 2003-03-13
 */

#ifndef IMAGEFIXHIDDENCOLOR_H
#define IMAGEFIXHIDDENCOLOR_H

#include "pandatoolbase.h"

#include "imageFilter.h"

/**
 * This program repairs an image's RGB values hidden behind an A value of 0.
 */
class ImageFixHiddenColor : public ImageFilter {
public:
  ImageFixHiddenColor();

  void run();

private:
  Filename _alpha_filename;
  double _min_opaque_alpha;
  double _max_transparent_alpha;
};

#include "imageFixHiddenColor.I"

#endif
