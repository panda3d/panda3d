/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnm-image-filter.cxx
 */

// The functions in this module support spatial filtering of an image by
// convolution with an (almost) arbitrary kernel.  There are a broad class of
// image filters to which this principle applies, including box filtering,
// Bartlett, and Gaussian.

// This particular approach breaks the 2-d kernel into two 1-d kernels, which
// improves performance at the expense of limiting the types of filters that
// may be applied.  In general, any of the square-based kernels are still
// applicable; the only circle-based kernel that still works with this
// approach is Gaussian.  Furthermore, the implementation assume that the
// kernel is symmetric about zero.

// The image is filtered first along one axis, then along the other.  This
// decreases the complexity of the convolution operation: it is faster to
// convolve twice with a one-dimensional kernel than once with a two-
// dimensional kernel.  In the interim, a temporary matrix of type StoreType
// (a numeric type, described below) is built which contains the results from
// the first convolution.  The entire process is then repeated for each
// channel in the image.

#include "pandabase.h"
#include <math.h>
#include "cmath.h"
#include "thread.h"

#include "pnmImage.h"
#include "pfmFile.h"

using std::max;
using std::min;

// WorkType is an abstraction that allows the filtering process to be
// recompiled to use either floating-point or integer arithmetic.  On SGI
// machines, there doesn't seem to be much of a performance difference-- if
// anything, the integer arithmetic is slower--though certainly other
// architectures may differ.

// A StoreType is a numeric type that is used to store the intermediate values
// of the filtering computation; temporary calculations are performed using
// WorkTypes.  source_max represents the largest value that will be stored in
// a StoreType, while filter_max represents the largest value that will
// multiplied by it as a weighted filter (source_max * filter_max must fit
// comfortably within a WorkType, with room to spare).

// Floating-point arithmetic is slightly faster and slightly more precise, but
// the main reason to use it is that it is conceptually easy to understand.
// All values are scaled in the range 0..1, as they should be.

// The biggest reason to use integer arithmetic is space.  A table of
// StoreTypes must be allocated to match the size of the image.  On an SGI,
// sizeof(float) is 4, while sizeof(short) is 2 and sizeof(char) is, of
// course, 1.  Since the source precision is probably 8 bits anyway (there are
// usually 8 bits per channel), it doesn't cost any precision at all to use
// shorts, and not very much to use chars.

// To use double-precision floating point, 8 bytes: (strictly for the
// neurotic)
/*
typedef double WorkType;
typedef double StoreType;
static const WorkType source_max = 1.0;
static const WorkType filter_max = 1.0;
*/

// To use single-precision floating point, 4 bytes:
typedef float WorkType;
typedef float StoreType;
static const WorkType source_max = 1.0f;
static const WorkType filter_max = 1.0f;

/*
// To use 16-bit integer arithmetic, 2 bytes:
typedef unsigned long WorkType;
typedef unsigned short StoreType;
static const WorkType source_max = 65535;
static const WorkType filter_max = 255;
*/

/*
// To use 8-bit integer arithmetic, 1 byte:
typedef unsigned long WorkType;
typedef unsigned char StoreType;
static const WorkType source_max = 255;
static const WorkType filter_max = 255;
*/

// filter_row() filters a single row by convolving with a one-dimensional
// kernel filter.  The kernel is defined by an array of weights in filter[],
// where the ith element of filter corresponds to abs(d * scale), if
// scale>1.0, and abs(d), if scale<=1.0, where d is the offset from the center
// and varies from -filter_width to filter_width.

// Note that filter_width is not necessarily the length of the array; it is
// the radius of interest of the filter function.  The array may need to be
// larger (by a factor of scale), to adequately cover all the values.

static void
filter_row(StoreType dest[], int dest_len,
           const StoreType source[], int source_len,
           float scale,                    //  == dest_len / source_len
           const WorkType filter[],
           float filter_width,
           int actual_width) {
  // If we are expanding the row (scale > 1.0), we need to look at a
  // fractional granularity.  Hence, we scale our filter index by scale.  If
  // we are compressing (scale < 1.0), we don't need to fiddle with the filter
  // index, so we leave it at one.

  float iscale;
  if (scale < 1.0f) {
    iscale = 1.0f;
    filter_width /= scale;
  } else {
    iscale = scale;
  }

  for (int dest_x = 0; dest_x < dest_len; dest_x++) {
    // The additional offset of 0.5 keeps the pixel centered.
    float center = (dest_x + 0.5f) / scale - 0.5f;

    // left and right are the starting and ending ranges of the radius of
    // interest of the filter function.  We need to apply the filter to each
    // value in this range.
    int left = max((int)cfloor(center - filter_width), 0);
    int right = min((int)cceil(center + filter_width), source_len - 1);

    // right_center is the point just to the right of the center.  This allows
    // us to flip the sign of the offset when we cross the center point.
    int right_center = (int)cceil(center);

    WorkType net_weight = 0;
    WorkType net_value = 0;

    int index, source_x;

    // This loop is broken into two pieces--the left of center and the right
    // of center--so we don't have to incur the overhead of calling fabs()
    // each time through the loop.
    for (source_x = left; source_x < right_center; source_x++) {
      index = (int)cfloor(iscale * (center - source_x) + 0.5f);
      nassertv(index >= 0 && index < actual_width);
      net_value += filter[index] * source[source_x];
      net_weight += filter[index];
    }

    for (; source_x <= right; source_x++) {
      index = (int)cfloor(iscale * (source_x - center) + 0.5f);
      nassertv(index >= 0 && index < actual_width);
      net_value += filter[index] * source[source_x];
      net_weight += filter[index];
    }

    if (net_weight > 0) {
      dest[dest_x] = (StoreType)(net_value / net_weight);
    } else {
      dest[dest_x] = 0;
    }
  }
  Thread::consider_yield();
}

// As above, but we also accept an array of weight values per element, to
// support scaling a sparse array (as in a PfmFile).
static void
filter_sparse_row(StoreType dest[], StoreType dest_weight[], int dest_len,
                  const StoreType source[], const StoreType source_weight[], int source_len,
                  float scale,                    //  == dest_len / source_len
                  const WorkType filter[],
                  float filter_width,
                  int actual_width) {
  // If we are expanding the row (scale > 1.0), we need to look at a
  // fractional granularity.  Hence, we scale our filter index by scale.  If
  // we are compressing (scale < 1.0), we don't need to fiddle with the filter
  // index, so we leave it at one.

  float iscale;
  if (scale < 1.0f) {
    iscale = 1.0f;
    filter_width /= scale;
  } else {
    iscale = scale;
  }

  for (int dest_x = 0; dest_x < dest_len; dest_x++) {
    // The additional offset of 0.5 keeps the pixel centered.
    float center = (dest_x + 0.5f) / scale - 0.5f;

    // left and right are the starting and ending ranges of the radius of
    // interest of the filter function.  We need to apply the filter to each
    // value in this range.
    int left = max((int)cfloor(center - filter_width), 0);
    int right = min((int)cceil(center + filter_width), source_len - 1);

    // right_center is the point just to the right of the center.  This allows
    // us to flip the sign of the offset when we cross the center point.
    int right_center = (int)cceil(center);

    WorkType net_weight = 0;
    WorkType net_value = 0;

    int index, source_x;

    // This loop is broken into two pieces--the left of center and the right
    // of center--so we don't have to incur the overhead of calling fabs()
    // each time through the loop.
    for (source_x = left; source_x < right_center; source_x++) {
      index = (int)cfloor(iscale * (center - source_x) + 0.5f);
      nassertv(index >= 0 && index < actual_width);
      net_value += filter[index] * source[source_x] * source_weight[source_x];
      net_weight += filter[index] * source_weight[source_x];
    }

    for (; source_x <= right; source_x++) {
      index = (int)cfloor(iscale * (source_x - center) + 0.5f);
      nassertv(index >= 0 && index < actual_width);
      net_value += filter[index] * source[source_x] * source_weight[source_x];
      net_weight += filter[index] * source_weight[source_x];
    }

    if (net_weight > 0) {
      dest[dest_x] = (StoreType)(net_value / net_weight);
    } else {
      dest[dest_x] = 0;
    }
    dest_weight[dest_x] = (StoreType)net_weight;
  }
  Thread::consider_yield();
}


// The various filter functions are called before each axis scaling to build
// an kernel array suitable for the given scaling factor.  Given a scaling
// ratio of the axis (dest_len  source_len), and a width parameter supplied by
// the user, they must build an array of filter values (described above) and
// also set the radius of interest of the filter function.

// The values of the elements of filter must completely cover the range
// 0..filter_max; the array must have enough elements to include all indices
// corresponding to values in the range -filter_width to filter_width.

typedef void FilterFunction(float scale, float width,
                            WorkType *&filter, float &filter_width, int &actual_width);

static void
box_filter_impl(float scale, float width,
                WorkType *&filter, float &filter_width,
                int &actual_width) {
  float fscale;
  if (scale < 1.0) {
    // If we are compressing the image, we want to expand the range of the
    // filter function to prevent dropping below the Nyquist rate.  Hence, we
    // divide by scale.
    fscale = 1.0 / scale;

  } else {
    // If we are expanding the image, we want to increase the granularity of
    // the filter function since we will need to access fractional cel values.
    // Hence, we multiply by scale.
    fscale = scale;
  }
  filter_width = width;

  // It seems we need a buffer of two extra values in the filter array
  // to allow room for all calculations (especially including the 1/2
  // pixel offset).
  actual_width = (int)cceil((filter_width + 1) * fscale) + 2;

  filter = (WorkType *)PANDA_MALLOC_ARRAY(actual_width * sizeof(WorkType));

  for (int i = 0; i < actual_width; i++) {
    filter[i] = (i <= filter_width * fscale) ? filter_max : 0;
  }
}

static void
gaussian_filter_impl(float scale, float width,
                     WorkType *&filter, float &filter_width,
                     int &actual_width) {
  float fscale;
  if (scale < 1.0) {
    // If we are compressing the image, we want to expand the range of the
    // filter function to prevent dropping below the Nyquist rate.  Hence, we
    // divide by scale (to make fscale larger).
    fscale = 1.0 / scale;
  } else {

    // If we are expanding the image, we want to increase the granularity of
    // the filter function since we will need to access fractional cel values.
    // Hence, we multiply by scale (to make fscale larger).
    fscale = scale;
  }

  float sigma = width/2;
  filter_width = 3.0 * sigma;

  // It seems we need a buffer of two extra values in the filter array
  // to allow room for all calculations (especially including the 1/2
  // pixel offset).
  actual_width = (int)cceil((filter_width + 1) * fscale) + 2;

  // G(x, y) = (1(2 pi sigma^2)) * exp( - (x^2 + y^2)  (2 sigma^2))

  // (We can throw away the initial factor, since these weights will all be
  // normalized; and we're only computing a 1-dimensional function, so we can
  // ignore the y^2.)

  filter = (WorkType *)PANDA_MALLOC_ARRAY(actual_width * sizeof(WorkType));
  float div = 2 * sigma * sigma;

  for (int i = 0; i < actual_width; i++) {
    float x = i / fscale;
    filter[i] = (WorkType)(filter_max * exp(-x*x / div));
    // The highest value of the exp function in this range is always 1.0, at
    // index value 0.  Thus, we scale the whole range by filter_max, to
    // produce a filter in the range [0..filter_max].
  }
}


// We have a function, defined in pnm-image-filter-core.cxx, that will scale
// an image in both X and Y directions for a particular channel, by setting up
// the temporary matrix appropriately and calling the above functions.

// What we really need is a series of such functions, one for each channel,
// and also one to scale by X first, and one to scale by Y first.  This sounds
// a lot like a C++ template: we want to compile the same function several
// times to work on slightly different sorts of things each time.  However,
// the things we want to vary are the particular member functions of PNMImage
// that we call (e.g.  Red(), Green(), etc.), and we can't declare a template
// of member functions, only of types.

// It's doable using templates.  It would involve the declaration of lots of
// silly little functor objects.  This is much more compact and no more
// difficult to read.

// The function in pnm-image-filter-core.cxx uses macros to access the member
// functions of PNMImage.  Hence, we only need to redefine those macros with
// each instance of the function to cause each instance to operate on the
// correct member.


// These instances scale by X first, then by Y.

#define FUNCTION_NAME filter_red_xy
#define IMAGETYPE PNMImage
#define ASIZE get_x_size
#define BSIZE get_y_size
#define GETVAL(a, b, channel) get_red(a, b)
#define SETVAL(a, b, channel, v) set_red(a, b, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_green_xy
#define IMAGETYPE PNMImage
#define ASIZE get_x_size
#define BSIZE get_y_size
#define GETVAL(a, b, channel) get_green(a, b)
#define SETVAL(a, b, channel, v) set_green(a, b, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_blue_xy
#define IMAGETYPE PNMImage
#define ASIZE get_x_size
#define BSIZE get_y_size
#define GETVAL(a, b, channel) get_blue(a, b)
#define SETVAL(a, b, channel, v) set_blue(a, b, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_gray_xy
#define IMAGETYPE PNMImage
#define ASIZE get_x_size
#define BSIZE get_y_size
#define GETVAL(a, b, channel) get_bright(a, b)
#define SETVAL(a, b, channel, v) set_xel(a, b, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_alpha_xy
#define IMAGETYPE PNMImage
#define ASIZE get_x_size
#define BSIZE get_y_size
#define GETVAL(a, b, channel) get_alpha(a, b)
#define SETVAL(a, b, channel, v) set_alpha(a, b, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME


// These instances scale by Y first, then by X.

#define FUNCTION_NAME filter_red_yx
#define IMAGETYPE PNMImage
#define ASIZE get_y_size
#define BSIZE get_x_size
#define GETVAL(a, b, channel) get_red(b, a)
#define SETVAL(a, b, channel, v) set_red(b, a, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_green_yx
#define IMAGETYPE PNMImage
#define ASIZE get_y_size
#define BSIZE get_x_size
#define GETVAL(a, b, channel) get_green(b, a)
#define SETVAL(a, b, channel, v) set_green(b, a, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_blue_yx
#define IMAGETYPE PNMImage
#define ASIZE get_y_size
#define BSIZE get_x_size
#define GETVAL(a, b, channel) get_blue(b, a)
#define SETVAL(a, b, channel, v) set_blue(b, a, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_gray_yx
#define IMAGETYPE PNMImage
#define ASIZE get_y_size
#define BSIZE get_x_size
#define GETVAL(a, b, channel) get_bright(b, a)
#define SETVAL(a, b, channel, v) set_xel(b, a, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_alpha_yx
#define IMAGETYPE PNMImage
#define ASIZE get_y_size
#define BSIZE get_x_size
#define GETVAL(a, b, channel) get_alpha(b, a)
#define SETVAL(a, b, channel, v) set_alpha(b, a, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME


// filter_image pulls everything together, and filters one image into another.
// Both images can be the same with no ill effects.
static void
filter_image(PNMImage &dest, const PNMImage &source,
             float width, FilterFunction *make_filter) {

  // We want to scale by the smallest destination axis first, for a slight
  // performance gain.

  // In the PNMImage case (unlike the PfmFile case), the channel parameter is
  // not used.  We *could* use it to avoid the replication of quite so many
  // functions, but we replicate them anyway, for another tiny performance
  // gain.

  if (dest.get_x_size() <= dest.get_y_size()) {
    if (dest.is_grayscale() || source.is_grayscale()) {
      filter_gray_xy(dest, source, width, make_filter, 0);
    } else {
      filter_red_xy(dest, source, width, make_filter, 0);
      filter_green_xy(dest, source, width, make_filter, 0);
      filter_blue_xy(dest, source, width, make_filter, 0);
    }

    if (dest.has_alpha() && source.has_alpha()) {
      filter_alpha_xy(dest, source, width, make_filter, 0);
    }

  } else {
    if (dest.is_grayscale() || source.is_grayscale()) {
      filter_gray_yx(dest, source, width, make_filter, 0);
    } else {
      filter_red_yx(dest, source, width, make_filter, 0);
      filter_green_yx(dest, source, width, make_filter, 0);
      filter_blue_yx(dest, source, width, make_filter, 0);
    }

    if (dest.has_alpha() && source.has_alpha()) {
      filter_alpha_yx(dest, source, width, make_filter, 0);
    }
  }
}

/**
 * Makes a resized copy of the indicated image into this one using the
 * indicated filter.  The image to be copied is squashed and stretched to
 * match the dimensions of the current image, applying the appropriate filter
 * to perform the stretching.
 */
void PNMImage::
box_filter_from(float width, const PNMImage &copy) {
  filter_image(*this, copy, width, &box_filter_impl);
}

/**
 * Makes a resized copy of the indicated image into this one using the
 * indicated filter.  The image to be copied is squashed and stretched to
 * match the dimensions of the current image, applying the appropriate filter
 * to perform the stretching.
 */
void PNMImage::
gaussian_filter_from(float width, const PNMImage &copy) {
  filter_image(*this, copy, width, &gaussian_filter_impl);
}

// Now we do it again, this time for PfmFile.  In this case we also need to
// support the sparse variants, since PfmFiles can be incomplete.  However, we
// don't need to have a different function for each channel.

#define FUNCTION_NAME filter_pfm_xy
#define IMAGETYPE PfmFile
#define ASIZE get_x_size
#define BSIZE get_y_size
#define GETVAL(a, b, channel) get_channel(a, b, channel)
#define SETVAL(a, b, channel, v) set_channel(a, b, channel, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_pfm_yx
#define IMAGETYPE PfmFile
#define ASIZE get_y_size
#define BSIZE get_x_size
#define GETVAL(a, b, channel) get_channel(b, a, channel)
#define SETVAL(a, b, channel, v) set_channel(b, a, channel, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME


#define FUNCTION_NAME filter_pfm_sparse_xy
#define IMAGETYPE PfmFile
#define ASIZE get_x_size
#define BSIZE get_y_size
#define HASVAL(a, b) has_point(a, b)
#define GETVAL(a, b, channel) get_channel(a, b, channel)
#define SETVAL(a, b, channel, v) set_channel(a, b, channel, v)
#include "pnm-image-filter-sparse-core.cxx"
#undef SETVAL
#undef GETVAL
#undef HASVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_pfm_sparse_yx
#define IMAGETYPE PfmFile
#define ASIZE get_y_size
#define BSIZE get_x_size
#define HASVAL(a, b) has_point(b, a)
#define GETVAL(a, b, channel) get_channel(b, a, channel)
#define SETVAL(a, b, channel, v) set_channel(b, a, channel, v)
#include "pnm-image-filter-sparse-core.cxx"
#undef SETVAL
#undef GETVAL
#undef HASVAL
#undef BSIZE
#undef ASIZE
#undef IMAGETYPE
#undef FUNCTION_NAME


// filter_image pulls everything together, and filters one image into another.
// Both images can be the same with no ill effects.
static void
filter_image(PfmFile &dest, const PfmFile &source,
             float width, FilterFunction *make_filter) {
  int num_channels = min(dest.get_num_channels(), source.get_num_channels());

  if (source.has_no_data_value()) {
    // We need to use the sparse variant.
    if (dest.get_x_size() <= dest.get_y_size()) {
      for (int ci = 0; ci < num_channels; ++ci) {
        filter_pfm_sparse_xy(dest, source, width, make_filter, ci);
      }

    } else {
      for (int ci = 0; ci < num_channels; ++ci) {
        filter_pfm_sparse_yx(dest, source, width, make_filter, ci);
      }
    }
  } else {
    // We can use the slightly faster fully-specified variant.
    if (dest.get_x_size() <= dest.get_y_size()) {
      for (int ci = 0; ci < num_channels; ++ci) {
        filter_pfm_xy(dest, source, width, make_filter, ci);
      }

    } else {
      for (int ci = 0; ci < num_channels; ++ci) {
        filter_pfm_yx(dest, source, width, make_filter, ci);
      }
    }
  }
}

/**
 * Makes a resized copy of the indicated image into this one using the
 * indicated filter.  The image to be copied is squashed and stretched to
 * match the dimensions of the current image, applying the appropriate filter
 * to perform the stretching.
 */
void PfmFile::
box_filter_from(float width, const PfmFile &copy) {
  filter_image(*this, copy, width, &box_filter_impl);
}

/**
 * Makes a resized copy of the indicated image into this one using the
 * indicated filter.  The image to be copied is squashed and stretched to
 * match the dimensions of the current image, applying the appropriate filter
 * to perform the stretching.
 */
void PfmFile::
gaussian_filter_from(float width, const PfmFile &copy) {
  filter_image(*this, copy, width, &gaussian_filter_impl);
}

// The following functions are support for quick_box_filter().

static INLINE void
box_filter_xel(const PNMImage &img,
               int x, int y, float x_contrib, float y_contrib,
               LColorf &color, float &pixel_count) {

  float contrib = x_contrib * y_contrib;
  color += img.get_xel_a(x, y) * contrib;
  pixel_count += contrib;
}

static INLINE void
box_filter_line(const PNMImage &image,
                float x0, int y, float x1, float y_contrib,
                LColorf &color, float &pixel_count) {
  int x = (int)x0;
  // Get the first (partial) xel
  box_filter_xel(image, x, y, (float)(x+1)-x0, y_contrib,
                 color, pixel_count);

  int x_last = (int)x1;
  if (x < x_last) {
    x++;
    while (x < x_last) {
      // Get each consecutive (complete) xel
      box_filter_xel(image, x, y, 1.0f, y_contrib,
                     color, pixel_count);
      x++;
    }

    // Get the final (partial) xel
    float x_contrib = x1 - (float)x_last;
    if (x_contrib > 0.0001f && x < image.get_x_size()) {
      box_filter_xel(image, x, y, x_contrib, y_contrib,
                     color, pixel_count);
    }
  }
}

static LColorf
box_filter_region(const PNMImage &image,
                  float x0, float y0, float x1, float y1) {
  LColorf color = LColorf::zero();
  float pixel_count = 0.0f;

  assert(y0 >= 0 && y1 >= 0);

  int y = (int)y0;
  // Get the first (partial) row
  box_filter_line(image, x0, y, x1, (float)(y+1)-y0,
                  color, pixel_count);

  int y_last = (int)y1;
  if (y < y_last) {
    y++;
    while (y < y_last) {
      // Get each consecutive (complete) row
      box_filter_line(image, x0, y, x1, 1.0f,
                      color, pixel_count);
      y++;
    }

    // Get the final (partial) row
    float y_contrib = y1 - (float)y_last;
    if (y_contrib > 0.0001f && y < image.get_y_size()) {
      box_filter_line(image, x0, y, x1, y_contrib,
                      color, pixel_count);
    }
  }

  // cerr << pixel_count << "\n";
  color /= pixel_count;
  return color;
}

/**
 * Resizes from the given image, with a fixed radius of 0.5. This is a very
 * specialized and simple algorithm that doesn't handle dropping below the
 * Nyquist rate very well, but is quite a bit faster than the more general
 * box_filter(), above.  If borders are specified, they will further restrict
 * the size of the resulting image.  There's no point in using
 * quick_box_filter() on a single image.
 */
void PNMImage::
quick_filter_from(const PNMImage &from, int xborder, int yborder) {
  int from_xs = from.get_x_size();
  int from_ys = from.get_y_size();

  int to_xs = get_x_size() - xborder;
  int to_ys = get_y_size() - yborder;

  int to_xoff = xborder / 2;
  int to_yoff = yborder / 2;

  float from_x0, from_x1, from_y0, from_y1;
  int to_x, to_y;

  float x_scale = (float)from_xs / (float)to_xs;
  float y_scale = (float)from_ys / (float)to_ys;

  LColorf color;

  from_y0 = max(0, -to_yoff) * y_scale;
  for (to_y = max(0, -to_yoff);
       to_y < min(to_ys, get_y_size()-to_yoff);
       to_y++) {
    from_y1 = (to_y+1) * y_scale;

    from_x0 = max(0, -to_xoff) * x_scale;
    for (to_x = max(0, -to_xoff);
         to_x < min(to_xs, get_x_size()-to_xoff);
         to_x++) {
      from_x1 = (to_x+1) * x_scale;

      // Now the box from (from_x0, from_y0) - (from_x1, from_y1) but not
      // including (from_x1, from_y1) maps to the pixel (to_x, to_y).
      color = box_filter_region(from,
                                from_x0, from_y0, from_x1, from_y1);

      set_xel_a(to_xoff + to_x, to_yoff + to_y, color);

      from_x0 = from_x1;
    }
    from_y0 = from_y1;
    Thread::consider_yield();
  }
}
