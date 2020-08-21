/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnm-image-filter-core.cxx
 */

// We map X and Y to A and B, because we might change our minds about which is
// dominant, and we map getset functions for the channel in question to
// GETVALSETVAL.


static void
FUNCTION_NAME(IMAGETYPE &dest, const IMAGETYPE &source,
              float width, FilterFunction *make_filter, int channel) {
  if (!dest.is_valid() || !source.is_valid()) {
    return;
  }

  // First, set up a 2-d column-major matrix of StoreTypes, big enough to hold
  // the image xelvals scaled in the A direction only.  This will hold the
  // adjusted xel data from our first pass.

  StoreType **matrix = (StoreType **)PANDA_MALLOC_ARRAY(dest.ASIZE() * sizeof(StoreType *));

  int a, b;

  for (a=0; a<dest.ASIZE(); a++) {
    matrix[a] = (StoreType *)PANDA_MALLOC_ARRAY(source.BSIZE() * sizeof(StoreType));
  }

  // First, scale the image in the A direction.
  float scale;
  StoreType *temp_source, *temp_dest;

  scale = (float)dest.ASIZE() / (float)source.ASIZE();
  temp_source = (StoreType *)PANDA_MALLOC_ARRAY(source.ASIZE() * sizeof(StoreType));
  temp_dest = (StoreType *)PANDA_MALLOC_ARRAY(dest.ASIZE() * sizeof(StoreType));

  WorkType *filter;
  float filter_width;
  int actual_width;

  make_filter(scale, width, filter, filter_width, actual_width);

  for (b = 0; b < source.BSIZE(); b++) {
    for (a = 0; a < source.ASIZE(); a++) {
      temp_source[a] = (StoreType)(source_max * source.GETVAL(a, b, channel));
    }

    filter_row(temp_dest, dest.ASIZE(),
               temp_source, source.ASIZE(),
               scale,
               filter, filter_width, actual_width);

    for (a = 0; a < dest.ASIZE(); a++) {
      matrix[a][b] = temp_dest[a];
    }
  }

  PANDA_FREE_ARRAY(temp_source);
  PANDA_FREE_ARRAY(temp_dest);
  PANDA_FREE_ARRAY(filter);

  // Now, scale the image in the B direction.
  scale = (float)dest.BSIZE() / (float)source.BSIZE();
  temp_dest = (StoreType *)PANDA_MALLOC_ARRAY(dest.BSIZE() * sizeof(StoreType));

  make_filter(scale, width, filter, filter_width, actual_width);

  for (a = 0; a < dest.ASIZE(); a++) {
    filter_row(temp_dest, dest.BSIZE(),
               matrix[a], source.BSIZE(),
               scale,
               filter, filter_width, actual_width);

    for (b = 0; b < dest.BSIZE(); b++) {
      dest.SETVAL(a, b, channel, (float)temp_dest[b]/(float)source_max);
    }
  }

  PANDA_FREE_ARRAY(temp_dest);
  PANDA_FREE_ARRAY(filter);

  // Now, clean up our temp matrix and go home!

  for (a = 0; a < dest.ASIZE(); a++) {
    PANDA_FREE_ARRAY(matrix[a]);
  }
  PANDA_FREE_ARRAY(matrix);
}
