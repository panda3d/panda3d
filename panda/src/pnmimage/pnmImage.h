// Filename: pnmImage.h
// Created by:  drose (14Jun00)
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

#ifndef PNMIMAGE_H
#define PNMIMAGE_H

#include "pandabase.h"

#include "pnmImageHeader.h"
#include "pnmBrush.h"

#include "luse.h"

class PNMReader;
class PNMWriter;
class PNMFileType;
class StackedPerlinNoise2;

////////////////////////////////////////////////////////////////////
//       Class : PNMImage
// Description : The name of this class derives from the fact that we
//               originally implemented it as a layer on top of the
//               "pnm library", based on netpbm, which was built to
//               implement pbm, pgm, and pbm files, and is the
//               underlying support of a number of public-domain image
//               file converters.  Nowadays we are no longer derived
//               directly from the pnm library, mainly to allow
//               support of C++ iostreams instead of the C stdio FILE
//               interface.
//
//               Conceptually, a PNMImage is a two-dimensional array
//               of xels, which are the PNM-defined generic pixel
//               type.  Each xel may have a red, green, and blue
//               component, or (if the image is grayscale) a gray
//               component.  The image may be read in, the individual
//               xels manipulated, and written out again, or a black
//               image may be constructed from scratch.
//
//               The image is of size XSize() by YSize() xels,
//               numbered from top to bottom, left to right, beginning
//               at zero.
//
//               Files can be specified by filename, or by an iostream
//               pointer.  The filename "-" refers to stdin or stdout.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PNMIMAGE PNMImage : public PNMImageHeader {
PUBLISHED:
  INLINE PNMImage();
  PNMImage(const Filename &filename, PNMFileType *type = NULL);
  INLINE PNMImage(int x_size, int y_size, int num_channels = 3,
                  xelval maxval = 255, PNMFileType *type = NULL);
  INLINE PNMImage(const PNMImage &copy);
  INLINE void operator = (const PNMImage &copy);

  INLINE ~PNMImage();

  INLINE xelval clamp_val(int input_value) const;
  INLINE xelval to_val(double input_value) const;
  INLINE double from_val(xelval input_value) const;

  void clear();
  void clear(int x_size, int y_size, int num_channels = 3,
             xelval maxval = 255, PNMFileType *type = NULL);

  void copy_from(const PNMImage &copy);
  void copy_channel(const PNMImage &copy, int src_channel, int dest_channel);
  void copy_header_from(const PNMImageHeader &header);
  void take_from(PNMImage &orig);

  INLINE void fill(double red, double green, double blue);
  INLINE void fill(double gray = 0.0);

  void fill_val(xelval red, xelval green, xelval blue);
  INLINE void fill_val(xelval gray = 0);

  INLINE void alpha_fill(double alpha = 0.0);
  void alpha_fill_val(xelval alpha = 0);

  INLINE void set_read_size(int x_size, int y_size);
  INLINE void clear_read_size();
  INLINE bool has_read_size() const;
  INLINE int get_read_x_size() const;
  INLINE int get_read_y_size() const;

  BLOCKING bool read(const Filename &filename, PNMFileType *type = NULL,
                     bool report_unknown_type = true);
  BLOCKING bool read(istream &data, const string &filename = string(),
                     PNMFileType *type = NULL,
                     bool report_unknown_type = true);
  BLOCKING bool read(PNMReader *reader);

  BLOCKING bool write(const Filename &filename, PNMFileType *type = NULL) const;
  BLOCKING bool write(ostream &data, const string &filename = string(),
                      PNMFileType *type = NULL) const;
  BLOCKING bool write(PNMWriter *writer) const;

  INLINE bool is_valid() const;

  INLINE void set_num_channels(int num_channels);
  void set_color_type(ColorType color_type);

  INLINE void add_alpha();
  INLINE void remove_alpha();
  INLINE void make_grayscale();
  void make_grayscale(double rc, double gc, double bc);
  INLINE void make_rgb();

  BLOCKING void reverse_rows();
  BLOCKING void flip(bool flip_x, bool flip_y, bool transpose);

  BLOCKING void set_maxval(xelval maxval);

  // The *_val() functions return or set the color values in the range
  // [0..get_maxval()].  This range may be different for different
  // images!  Use the corresponding functions (without _val()) to work
  // in the normalized range [0..1].

  INLINE const xel &get_xel_val(int x, int y) const;
  INLINE void set_xel_val(int x, int y, const xel &value);
  INLINE void set_xel_val(int x, int y, xelval r, xelval g, xelval b);
  INLINE void set_xel_val(int x, int y, xelval gray);

  INLINE xelval get_red_val(int x, int y) const;
  INLINE xelval get_green_val(int x, int y) const;
  INLINE xelval get_blue_val(int x, int y) const;
  INLINE xelval get_gray_val(int x, int y) const;
  INLINE xelval get_alpha_val(int x, int y) const;

  INLINE void set_red_val(int x, int y, xelval r);
  INLINE void set_green_val(int x, int y, xelval g);
  INLINE void set_blue_val(int x, int y, xelval b);
  INLINE void set_gray_val(int x, int y, xelval gray);
  INLINE void set_alpha_val(int x, int y, xelval a);

  xelval get_channel_val(int x, int y, int channel) const;
  void set_channel_val(int x, int y, int channel, xelval value);

  PixelSpec get_pixel(int x, int y) const;
  void set_pixel(int x, int y, const PixelSpec &pixel);

  // The corresponding get_xel(), set_xel(), get_red(), etc. functions
  // automatically scale their values by get_maxval() into the range
  // [0..1].

  INLINE LRGBColord get_xel(int x, int y) const;
  INLINE void set_xel(int x, int y, const LRGBColord &value);
  INLINE void set_xel(int x, int y, double r, double g, double b);
  INLINE void set_xel(int x, int y, double gray);

  INLINE LColord get_xel_a(int x, int y) const;
  INLINE void set_xel_a(int x, int y, const LColord &value);
  INLINE void set_xel_a(int x, int y, double r, double g, double b, double a);

  INLINE double get_red(int x, int y) const;
  INLINE double get_green(int x, int y) const;
  INLINE double get_blue(int x, int y) const;
  INLINE double get_gray(int x, int y) const;
  INLINE double get_alpha(int x, int y) const;

  INLINE void set_red(int x, int y, double r);
  INLINE void set_green(int x, int y, double g);
  INLINE void set_blue(int x, int y, double b);
  INLINE void set_gray(int x, int y, double gray);
  INLINE void set_alpha(int x, int y, double a);

  INLINE double get_channel(int x, int y, int channel) const;
  INLINE void set_channel(int x, int y, int channel, double value);

  INLINE double get_bright(int x, int y) const;
  INLINE double get_bright(int x, int y, double rc, double gc,
                           double bc) const;
  INLINE double get_bright(int x, int y, double rc, double gc,
                           double bc, double ac) const;

  INLINE void blend(int x, int y, const LRGBColord &val, double alpha);
  void blend(int x, int y, double r, double g, double b, double alpha);

  // If you're used to the NetPBM library and like working with a 2-d
  // array of xels, and using the PNM macros to access their components,
  // you may treat the PNMImage as such directly.

  INLINE xel *operator [] (int y);
  INLINE const xel *operator [] (int y) const;

  void copy_sub_image(const PNMImage &copy, int xto, int yto,
                      int xfrom = 0, int yfrom = 0,
                      int x_size = -1, int y_size = -1);
  void blend_sub_image(const PNMImage &copy, int xto, int yto,
                       int xfrom = 0, int yfrom = 0,
                       int x_size = -1, int y_size = -1,
                       double pixel_scale = 1.0);
  void add_sub_image(const PNMImage &copy, int xto, int yto,
                     int xfrom = 0, int yfrom = 0,
                     int x_size = -1, int y_size = -1,
                     double pixel_scale = 1.0);
  void mult_sub_image(const PNMImage &copy, int xto, int yto,
                      int xfrom = 0, int yfrom = 0,
                      int x_size = -1, int y_size = -1,
                      double pixel_scale = 1.0);
  void darken_sub_image(const PNMImage &copy, int xto, int yto,
                        int xfrom = 0, int yfrom = 0,
                        int x_size = -1, int y_size = -1,
                        double pixel_scale = 1.0);
  void lighten_sub_image(const PNMImage &copy, int xto, int yto,
                         int xfrom = 0, int yfrom = 0,
                         int x_size = -1, int y_size = -1,
                         double pixel_scale = 1.0);
  void threshold(const PNMImage &select_image, int channel, double threshold,
                 const PNMImage &lt, const PNMImage &ge);
  BLOCKING void fill_distance_inside(const PNMImage &mask, double threshold, int radius, bool shrink_from_border);
  BLOCKING void fill_distance_outside(const PNMImage &mask, double threshold, int radius);

  void rescale(double min_val, double max_val);

  void copy_channel(const PNMImage &copy, int xto, int yto, int cto,
                    int xfrom = 0, int yfrom = 0, int cfrom = 0,
                    int x_size = -1, int y_size = -1);

  void render_spot(const LColord &fg, const LColord &bg,
                   double min_radius, double max_radius);

  void expand_border(int left, int right, int bottom, int top,
                     const LColord &color);

  // The bodies for the non-inline *_filter() functions can be found
  // in the file pnm-image-filter.cxx.

  INLINE void box_filter(double radius = 1.0);
  INLINE void gaussian_filter(double radius = 1.0);

  void unfiltered_stretch_from(const PNMImage &copy);
  void box_filter_from(double radius, const PNMImage &copy);
  void gaussian_filter_from(double radius, const PNMImage &copy);
  void quick_filter_from(const PNMImage &copy,
                         int xborder = 0, int yborder = 0);

  void make_histogram(Histogram &hist);
  void perlin_noise_fill(double sx, double sy, int table_size = 256,
                         unsigned long seed = 0);
  void perlin_noise_fill(StackedPerlinNoise2 &perlin);

  void remix_channels(const LMatrix4 &conv);
  INLINE void gamma_correct(double from_gamma, double to_gamma);
  INLINE void gamma_correct_alpha(double from_gamma, double to_gamma);
  INLINE void apply_exponent(double gray_exponent);
  INLINE void apply_exponent(double gray_exponent, double alpha_exponent);
  INLINE void apply_exponent(double red_exponent, double green_exponent, double blue_exponent);
  void apply_exponent(double red_exponent, double green_exponent, double blue_exponent, double alpha_exponent);

  LRGBColord get_average_xel() const;
  LColord get_average_xel_a() const;
  double get_average_gray() const;

  void do_fill_distance(int xi, int yi, int d);

public:
  // Know what you are doing if you access the underlying data arrays
  // directly.
  INLINE xel *get_array();
  INLINE const xel *get_array() const;
  INLINE xelval *get_alpha_array();
  INLINE const xelval *get_alpha_array() const;

  INLINE xel *take_array();
  INLINE xelval *take_alpha_array();
  void set_array(xel *array);
  void set_alpha_array(xelval *alpha);

private:
  INLINE void allocate_array();
  INLINE void allocate_alpha();

  INLINE xel *row(int row) const;
  INLINE xelval *alpha_row(int row) const;

  INLINE void setup_sub_image(const PNMImage &copy, int &xto, int &yto,
                              int &xfrom, int &yfrom, int &x_size, int &y_size,
                              int &xmin, int &ymin, int &xmax, int &ymax);

  INLINE static void compute_spot_pixel(LColord &c, double d2,
                                        double min_radius, double max_radius,
                                        const LColord &fg, const LColord &bg);

  void setup_rc();

PUBLISHED:
  PNMImage operator ~() const;

  INLINE PNMImage operator + (const PNMImage &other) const;
  INLINE PNMImage operator + (const LColord &other) const;
  INLINE PNMImage operator - (const PNMImage &other) const;
  INLINE PNMImage operator - (const LColord &other) const;
  INLINE PNMImage operator * (const PNMImage &other) const;
  INLINE PNMImage operator * (double multiplier) const;
  INLINE PNMImage operator * (const LColord &other) const;
  void operator += (const PNMImage &other);
  void operator += (const LColord &other);
  void operator -= (const PNMImage &other);
  void operator -= (const LColord &other);
  void operator *= (const PNMImage &other);
  void operator *= (double multiplier);
  void operator *= (const LColord &other);

private:
  xel *_array;
  xelval *_alpha;
  double _default_rc, _default_gc, _default_bc;

  int _read_x_size, _read_y_size;
  bool _has_read_size;
};

#include "pnmImage.I"

#endif
