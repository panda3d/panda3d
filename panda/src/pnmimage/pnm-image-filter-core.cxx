// Filename: pnm-image-filter-core.cxx
// Created by:  
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

// We map X and Y to A and B, because we might change our minds about which
// is dominant, and we map get/set functions for the channel in question to
// GETVAL/SETVAL.


static void
FUNCTION_NAME(PNMImage &dest, const PNMImage &source,
              double width, FilterFunction *make_filter) {
  if (!dest.is_valid() || !source.is_valid()) {
    return;
  }

  // First, set up a 2-d column-major matrix of StoreTypes, big enough to hold
  // the image xelvals scaled in the A direction only.  This will hold the
  // adjusted xel data from our first pass.

  typedef StoreType *StoreTypeP;
  StoreType **matrix = (StoreType **)PANDA_MALLOC_ARRAY(dest.ASIZE() * sizeof(StoreType *));

  int a, b;

  for (a=0; a<dest.ASIZE(); a++) {
    matrix[a] = (StoreType *)PANDA_MALLOC_ARRAY(source.BSIZE() * sizeof(StoreType));
  }

  // Now, scale the image in the A direction.
  double scale = (double)dest.ASIZE() / (double)source.ASIZE();

  StoreType *temp_source = (StoreType *)PANDA_MALLOC_ARRAY(source.ASIZE() * sizeof(StoreType));
  StoreType *temp_dest = (StoreType *)PANDA_MALLOC_ARRAY(dest.ASIZE() * sizeof(StoreType));

  WorkType *filter;
  double filter_width;

  make_filter(scale, width, filter, filter_width);

  for (b=0; b<source.BSIZE(); b++) {
    for (a=0; a<source.ASIZE(); a++) {
      temp_source[a] = (StoreType)(source_max * source.GETVAL(a, b));
    }

    filter_row(temp_dest, dest.ASIZE(),
               temp_source, source.ASIZE(),
               scale,
               filter, filter_width);

    for (a=0; a<dest.ASIZE(); a++) {
      matrix[a][b] = temp_dest[a];
    }
  }

  PANDA_FREE_ARRAY(temp_source);
  PANDA_FREE_ARRAY(temp_dest);
  PANDA_FREE_ARRAY(filter);

  // Now, scale the image in the B direction.
  scale = (double)dest.BSIZE() / (double)source.BSIZE();

  temp_dest = (StoreType *)PANDA_MALLOC_ARRAY(dest.BSIZE() * sizeof(StoreType));

  make_filter(scale, width, filter, filter_width);

  for (a=0; a<dest.ASIZE(); a++) {

    filter_row(temp_dest, dest.BSIZE(),
               matrix[a], source.BSIZE(),
               scale,
               filter, filter_width);

    for (b=0; b<dest.BSIZE(); b++) {
      dest.SETVAL(a, b, (double)temp_dest[b]/(double)source_max);
    }
  }

  PANDA_FREE_ARRAY(temp_dest);
  PANDA_FREE_ARRAY(filter);

  // Now, clean up our temp matrix and go home!

  for (a=0; a<dest.ASIZE(); a++) {
    PANDA_FREE_ARRAY(matrix[a]);
  }
  PANDA_FREE_ARRAY(matrix);
}

