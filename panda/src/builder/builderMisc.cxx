// Filename: builderMisc.cxx
// Created by:  drose (18Sep97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "builderMisc.h"
#include "builderTypes.h"

#include "luse.h"
#include <stdlib.h>
////////////////////////////////////////////////////////////////////
//     Function: make_random_color
//  Description: Chooses a reasonable random color.
////////////////////////////////////////////////////////////////////
void
make_random_color(Colorf &color) {
  LVector3f rgb;
  float len;
  do {
    for (int i = 0; i < 3; i++) {
      rgb[i] = (double)rand() / (double)RAND_MAX;
    }
    len = length(rgb);

    // Repeat until we have a color that's not too dark or too light.
  } while (len < .1 || len > 1.5);

#ifdef WIN32_VC
  color.set(rgb[0], rgb[1], rgb[2],
            0.25 + 0.75 * (double)rand() / (double)RAND_MAX);
#else
  color.set(rgb[0], rgb[1], rgb[2],
            0.25 + 0.75 * (double)random() / (double)RAND_MAX);
#endif
}

