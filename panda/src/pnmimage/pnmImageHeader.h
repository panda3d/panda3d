// Filename: pnmImageHeader.h
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

#ifndef PNMIMAGEHEADER_H
#define PNMIMAGEHEADER_H

#include "pandabase.h"

#include "pnmimage_base.h"

#include "typedObject.h"
#include "filename.h"

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
class EXPCL_PANDA PNMImageHeader {
public:
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

  INLINE int get_x_size() const;
  INLINE int get_y_size() const;

  INLINE bool has_type() const;
  INLINE PNMFileType *get_type() const;
  INLINE void set_type(PNMFileType *type);

  bool read_header(const Filename &filename, PNMFileType *type = NULL);

  PNMReader *make_reader(const Filename &filename,
                         PNMFileType *type = NULL) const;
  PNMReader *make_reader(istream *file, bool owns_file = true,
                         const Filename &filename = Filename(),
                         string magic_number = string(),
                         PNMFileType *type = NULL) const;

  PNMWriter *make_writer(const Filename &filename,
                         PNMFileType *type = NULL) const;
  PNMWriter *make_writer(ostream *file, bool owns_file = true,
                         const Filename &filename = Filename(),
                         PNMFileType *type = NULL) const;

  static bool read_magic_number(istream *file, string &magic_number,
                                int num_bytes);

  void output(ostream &out) const;

public:
  // These classes are used internally, but must be declared public
  // for fiddly reasons.
  class PixelSpec {
  public:
    INLINE PixelSpec(xelval gray);
    INLINE PixelSpec(xelval gray, xelval alpha);
    INLINE PixelSpec(xelval red, xelval green, xelval blue);
    INLINE PixelSpec(xelval red, xelval green, xelval blue, xelval alpha);
    INLINE PixelSpec(const PixelSpec &copy);
    INLINE void operator = (const PixelSpec &copy);

    INLINE bool operator < (const PixelSpec &other) const;
    void output(ostream &out) const;

    xelval _red, _green, _blue, _alpha;
  };
  typedef pmap<PixelSpec, int> Histogram;
  typedef pvector<PixelSpec> Palette;

protected:
  bool compute_histogram(Histogram &hist, xel *array, xelval *alpha,
                         int max_colors = 0);
  bool compute_palette(Palette &palette, xel *array, xelval *alpha,
                       int max_colors = 0);
  INLINE void record_color(Histogram &hist, const PixelSpec &color);

  int _x_size, _y_size;
  int _num_channels;
  xelval _maxval;
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
