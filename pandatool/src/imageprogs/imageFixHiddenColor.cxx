/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageFixHiddenColor.cxx
 * @author drose
 * @date 2003-03-13
 */

#include "imageFixHiddenColor.h"
#include "string_utils.h"

/**
 *
 */
ImageFixHiddenColor::
ImageFixHiddenColor() : ImageFilter(true) {
  set_program_brief("change the color of transparent pixels in an image");
  set_program_description
    ("This program is designed to fix the color channels of an "
     "alpha-cutout image, making the \"color\" of the invisible part of the "
     "image consistent with the rest of the image.  It does this by "
     "analyzing the RGB values of the image where alpha == 1, and using the "
     "average of these values as the overall image color, which it then "
     "applies to the image wherever alpha == 0.  When the alpha value is "
     "neither 1 nor 0, the pixel is ignored.\n\n"

     "This process is important for applications that filter the texture "
     "size down by averaging neighboring pixels.  If the color under the "
     "alpha == 0 pixels is very different from the color elsewhere, this "
     "kind of filtering can have a visible effect on the image's color, even "
     "where alpha != 0.");

  add_option
    ("alpha", "filename", 0,
     "Specifies a separate filename that will be used in lieu of the "
     "alpha channel on the source image.  If this file has an alpha "
     "channel, that alpha channel is used; otherwise, the grayscale "
     "value of the image is used.",
     &ImageFixHiddenColor::dispatch_filename, nullptr, &_alpha_filename);

  add_option
    ("opaque", "alpha", 0,
     "Specifies the minimum alpha value (in the range of 0 to 1) for a "
     "pixel to be considered fully opaque.  The default is 1.",
     &ImageFixHiddenColor::dispatch_double, nullptr, &_min_opaque_alpha);

  add_option
    ("transparent", "alpha", 0,
     "Specifies the maximum alpha value (in the range of 0 to 1) for a "
     "pixel to be considered fully transparent.  The default is 0.",
     &ImageFixHiddenColor::dispatch_double, nullptr, &_max_transparent_alpha);

  _min_opaque_alpha = 1.0;
  _max_transparent_alpha = 0.0;
}

/**
 *
 */
void ImageFixHiddenColor::
run() {
  PNMImage alpha_image;

  if (_alpha_filename.empty()) {
    // No separate alpha file is provided; use the base file's alpha channel.
    if (!_image.has_alpha()) {
      nout << "Image does not have an alpha channel.\n";
      exit(1);
    }
    alpha_image = _image;

  } else {
    // In this case, the alpha channel is in a separate file.
    if (!alpha_image.read(_alpha_filename)) {
      nout << "Unable to read " << _alpha_filename << ".\n";
      exit(1);
    }

    if (!alpha_image.has_alpha()) {
      // Copy the grayscale value to the alpha channel for the benefit of the
      // code below.
      alpha_image.add_alpha();
      int xi, yi;
      for (yi = 0; yi < alpha_image.get_y_size(); ++yi) {
        for (xi = 0; xi < alpha_image.get_x_size(); ++xi) {
          alpha_image.set_alpha(xi, yi, alpha_image.get_gray(xi, yi));
        }
      }
    }

    // Make sure the alpha image matches the size of the source image.
    if (alpha_image.get_x_size() != _image.get_x_size() ||
        alpha_image.get_y_size() != _image.get_y_size()) {
      PNMImage scaled(_image.get_x_size(), _image.get_y_size(), alpha_image.get_num_channels());
      scaled.quick_filter_from(alpha_image);
      alpha_image = scaled;
    }
  }

  // First, get the average color of all the opaque pixels.
  int count = 0;
  LRGBColor color(0.0, 0.0, 0.0);
  int xi, yi;
  for (yi = 0; yi < _image.get_y_size(); ++yi) {
    for (xi = 0; xi < _image.get_x_size(); ++xi) {
      if (alpha_image.get_alpha(xi, yi) >= _min_opaque_alpha) {
        color += _image.get_xel(xi, yi);
        ++count;
      }
    }
  }
  if (count == 0) {
    nout << "Image has no opaque pixels.\n";
    exit(1);
  }
  color /= (double)count;
  nout << "  average color of " << count << " opaque pixels is " << color << "\n";

  // Now, apply that wherever there are transparent pixels.
  count = 0;
  for (yi = 0; yi < _image.get_y_size(); ++yi) {
    for (xi = 0; xi < _image.get_x_size(); ++xi) {
      if (alpha_image.get_alpha(xi, yi) <= _max_transparent_alpha) {
        _image.set_xel(xi, yi, color);
        ++count;
      }
    }
  }
  if (count == 0) {
    nout << "Image has no transparent pixels.\n";
    exit(1);
  }
  nout << "  applied to " << count << " transparent pixels.\n";

  write_image(_image);
}


int main(int argc, char *argv[]) {
  ImageFixHiddenColor prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
