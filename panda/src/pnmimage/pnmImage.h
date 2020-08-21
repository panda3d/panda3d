/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmImage.h
 * @author drose
 * @date 2000-06-14
 */

#ifndef PNMIMAGE_H
#define PNMIMAGE_H

#include "pandabase.h"

#include "pnmImageHeader.h"
#include "pnmBrush.h"
#include "stackedPerlinNoise2.h"
#include "convert_srgb.h"
#include "luse.h"

class PNMReader;
class PNMWriter;
class PNMFileType;

/**
 * The name of this class derives from the fact that we originally implemented
 * it as a layer on top of the "pnm library", based on netpbm, which was built
 * to implement pbm, pgm, and pbm files, and is the underlying support of a
 * number of public-domain image file converters.  Nowadays we are no longer
 * derived directly from the pnm library, mainly to allow support of C++
 * iostreams instead of the C stdio FILE interface.
 *
 * Conceptually, a PNMImage is a two-dimensional array of xels, which are the
 * PNM-defined generic pixel type.  Each xel may have a red, green, and blue
 * component, or (if the image is grayscale) a gray component.  The image may
 * be read in, the individual xels manipulated, and written out again, or a
 * black image may be constructed from scratch.
 *
 * A PNMImage has a color space and a maxval, the combination of which defines
 * how a floating-point linear color value is encoded as an integer value in
 * memory.  The functions ending in _val operate on encoded colors, whereas
 * the regular ones work with linear floating-point values.  All operations
 * are color space correct unless otherwise specified.
 *
 * The image is of size XSize() by YSize() xels, numbered from top to bottom,
 * left to right, beginning at zero.
 *
 * Files can be specified by filename, or by an iostream pointer.  The
 * filename "-" refers to stdin or stdout.
 *
 * This class is not inherently thread-safe; use it from a single thread or
 * protect access using a mutex.
 */
class EXPCL_PANDA_PNMIMAGE PNMImage : public PNMImageHeader {
PUBLISHED:
  INLINE PNMImage();
  explicit PNMImage(const Filename &filename, PNMFileType *type = nullptr);
  INLINE explicit PNMImage(int x_size, int y_size, int num_channels = 3,
                           xelval maxval = 255, PNMFileType *type = nullptr,
                           ColorSpace color_space = CS_linear);
  INLINE PNMImage(const PNMImage &copy);
  INLINE void operator = (const PNMImage &copy);

  INLINE ~PNMImage();

  INLINE xelval clamp_val(int input_value) const;
  INLINE xel to_val(const LRGBColorf &input_value) const;
  INLINE xelval to_val(float input_value) const;
  INLINE xelval to_alpha_val(float input_value) const;
  INLINE LRGBColorf from_val(const xel &input_value) const;
  INLINE float from_val(xelval input_value) const;
  INLINE float from_alpha_val(xelval input_value) const;

  void clear();
  void clear(int x_size, int y_size, int num_channels = 3,
             xelval maxval = 255, PNMFileType *type = nullptr,
             ColorSpace color_space = CS_linear);

  void copy_from(const PNMImage &copy);
  void copy_channel(const PNMImage &copy, int src_channel, int dest_channel);
  void copy_channel_bits(const PNMImage &copy, int src_channel, int dest_channel, xelval src_mask, int right_shift);
  void copy_header_from(const PNMImageHeader &header);
  void take_from(PNMImage &orig);

  INLINE void fill(float red, float green, float blue);
  INLINE void fill(float gray = 0.0);

  void fill_val(xelval red, xelval green, xelval blue);
  INLINE void fill_val(xelval gray = 0);

  INLINE void alpha_fill(float alpha = 0.0);
  void alpha_fill_val(xelval alpha = 0);

  INLINE void set_read_size(int x_size, int y_size);
  INLINE void clear_read_size();
  INLINE bool has_read_size() const;
  INLINE int get_read_x_size() const;
  INLINE int get_read_y_size() const;
  INLINE ColorSpace get_color_space() const;

  BLOCKING bool read(const Filename &filename, PNMFileType *type = nullptr,
                     bool report_unknown_type = true);
  BLOCKING bool read(std::istream &data, const std::string &filename = std::string(),
                     PNMFileType *type = nullptr,
                     bool report_unknown_type = true);
  BLOCKING bool read(PNMReader *reader);

  BLOCKING bool write(const Filename &filename, PNMFileType *type = nullptr) const;
  BLOCKING bool write(std::ostream &data, const std::string &filename = std::string(),
                      PNMFileType *type = nullptr) const;
  BLOCKING bool write(PNMWriter *writer) const;

  INLINE bool is_valid() const;

  INLINE void set_num_channels(int num_channels);
  void set_color_type(ColorType color_type);
  void set_color_space(ColorSpace color_space);

  INLINE void add_alpha();
  INLINE void remove_alpha();
  INLINE void make_grayscale();
  void make_grayscale(float rc, float gc, float bc);
  INLINE void make_rgb();

  BLOCKING void premultiply_alpha();
  BLOCKING void unpremultiply_alpha();

  BLOCKING void reverse_rows();
  BLOCKING void flip(bool flip_x, bool flip_y, bool transpose);

  BLOCKING void set_maxval(xelval maxval);

  // The *_val() functions return or set the color values in the range
  // [0..get_maxval()].  This range may be different for different images!
  // Use the corresponding functions (without _val()) to work in the
  // normalized range [0..1].  These return values in the image's stored color
  // space.

  INLINE xel &get_xel_val(int x, int y);
  INLINE xel get_xel_val(int x, int y) const;
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
  float get_channel(int x, int y, int channel) const;
  void set_channel(int x, int y, int channel, float value);

  PixelSpec get_pixel(int x, int y) const;
  void set_pixel(int x, int y, const PixelSpec &pixel);

  // The corresponding get_xel(), set_xel(), get_red(), etc.  functions
  // automatically scale their values by get_maxval() into the range [0..1],
  // and into the linear color space.

  INLINE LRGBColorf get_xel(int x, int y) const;
  INLINE void set_xel(int x, int y, const LRGBColorf &value);
  INLINE void set_xel(int x, int y, float r, float g, float b);
  INLINE void set_xel(int x, int y, float gray);

  INLINE LColorf get_xel_a(int x, int y) const;
  INLINE void set_xel_a(int x, int y, const LColorf &value);
  INLINE void set_xel_a(int x, int y, float r, float g, float b, float a);

  INLINE float get_red(int x, int y) const;
  INLINE float get_green(int x, int y) const;
  INLINE float get_blue(int x, int y) const;
  INLINE float get_gray(int x, int y) const;
  INLINE float get_alpha(int x, int y) const;

  INLINE void set_red(int x, int y, float r);
  INLINE void set_green(int x, int y, float g);
  INLINE void set_blue(int x, int y, float b);
  INLINE void set_gray(int x, int y, float gray);
  INLINE void set_alpha(int x, int y, float a);

  INLINE float get_bright(int x, int y) const;
  INLINE float get_bright(int x, int y, float rc, float gc,
                           float bc) const;
  INLINE float get_bright(int x, int y, float rc, float gc,
                           float bc, float ac) const;

  INLINE void blend(int x, int y, const LRGBColorf &val, float alpha);
  void blend(int x, int y, float r, float g, float b, float alpha);

  void copy_sub_image(const PNMImage &copy, int xto, int yto,
                      int xfrom = 0, int yfrom = 0,
                      int x_size = -1, int y_size = -1);
  void blend_sub_image(const PNMImage &copy, int xto, int yto,
                       int xfrom = 0, int yfrom = 0,
                       int x_size = -1, int y_size = -1,
                       float pixel_scale = 1.0);
  void add_sub_image(const PNMImage &copy, int xto, int yto,
                     int xfrom = 0, int yfrom = 0,
                     int x_size = -1, int y_size = -1,
                     float pixel_scale = 1.0);
  void mult_sub_image(const PNMImage &copy, int xto, int yto,
                      int xfrom = 0, int yfrom = 0,
                      int x_size = -1, int y_size = -1,
                      float pixel_scale = 1.0);
  void darken_sub_image(const PNMImage &copy, int xto, int yto,
                        int xfrom = 0, int yfrom = 0,
                        int x_size = -1, int y_size = -1,
                        float pixel_scale = 1.0);
  void lighten_sub_image(const PNMImage &copy, int xto, int yto,
                         int xfrom = 0, int yfrom = 0,
                         int x_size = -1, int y_size = -1,
                         float pixel_scale = 1.0);
  void threshold(const PNMImage &select_image, int channel, float threshold,
                 const PNMImage &lt, const PNMImage &ge);
  BLOCKING void fill_distance_inside(const PNMImage &mask, float threshold, int radius, bool shrink_from_border);
  BLOCKING void fill_distance_outside(const PNMImage &mask, float threshold, int radius);

  void indirect_1d_lookup(const PNMImage &index_image, int channel,
                          const PNMImage &pixel_values);

  void rescale(float min_val, float max_val);

  void copy_channel(const PNMImage &copy, int xto, int yto, int cto,
                    int xfrom = 0, int yfrom = 0, int cfrom = 0,
                    int x_size = -1, int y_size = -1);

  void render_spot(const LColorf &fg, const LColorf &bg,
                   float min_radius, float max_radius);

  void expand_border(int left, int right, int bottom, int top,
                     const LColorf &color);

  // The bodies for the non-inline *_filter() functions can be found in the
  // file pnm-image-filter.cxx.

  BLOCKING INLINE void box_filter(float radius = 1.0);
  BLOCKING INLINE void gaussian_filter(float radius = 1.0);

  BLOCKING void unfiltered_stretch_from(const PNMImage &copy);
  BLOCKING void box_filter_from(float radius, const PNMImage &copy);
  BLOCKING void gaussian_filter_from(float radius, const PNMImage &copy);
  BLOCKING void quick_filter_from(const PNMImage &copy,
                                  int xborder = 0, int yborder = 0);

  void make_histogram(Histogram &hist);
  void quantize(size_t max_colors);
  BLOCKING void perlin_noise_fill(float sx, float sy, int table_size = 256,
                                  unsigned long seed = 0);
  void perlin_noise_fill(StackedPerlinNoise2 &perlin);

  void remix_channels(const LMatrix4 &conv);
  BLOCKING INLINE void gamma_correct(float from_gamma, float to_gamma);
  BLOCKING INLINE void gamma_correct_alpha(float from_gamma, float to_gamma);
  BLOCKING INLINE void apply_exponent(float gray_exponent);
  BLOCKING INLINE void apply_exponent(float gray_exponent, float alpha_exponent);
  BLOCKING INLINE void apply_exponent(float red_exponent, float green_exponent, float blue_exponent);
  BLOCKING void apply_exponent(float red_exponent, float green_exponent, float blue_exponent, float alpha_exponent);

  LRGBColorf get_average_xel() const;
  LColorf get_average_xel_a() const;
  float get_average_gray() const;

  void do_fill_distance(int xi, int yi, int d);

PUBLISHED:
  // Provides an accessor for reading or writing the contents of one row of
  // the image in-place.
  class EXPCL_PANDA_PNMIMAGE Row {
  PUBLISHED:
    INLINE size_t size() const;
    INLINE LColorf operator[](int x) const;
#ifdef HAVE_PYTHON
    INLINE void __setitem__(int x, const LColorf &v);
#endif
    INLINE xel &get_xel_val(int x);
    INLINE void set_xel_val(int x, const xel &v);
    INLINE xelval get_alpha_val(int x) const;
    INLINE void set_alpha_val(int x, xelval v);

  public:
    INLINE Row(PNMImage &image, int y);

  private:
    PNMImage &_image;
    int _y;
  };

  // Provides an accessor for reading the contents of one row of the image in-
  // place.
  class EXPCL_PANDA_PNMIMAGE CRow {
  PUBLISHED:
    INLINE size_t size() const;
    INLINE LColorf operator[](int x) const;
    INLINE xel get_xel_val(int x) const;
    INLINE xelval get_alpha_val(int x) const;

  public:
    INLINE CRow(const PNMImage &image, int y);

  private:
    const PNMImage &_image;
    int _y;
  };

  INLINE Row operator [] (int y);
  INLINE CRow operator [] (int y) const;

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

  INLINE static void compute_spot_pixel(LColorf &c, float d2,
                                        float min_radius, float max_radius,
                                        const LColorf &fg, const LColorf &bg);

  void setup_rc();
  void setup_encoding();

  void r_quantize(pmap<xel, xel> &color_map, size_t max_colors,
                  xel *colors, size_t num_colors);

PUBLISHED:
  PNMImage operator ~() const;

  INLINE PNMImage operator + (const PNMImage &other) const;
  INLINE PNMImage operator + (const LColorf &other) const;
  INLINE PNMImage operator - (const PNMImage &other) const;
  INLINE PNMImage operator - (const LColorf &other) const;
  INLINE PNMImage operator * (const PNMImage &other) const;
  INLINE PNMImage operator * (float multiplier) const;
  INLINE PNMImage operator * (const LColorf &other) const;
  void operator += (const PNMImage &other);
  void operator += (const LColorf &other);
  void operator -= (const PNMImage &other);
  void operator -= (const LColorf &other);
  void operator *= (const PNMImage &other);
  void operator *= (float multiplier);
  void operator *= (const LColorf &other);

private:
  friend class Row;
  friend class Texture;

  xel *_array;
  xelval *_alpha;
  float _default_rc, _default_gc, _default_bc;

  int _read_x_size, _read_y_size;
  bool _has_read_size;

  // The reciprocal of _maxval, as an optimization for from_val.
  float _inv_maxval;

  // These method pointers contain the implementation for to_val and from_val,
  // respectively, dependent on the maxval and color space.
  ColorSpace _color_space;

  // The following enum determines which code path we should take in the
  // set_xel and get_xel methods.
  enum XelEncoding {
    XE_generic,
    XE_generic_alpha,
    XE_generic_sRGB,
    XE_generic_sRGB_alpha,
    XE_uchar_sRGB,
    XE_uchar_sRGB_alpha,
    XE_uchar_sRGB_sse2,
    XE_uchar_sRGB_alpha_sse2,
    XE_scRGB,
    XE_scRGB_alpha
  } _xel_encoding;
};

#include "pnmImage.I"

#endif
