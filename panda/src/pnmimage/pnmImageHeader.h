// Filename: pnmImageHeader.h
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

#ifndef PNMIMAGEHEADER_H
#define PNMIMAGEHEADER_H

#include "pandabase.h"

#include "pnmimage_base.h"

#include "typedObject.h"
#include "filename.h"
#include "pnotify.h"
#include "pmap.h"
#include "pvector.h"
#include "colorSpace.h"

class PNMFileType;
class PNMReader;
class PNMWriter;

////////////////////////////////////////////////////////////////////
//       Class : PNMImageHeader
// Description : This is the base class of PNMImage, PNMReader, and
//               PNMWriter.  It encapsulates all the information
//               associated with an image that describes its size,
//               number of channels, etc; that is, all the information
//               about the image except the image data itself.  It's
//               the sort of information you typically read from the
//               image file's header.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PNMIMAGE PNMImageHeader {
PUBLISHED:
  INLINE PNMImageHeader();
  INLINE PNMImageHeader(const PNMImageHeader &copy);
  INLINE void operator = (const PNMImageHeader &copy);
  INLINE ~PNMImageHeader();

  // This enumerated type indicates the number of channels in the
  // image, and also implies an image type.  You can treat it either
  // as an integer number of channels or as an enumerated image type.
  enum ColorType {
    CT_invalid      = 0,
    CT_grayscale    = 1,
    CT_two_channel  = 2,
    CT_color        = 3,
    CT_four_channel = 4,
  };

  INLINE ColorType get_color_type() const;
  INLINE int get_num_channels() const;

  INLINE static bool is_grayscale(ColorType color_type);
  INLINE bool is_grayscale() const;

  INLINE static bool has_alpha(ColorType color_type);
  INLINE bool has_alpha() const;

  INLINE xelval get_maxval() const;
  INLINE ColorSpace get_color_space() const;

  INLINE int get_x_size() const;
  INLINE int get_y_size() const;

  INLINE string get_comment() const;
  INLINE void set_comment(const string &comment);

  INLINE bool has_type() const;
  INLINE PNMFileType *get_type() const;
  INLINE void set_type(PNMFileType *type);

  BLOCKING bool read_header(const Filename &filename, PNMFileType *type = NULL,
                            bool report_unknown_type = true);
  BLOCKING bool read_header(istream &data, const string &filename = string(),
                            PNMFileType *type = NULL, bool report_unknown_type = true);

  PNMReader *make_reader(const Filename &filename,
                         PNMFileType *type = NULL,
                         bool report_unknown_type = true) const;
  PNMReader *make_reader(istream *file, bool owns_file = true,
                         const Filename &filename = Filename(),
                         string magic_number = string(),
                         PNMFileType *type = NULL,
                         bool report_unknown_type = true) const;

  PNMWriter *make_writer(const Filename &filename,
                         PNMFileType *type = NULL) const;
  PNMWriter *make_writer(ostream *file, bool owns_file = true,
                         const Filename &filename = Filename(),
                         PNMFileType *type = NULL) const;

  static bool read_magic_number(istream *file, string &magic_number,
                                int num_bytes);

  void output(ostream &out) const;

  // Contains a single pixel specification used in compute_histogram()
  // and make_histogram().  Note that pixels are stored by integer
  // value, not by floating-point scaled value.
  class EXPCL_PANDA_PNMIMAGE PixelSpec {
  PUBLISHED:
    INLINE PixelSpec(xelval gray_value);
    INLINE PixelSpec(xelval gray_value, xelval alpha);
    INLINE PixelSpec(xelval red, xelval green, xelval blue);
    INLINE PixelSpec(xelval red, xelval green, xelval blue, xelval alpha);
    INLINE PixelSpec(const xel &rgb);
    INLINE PixelSpec(const xel &rgb, xelval alpha);
    INLINE PixelSpec(const PixelSpec &copy);
    INLINE void operator = (const PixelSpec &copy);

    INLINE bool operator < (const PixelSpec &other) const;
    INLINE bool operator == (const PixelSpec &other) const;
    INLINE bool operator != (const PixelSpec &other) const;
    INLINE int compare_to(const PixelSpec &other) const;

    INLINE xelval get_red() const;
    INLINE xelval get_green() const;
    INLINE xelval get_blue() const;
    INLINE xelval get_alpha() const;

    INLINE void set_red(xelval red);
    INLINE void set_green(xelval green);
    INLINE void set_blue(xelval blue);
    INLINE void set_alpha(xelval alpha);

    INLINE xelval operator [](int n) const;
    INLINE static int size();

    void output(ostream &out) const;

  public:
    xelval _red, _green, _blue, _alpha;
  };

  // Associates a pixel specification with an appearance count, for
  // use in Histogram, below.
  class EXPCL_PANDA_PNMIMAGE PixelSpecCount {
  public:
    INLINE PixelSpecCount(const PixelSpec &pixel, int count);
    INLINE bool operator < (const PixelSpecCount &other) const;

    PixelSpec _pixel;
    int _count;
  };

  typedef pmap<PixelSpec, int> HistMap;
  typedef pvector<PixelSpecCount> PixelCount;
  typedef pvector<PixelSpec> Palette;

  // Used to return a pixel histogram in PNMImage::get_histogram().
  class EXPCL_PANDA_PNMIMAGE Histogram {
  PUBLISHED:
    INLINE Histogram();

    INLINE int get_num_pixels() const;
    INLINE const PixelSpec &get_pixel(int n) const;
    INLINE int get_count(int n) const;
    INLINE int get_count(const PixelSpec &pixel) const;
    MAKE_SEQ(get_pixels, get_num_pixels, get_pixel);

    void write(ostream &out) const;

  public:
    INLINE void swap(PixelCount &pixels, HistMap &hist_map);

  private:
    PixelCount _pixels;
    HistMap _hist_map;
  };

protected:
  bool compute_histogram(HistMap &hist, xel *array, xelval *alpha,
                         int max_colors = 0);
  bool compute_palette(Palette &palette, xel *array, xelval *alpha,
                       int max_colors = 0);
  INLINE void record_color(HistMap &hist, const PixelSpec &color);

  int _x_size, _y_size;
  int _num_channels;
  xelval _maxval;
  ColorSpace _color_space;
  string _comment;
  PNMFileType *_type;
};

INLINE ostream &operator << (ostream &out, const PNMImageHeader &header) {
  header.output(out);
  return out;
}

INLINE ostream &operator << (ostream &out, const PNMImageHeader::PixelSpec &pixel) {
  pixel.output(out);
  return out;
}

#include "pnmImageHeader.I"

#endif
