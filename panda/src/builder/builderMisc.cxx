// Filename: builderMisc.cxx
// Created by:  drose (18Sep97)
// 
////////////////////////////////////////////////////////////////////

#include "builderMisc.h"
#include "builderTypes.h"

#include <luse.h>
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

