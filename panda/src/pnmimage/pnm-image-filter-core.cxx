// This file is actually a poor man's template function.  This is #included
// several times into pnm-image-filter.cc, with certain macros redefined each
// time to declare several copies of this function that operate on the
// different channels of the image.

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
  StoreType **matrix = new StoreTypeP[dest.ASIZE()];

  int a, b;

  for (a=0; a<dest.ASIZE(); a++) {
    matrix[a] = new StoreType[source.BSIZE()];
  }

  // Now, scale the image in the A direction.
  double scale = (double)dest.ASIZE() / (double)source.ASIZE();

  StoreType *temp_source = new StoreType[source.ASIZE()];
  StoreType *temp_dest = new StoreType[dest.ASIZE()];

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

  delete[] temp_source;
  delete[] temp_dest;
  delete[] filter;

  // Now, scale the image in the B direction.
  scale = (double)dest.BSIZE() / (double)source.BSIZE();

  temp_dest = new StoreType[dest.BSIZE()];

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

  delete[] temp_dest;
  delete[] filter;

  // Now, clean up our temp matrix and go home!

  for (a=0; a<dest.ASIZE(); a++) {
    delete[] matrix[a];
  }
  delete[] matrix;
}

