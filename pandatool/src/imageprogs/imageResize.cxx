/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageResize.cxx
 * @author drose
 * @date 2003-03-13
 */

#include "imageResize.h"
#include "string_utils.h"

/**
 *
 */
ImageResize::
ImageResize() : ImageFilter(true) {
  set_program_brief("resize an image file");
  set_program_description
    ("This program reads an image file and resizes it to a larger or smaller "
     "image file.");

  add_option
    ("x", "xsize", 0,
     "Specify the width of the output image in pixels, or as a percentage "
     "of the original width (if a trailing percent sign is included).  "
     "If this is omitted, the ratio is taken from the ysize parameter.",
     &ImageResize::dispatch_size_request, nullptr, &_x_size);

  add_option
    ("y", "ysize", 0,
     "Specify the height of the output image in pixels, or as a percentage "
     "of the original height (if a trailing percent sign is included).  "
     "If this is omitted, the ratio is taken from the xsize parameter.",
     &ImageResize::dispatch_size_request, nullptr, &_y_size);

  add_option
    ("g", "radius", 0,
     "Use Gaussian filtering to resize the image, with the indicated radius.",
     &ImageResize::dispatch_double, &_use_gaussian_filter, &_filter_radius);

  add_option
    ("1", "", 0,
     "This option is ignored.  It is provided only for backward compatibility "
     "with a previous version of image-resize.",
     &ImageResize::dispatch_none, nullptr, nullptr);

  _filter_radius = 1.0;
}

/**
 *
 */
void ImageResize::
run() {
  if (_x_size.get_type() == RT_none && _y_size.get_type() == RT_none) {
    _x_size.set_ratio(1.0);
    _y_size.set_ratio(1.0);
  } else if (_x_size.get_type() == RT_none) {
    _x_size.set_ratio(_y_size.get_ratio(_image.get_y_size()));
  } else if (_y_size.get_type() == RT_none) {
    _y_size.set_ratio(_x_size.get_ratio(_image.get_x_size()));
  }

  int x_size = _x_size.get_pixel_size(_image.get_x_size());
  int y_size = _y_size.get_pixel_size(_image.get_y_size());

  nout << "Resizing to " << x_size << " x " << y_size << "\n";
  PNMImage new_image(x_size, y_size,
                     _image.get_num_channels(),
                     _image.get_maxval(), _image.get_type());

  if (_use_gaussian_filter) {
    new_image.gaussian_filter_from(_filter_radius, _image);
  } else {
    new_image.quick_filter_from(_image);
  }

  write_image(new_image);
}

/**
 * Interprets the -x or -y parameters.
 */
bool ImageResize::
dispatch_size_request(const std::string &opt, const std::string &arg, void *var) {
  SizeRequest *ip = (SizeRequest *)var;
  if (!arg.empty() && arg[arg.length() - 1] == '%') {
    // A ratio.
    std::string str = arg.substr(0, arg.length() - 1);
    double ratio;
    if (!string_to_double(str, ratio)) {
      nout << "Invalid ratio for -" << opt << ": "
           << str << "\n";
      return false;
    }
    ip->set_ratio(ratio / 100.0);

  } else {
    // A pixel size.
    int pixel_size;
    if (!string_to_int(arg, pixel_size)) {
      nout << "Invalid pixel size for -" << opt << ": "
           << arg << "\n";
      return false;
    }
    ip->set_pixel_size(pixel_size);
  }

  return true;
}


int main(int argc, char *argv[]) {
  ImageResize prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
