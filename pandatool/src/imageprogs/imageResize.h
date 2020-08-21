/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageResize.h
 * @author drose
 * @date 2003-03-13
 */

#ifndef IMAGERESIZE_H
#define IMAGERESIZE_H

#include "pandatoolbase.h"

#include "imageFilter.h"

/**
 * A program to read an image file and resize it to a larger or smaller image
 * file.
 */
class ImageResize : public ImageFilter {
public:
  ImageResize();

  void run();

private:
  static bool dispatch_size_request(const std::string &opt, const std::string &arg, void *var);

  enum RequestType {
    RT_none,
    RT_pixel_size,
    RT_ratio,
  };
  class SizeRequest {
  public:
    INLINE SizeRequest();
    INLINE RequestType get_type() const;

    INLINE void set_pixel_size(int pixel_size);
    INLINE int get_pixel_size() const;
    INLINE int get_pixel_size(int orig_pixel_size) const;
    INLINE void set_ratio(double ratio);
    INLINE double get_ratio() const;
    INLINE double get_ratio(int orig_pixel_size) const;

  private:
    RequestType _type;
    union {
      int _pixel_size;
      double _ratio;
    } _e;
  };

  SizeRequest _x_size;
  SizeRequest _y_size;

  bool _use_gaussian_filter;
  double _filter_radius;
};

#include "imageResize.I"

#endif
