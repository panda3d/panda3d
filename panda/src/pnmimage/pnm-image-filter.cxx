// Filename: pnm-image-filter.cxx
// Created by:  
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
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
// (a numeric type, described below) is built which contains the results
// from the first convolution.  The entire process is then repeated for
// each channel in the image.

#include "pandabase.h"
#include <math.h>
#include "cmath.h"

#include "pnmImage.h"

// WorkType is an abstraction that allows the filtering process to be
// recompiled to use either floating-point or integer arithmetic.  On SGI
// machines, there doesn't seem to be much of a performance difference--
// if anything, the integer arithmetic is slower--though certainly other
// architectures may differ.

// A StoreType is a numeric type that is used to store the intermediate values
// of the filtering computation; temporary calculations are performed using
// WorkTypes.  source_max represents the largest value that will be stored in
// a StoreType, while filter_max represents the largest value that will
// multiplied by it as a weighted filter (source_max * filter_max must fit
// comfortably within a WorkType, with room to spare).

// Floating-point arithmetic is slightly faster and slightly more precise,
// but the main reason to use it is that it is conceptually easy to understand.
// All values are scaled in the range 0..1, as they should be.

// The biggest reason to use integer arithmetic is space.  A table of
// StoreTypes must be allocated to match the size of the image.  On an SGI,
// sizeof(float) is 4, while sizeof(short) is 2 and sizeof(char) is, of
// course, 1.  Since the source precision is probably 8 bits anyway (there
// are usually 8 bits per channel), it doesn't cost any precision at all to
// use shorts, and not very much to use chars.

/*
// To use double-precision floating point, 8 bytes: (strictly for the neurotic)
typedef double WorkType;
typedef double StoreType;
static const WorkType source_max = 1.0;
static const WorkType filter_max = 1.0;
*/

// To use single-precision floating point, 4 bytes:
typedef double WorkType;
typedef float StoreType;
static const WorkType source_max = 1.0;
static const WorkType filter_max = 1.0;

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
// where the ith element of filter corresponds to abs(d * scale), if scale>1.0,
// and abs(d), if scale<=1.0, where d is the offset from the center and varies
// from -filter_width to filter_width.

// Note that filter_width is not necessarily the length of the array; it is
// the radius of interest of the filter function.  The array may need to be
// larger (by a factor of scale), to adequately cover all the values.

static void
filter_row(StoreType dest[], int dest_len,
           const StoreType source[], int source_len,
           double scale,                    //  == dest_len / source_len
           const WorkType filter[],
           double filter_width) {
  // If we are expanding the row (scale>1.0), we need to look at a fractional
  // granularity.  Hence, we scale our filter index by scale.  If we are
  // compressing (scale<1.0), we don't need to fiddle with the filter index, so
  // we leave it at one.
  double iscale = max(scale, 1.0);

  // Similarly, if we are expanding the row, we want to start the new row at
  // the far left edge of the original pixel, not in the center.  So we will
  // have a non-zero offset.
  int offset = (int)cfloor(iscale*0.5);

  for (int dest_x=0; dest_x<dest_len; dest_x++) {
    double center = (dest_x-offset)/scale;

    // left and right are the starting and ending ranges of the radius of
    // interest of the filter function.  We need to apply the filter to each
    // value in this range.
    int left = max((int)cfloor(center - filter_width), 0);
    int right = min((int)cceil(center + filter_width), source_len-1);

    // right_center is the point just to the right of the center.  This
    // allows us to flip the sign of the offset when we cross the center point.
    int right_center = (int)cceil(center);

    WorkType net_weight = 0;
    WorkType net_value = 0;

    int index, source_x;

    // This loop is broken into two pieces--the left of center and the right
    // of center--so we don't have to incur the overhead of calling fabs()
    // each time through the loop.
    for (source_x=left; source_x<right_center; source_x++) {
      index = (int)(iscale*(center-source_x));
      net_value += filter[index] * source[source_x];
      net_weight += filter[index];
    }

    for (; source_x<=right; source_x++) {
      index = (int)(iscale*(source_x-center));
      net_value += filter[index] * source[source_x];
      net_weight += filter[index];
    }

    if (net_weight>0) {
      dest[dest_x] = (StoreType)(net_value / net_weight);
    } else {
      dest[dest_x] = 0;
    }
  }
}


// The various filter functions are called before each axis scaling to build
// an kernel array suitable for the given scaling factor.  Given a scaling
// ratio of the axis (dest_len / source_len), and a width parameter supplied
// by the user, they must build an array of filter values (described above)
// and also set the radius of interest of the filter function.

// The values of the elements of filter must completely cover the range
// 0..filter_max; the array must have enough elements to include all indices
// corresponding to values in the range -filter_width to filter_width.

typedef void FilterFunction(double scale, double width,
                            WorkType *&filter, double &filter_width);

static void
box_filter_impl(double scale, double width,
                WorkType *&filter, double &filter_width) {
  double fscale;
  if (scale < 1.0) {
    // If we are compressing the image, we want to expand the range of
    // the filter function to prevent dropping below the Nyquist rate.
    // Hence, we divide by scale.
    fscale = 1.0 / scale;
  } else {

    // If we are expanding the image, we want to increase the granularity
    // of the filter function since we will need to access fractional cel
    // values.  Hence, we multiply by scale.
    fscale = scale;
  }
  filter_width = width;
  int actual_width = (int)cceil((filter_width+1) * fscale);

  filter = new WorkType[actual_width];

  for (int i=0; i<actual_width; i++) {
    filter[i] = (i<=filter_width*fscale) ? filter_max : 0;
  }
}

static void
gaussian_filter_impl(double scale, double width,
                     WorkType *&filter, double &filter_width) {
  double fscale;
  if (scale < 1.0) {
    // If we are compressing the image, we want to expand the range of
    // the filter function to prevent dropping below the Nyquist rate.
    // Hence, we divide by scale (to make fscale larger).
    fscale = 1.0 / scale;
  } else {

    // If we are expanding the image, we want to increase the granularity
    // of the filter function since we will need to access fractional cel
    // values.  Hence, we multiply by scale (to make fscale larger).
    fscale = scale;
  }
  double sigma = width/2;
  filter_width = 3.0 * sigma;
  int actual_width = (int)cceil((filter_width+1) * fscale);

  // G(x, y) = (1/(2 pi sigma^2)) * exp( - (x^2 + y^2) / (2 sigma^2))

  // (We can throw away the initial factor, since these weights will all
  // be normalized; and we're only computing a 1-dimensional function,
  // so we can ignore the y^2.)

  filter = new WorkType[actual_width];
  double div = 2*sigma*sigma;

  for (int i=0; i<actual_width; i++) {
    double x = i/fscale;
    filter[i] = (WorkType)(filter_max * exp(-x*x / div));
    // The highest value of the exp function in this range is always 1.0,
    // at index value 0.  Thus, we scale the whole range by filter_max,
    // to produce a filter in the range [0..filter_max].
  }
}


// We have a function, defined in pnm-image-filter-core.cxx, that will scale
// an image in both X and Y directions for a particular channel, by setting
// up the temporary matrix appropriately and calling the above functions.

// What we really need is a series of such functions, one for each channel,
// and also one to scale by X first, and one to scale by Y first.  This sounds
// a lot like a C++ template: we want to compile the same function several
// times to work on slightly different sorts of things each time.  However,
// the things we want to vary are the particular member functions of PNMImage
// that we call (e.g. Red(), Green(), etc.), and we can't declare a template
// of member functions, only of types.

// It's doable using templates.  It would involve the declaration of
// lots of silly little functor objects.  This is much more compact
// and no more difficult to read.

// The function in pnm-image-filter-core.cxx uses macros to access the member
// functions of PNMImage.  Hence, we only need to redefine those macros
// with each instance of the function to cause each instance to operate on
// the correct member.


// These instances scale by X first, then by Y.

#define FUNCTION_NAME filter_red_xy
#define ASIZE get_x_size
#define BSIZE get_y_size
#define GETVAL(a, b) get_red(a, b)
#define SETVAL(a, b, v) set_red(a, b, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_green_xy
#define ASIZE get_x_size
#define BSIZE get_y_size
#define GETVAL(a, b) get_green(a, b)
#define SETVAL(a, b, v) set_green(a, b, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_blue_xy
#define ASIZE get_x_size
#define BSIZE get_y_size
#define GETVAL(a, b) get_blue(a, b)
#define SETVAL(a, b, v) set_blue(a, b, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_gray_xy
#define ASIZE get_x_size
#define BSIZE get_y_size
#define GETVAL(a, b) get_bright(a, b)
#define SETVAL(a, b, v) set_xel(a, b, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_alpha_xy
#define ASIZE get_x_size
#define BSIZE get_y_size
#define GETVAL(a, b) get_alpha(a, b)
#define SETVAL(a, b, v) set_alpha(a, b, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef FUNCTION_NAME


// These instances scale by Y first, then by X.

#define FUNCTION_NAME filter_red_yx
#define ASIZE get_y_size
#define BSIZE get_x_size
#define GETVAL(a, b) get_red(b, a)
#define SETVAL(a, b, v) set_red(b, a, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_green_yx
#define ASIZE get_y_size
#define BSIZE get_x_size
#define GETVAL(a, b) get_green(b, a)
#define SETVAL(a, b, v) set_green(b, a, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_blue_yx
#define ASIZE get_y_size
#define BSIZE get_x_size
#define GETVAL(a, b) get_blue(b, a)
#define SETVAL(a, b, v) set_blue(b, a, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_gray_yx
#define ASIZE get_y_size
#define BSIZE get_x_size
#define GETVAL(a, b) get_bright(b, a)
#define SETVAL(a, b, v) set_xel(b, a, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef FUNCTION_NAME

#define FUNCTION_NAME filter_alpha_yx
#define ASIZE get_y_size
#define BSIZE get_x_size
#define GETVAL(a, b) get_alpha(b, a)
#define SETVAL(a, b, v) set_alpha(b, a, v)
#include "pnm-image-filter-core.cxx"
#undef SETVAL
#undef GETVAL
#undef BSIZE
#undef ASIZE
#undef FUNCTION_NAME


// filter_image pulls everything together, and filters one image into
// another.  Both images can be the same with no ill effects.
static void
filter_image(PNMImage &dest, const PNMImage &source,
             double width, FilterFunction *make_filter) {

  // We want to scale by the smallest destination axis first, for a
  // slight performance gain.

  if (dest.get_x_size() <= dest.get_y_size()) {
    if (dest.is_grayscale() || source.is_grayscale()) {
      filter_gray_xy(dest, source, width, make_filter);
    } else {
      filter_red_xy(dest, source, width, make_filter);
      filter_green_xy(dest, source, width, make_filter);
      filter_blue_xy(dest, source, width, make_filter);
    }

    if (dest.has_alpha() && source.has_alpha()) {
      filter_alpha_xy(dest, source, width, make_filter);
    }

  } else {
    if (dest.is_grayscale() || source.is_grayscale()) {
      filter_gray_yx(dest, source, width, make_filter);
    } else {
      filter_red_yx(dest, source, width, make_filter);
      filter_green_yx(dest, source, width, make_filter);
      filter_blue_yx(dest, source, width, make_filter);
    }

    if (dest.has_alpha() && source.has_alpha()) {
      filter_alpha_yx(dest, source, width, make_filter);
    }
  }
}



////////////////////////////////////////////////////////////////////
//     Function: PNMImage::box_filter_from
//       Access: Public
//  Description: Makes a resized copy of the indicated image into this
//               one using the indicated filter.  The image to be
//               copied is squashed and stretched to match the
//               dimensions of the current image, applying the
//               appropriate filter to perform the stretching.
////////////////////////////////////////////////////////////////////
void PNMImage::
box_filter_from(double width, const PNMImage &copy) {
  filter_image(*this, copy, width, &box_filter_impl);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::gaussian_filter_from
//       Access: Public
//  Description: Makes a resized copy of the indicated image into this
//               one using the indicated filter.  The image to be
//               copied is squashed and stretched to match the
//               dimensions of the current image, applying the
//               appropriate filter to perform the stretching.
////////////////////////////////////////////////////////////////////
void PNMImage::
gaussian_filter_from(double width, const PNMImage &copy) {
  filter_image(*this, copy, width, &gaussian_filter_impl);
}


//
// The following functions are support for quick_box_filter().
//

INLINE void
box_filter_xel(const PNMImage &image,
               int x, int y, double x_contrib, double y_contrib,
               double &red, double &grn, double &blu, double &alpha,
               double &pixel_count) {
  double contrib = x_contrib * y_contrib;
  red += image.get_red_val(x, y) * contrib;
  grn += image.get_green_val(x, y) * contrib;
  blu += image.get_blue_val(x, y) * contrib;
  if (image.has_alpha()) {
    alpha += image.get_alpha_val(x, y) * contrib;
  }

  pixel_count += contrib;
}


INLINE void
box_filter_line(const PNMImage &image,
                double x0, int y, double x1, double y_contrib,
                double &red, double &grn, double &blu, double &alpha,
                double &pixel_count) {
  int x = (int)x0;
  // Get the first (partial) xel
  box_filter_xel(image, x, y, (double)(x+1)-x0, y_contrib,
                 red, grn, blu, alpha, pixel_count);

  int x_last = (int)x1;
  if (x < x_last) {
    x++;
    while (x < x_last) {
      // Get each consecutive (complete) xel
      box_filter_xel(image, x, y, 1.0, y_contrib,
                     red, grn, blu, alpha, pixel_count);
      x++;
    }

    // Get the final (partial) xel
    double x_contrib = x1 - (double)x_last;
    if (x_contrib > 0.0001) {
      box_filter_xel(image, x, y, x_contrib, y_contrib,
                     red, grn, blu, alpha, pixel_count);
    }
  }
}

static void
box_filter_region(const PNMImage &image,
                  double x0, double y0, double x1, double y1,
                  xel &result, xelval &alpha_result) {
  double red = 0.0, grn = 0.0, blu = 0.0, alpha = 0.0;
  double pixel_count = 0.0;

  assert(y0 >=0 && y1 >=0);

  int y = (int)y0;
  // Get the first (partial) row
  box_filter_line(image, x0, y, x1, (double)(y+1)-y0,
                  red, grn, blu, alpha, pixel_count);

  int y_last = (int)y1;
  if (y < y_last) {
    y++;
    while (y < y_last) {
      // Get each consecutive (complete) row
      box_filter_line(image, x0, y, x1, 1.0,
                      red, grn, blu, alpha, pixel_count);
      y++;
    }

    // Get the final (partial) row
    double y_contrib = y1 - (double)y_last;
    if (y_contrib > 0.0001) {
      box_filter_line(image, x0, y, x1, y_contrib,
                      red, grn, blu, alpha, pixel_count);
    }
  }

  PPM_ASSIGN(result,
             (xelval)(red / pixel_count + 0.5),
             (xelval)(grn / pixel_count + 0.5),
             (xelval)(blu / pixel_count + 0.5));

  alpha_result = (xelval)(alpha / pixel_count + 0.5);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMImage::quick_filter_from
//       Access: Public
//  Description: Resizes from the given image, with a fixed radius of
//               0.5. This is a very specialized and simple algorithm
//               that doesn't handle dropping below the Nyquist rate
//               very well, but is quite a bit faster than the more
//               general box_filter(), above.  If borders are
//               specified, they will further restrict the size of the
//               resulting image. There's no point in using
//               quick_box_filter() on a single image.
////////////////////////////////////////////////////////////////////
void PNMImage::
quick_filter_from(const PNMImage &from, int xborder, int yborder) {
  int from_xs = from.get_x_size();
  int from_ys = from.get_y_size();

  int to_xs = get_x_size() - xborder;
  int to_ys = get_y_size() - yborder;

  int to_xoff = xborder / 2;
  int to_yoff = yborder / 2;

  double from_x0, from_x1, from_y0, from_y1;
  int to_x, to_y;

  double x_scale = (double)from_xs / (double)to_xs;
  double y_scale = (double)from_ys / (double)to_ys;

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

      // Now the box from (from_x0, from_y0) - (from_x1, from_y1)
      // but not including (from_x1, from_y1) maps to the pixel (to_x, to_y).
      xelval alpha_result;
      box_filter_region(from,
                        from_x0, from_y0, from_x1, from_y1,
                        (*this)[to_yoff + to_y][to_xoff + to_x],
                        alpha_result);
      if (has_alpha()) {
        set_alpha_val(to_xoff+to_x, to_yoff+to_y, alpha_result);
      }

      from_x0 = from_x1;
    }
    from_y0 = from_y1;
  }
}
