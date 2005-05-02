// Filename: pnmImage.h
// Created by:  drose (14Jun00)
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

#ifndef PNMIMAGE_H
#define PNMIMAGE_H

#include "pandabase.h"

#include "pnmImageHeader.h"

#include "luse.h"

class PNMReader;
class PNMWriter;
class PNMFileType;

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
class EXPCL_PANDA PNMImage : public PNMImageHeader {
PUBLISHED:
  INLINE PNMImage();
  INLINE PNMImage(const Filename &filename, PNMFileType *type = NULL);
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
  void copy_header_from(const PNMImageHeader &header);

  INLINE void fill(double red, double green, double blue);
  INLINE void fill(double gray = 0.0);

  void fill_val(xelval red, xelval green, xelval blue);
  INLINE void fill_val(xelval gray = 0);

  INLINE void alpha_fill(double alpha = 0.0);
  void alpha_fill_val(xelval alpha = 0);

  bool read(const Filename &filename, PNMFileType *type = NULL);
  bool read(istream &data, const string &filename = string(), 
            PNMFileType *type = NULL);
  bool read(PNMReader *reader);

  bool write(const Filename &filename, PNMFileType *type = NULL) const;
  bool write(ostream &data, const string &filename = string(),
             PNMFileType *type = NULL) const;
  bool write(PNMWriter *writer) const;

  INLINE bool is_valid() const;

  INLINE void set_num_channels(int num_channels);
  void set_color_type(ColorType color_type);

  INLINE void add_alpha();
  INLINE void remove_alpha();
  INLINE void make_grayscale();
  void make_grayscale(double rc, double gc, double bc);
  INLINE void make_rgb();

  void set_maxval(xelval maxval);

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

  // The corresponding get_xel(), set_xel(), get_red(), etc. functions
  // automatically scale their values by get_maxval() into the range
  // [0..1].

  INLINE RGBColord get_xel(int x, int y) const;
  INLINE void set_xel(int x, int y, const RGBColord &value);
  INLINE void set_xel(int x, int y, double r, double g, double b);
  INLINE void set_xel(int x, int y, double gray);

  INLINE Colord get_xel_a(int x, int y) const;
  INLINE void set_xel_a(int x, int y, const Colord &value);
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

  INLINE void blend(int x, int y, const RGBColord &val, double alpha);
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
                       int x_size = -1, int y_size = -1);

  void render_spot(const Colord &fg, const Colord &bg,
                   double min_radius, double max_radius);

  // The bodies for the non-inline *_filter() functions can be found
  // in the file pnm-image-filter.cxx.

  INLINE void box_filter(double radius = 1.0);
  INLINE void gaussian_filter(double radius = 1.0);

  void box_filter_from(double radius, const PNMImage &copy);
  void gaussian_filter_from(double radius, const PNMImage &copy);
  void quick_filter_from(const PNMImage &copy,
                         int xborder = 0, int yborder = 0);

private:
  INLINE void allocate_array();
  INLINE void allocate_alpha();

  INLINE xel *row(int row) const;
  INLINE xelval *alpha_row(int row) const;

  void setup_rc();

private:
  xel *_array;
  xelval *_alpha;
  double _default_rc, _default_gc, _default_bc;
};

#include "pnmImage.I"

#endif
