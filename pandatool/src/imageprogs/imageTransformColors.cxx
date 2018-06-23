/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageTransformColors.cxx
 * @author drose
 * @date 2009-03-25
 */

#include "imageTransformColors.h"
#include "string_utils.h"
#include "pnmImage.h"
#include <math.h>

using std::max;
using std::min;
using std::string;

/**
 *
 */
ImageTransformColors::
ImageTransformColors() {
  set_program_brief("transform colors in an image file");
  set_program_description
    ("This program can apply a global color transform to all of the "
     "pixels in an image, or in a series of images.  This can be used, "
     "for instance, to increase or decrease the dynamic range; or to "
     "rotate the hue; or to reduce the saturation of colors in the image.\n\n"

     "Each parameter is encoded in a 4x4 matrix, which modifies the R, G, B "
     "colors of the image (the alpha values, if any, are not affected).  "
     "RGB values are clamped at 0 and 1 after the operation.  "
     "Multiple parameters are composed together in the order in which they "
     "are listed.");

  add_option
    ("hls", "", 0,
     "Specifies that all of the matrix operations are performed in HLS "
     "space, instead of the default RGB space.  In this mode, the first "
     "component controls hue, the second controls lightness, and the third "
     "controls saturation.",
     &ImageTransformColors::dispatch_none, &_hls, nullptr);

  add_option
    ("range", "min,max", 0,
     "Compresses the overall dynamic range from 0,1 to min,max.  If min,max "
     "exceed 0,1, the dynamic range is expanded.  This doesn't make sense in "
     "HLS mode.",
     &ImageTransformColors::dispatch_range, nullptr, &_mat);

  add_option
    ("scale", "r,g,b", 0,
     "Scales the r,g,b components by the indicated values.  In HLS mode, "
     "the scale is applied to the h,l,s components.",
     &ImageTransformColors::dispatch_scale, nullptr, &_mat);

  add_option
    ("add", "r,g,b", 0,
     "Adds the indicated values to the r,g,b components.  In HLS mode, "
     "the sum is applied to the h,l,s components.",
     &ImageTransformColors::dispatch_add, nullptr, &_mat);

  add_option
    ("mat4", "m00,m01,m02,m03,m10,m11,m12,m13,m20,m21,m22,m23,m30,m31,m32,m33",
     0, "Defines an arbitrary 4x4 RGB matrix.",
     &ImageTransformColors::dispatch_mat4, nullptr, &_mat);

  add_option
    ("mat3", "m00,m01,m02,m10,m11,m12,m20,m21,m22", 0,
     "Defines an arbitrary 3x3 RGB matrix.",
     &ImageTransformColors::dispatch_mat3, nullptr, &_mat);

  add_option
    ("o", "filename", 50,
     "Specify the filename to which the resulting image file will be written.  "
     "This is only valid when there is only one input image file on the command "
     "line.  If you want to process multiple files simultaneously, you must "
     "use either -d or -inplace.",
     &ImageTransformColors::dispatch_filename, &_got_output_filename, &_output_filename);

  add_option
    ("d", "dirname", 50,
     "Specify the name of the directory in which to write the resulting image "
     "files.  If you are processing only one image file, this may be omitted "
     "in lieu of the -o option.  If you are processing multiple image files, "
     "this may be omitted only if you specify -inplace instead.",
     &ImageTransformColors::dispatch_filename, &_got_output_dirname, &_output_dirname);

  add_option
    ("inplace", "", 50,
     "If this option is given, the input image files will be rewritten in "
     "place with the results.  This obviates the need to specify -d "
     "for an output directory; however, it's risky because the original "
     "input image files are lost.",
     &ImageTransformColors::dispatch_none, &_inplace);

  _mat = LMatrix4d::ident_mat();
}

/**
 *
 */
void ImageTransformColors::
run() {
  _mat.write(nout, 0);
  nout << "\n";

  Filenames::iterator fi;
  for (fi = _filenames.begin(); fi != _filenames.end(); ++fi) {
    const Filename &source_filename = (*fi);
    nout << source_filename << "\n";
    PNMImage image;
    if (!image.read(source_filename)) {
      nout << "Couldn't read " << source_filename << "; ignoring.\n";
      continue;
    }

    process_image(image);

    Filename output_filename = get_output_filename(source_filename);
    if (!image.write(output_filename)) {
      nout << "Couldn't write " << output_filename << "; ignoring.\n";
    }
  }
}

/**
 * Takes a series of 16 numbers as a 4x4 matrix.
 */
bool ImageTransformColors::
dispatch_mat4(const string &opt, const string &arg, void *var) {
  LMatrix4d &orig = *(LMatrix4d *)var;

  vector_string words;
  tokenize(arg, words, ",");

  LMatrix4d mat;
  bool okflag = false;
  if (words.size() == 16) {
    okflag =
      string_to_double(words[0], mat[0][0]) &&
      string_to_double(words[1], mat[0][1]) &&
      string_to_double(words[2], mat[0][2]) &&
      string_to_double(words[3], mat[0][3]) &&
      string_to_double(words[4], mat[1][0]) &&
      string_to_double(words[5], mat[1][1]) &&
      string_to_double(words[6], mat[1][2]) &&
      string_to_double(words[7], mat[1][3]) &&
      string_to_double(words[8], mat[2][0]) &&
      string_to_double(words[9], mat[2][1]) &&
      string_to_double(words[10], mat[2][2]) &&
      string_to_double(words[11], mat[2][3]) &&
      string_to_double(words[12], mat[3][0]) &&
      string_to_double(words[13], mat[3][1]) &&
      string_to_double(words[14], mat[3][2]) &&
      string_to_double(words[15], mat[3][3]);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires sixteen numbers separated by commas.\n";
    return false;
  }

  orig *= mat;

  return true;
}

/**
 * Takes a series of 9 numbers as a 3x3 matrix.
 */
bool ImageTransformColors::
dispatch_mat3(const string &opt, const string &arg, void *var) {
  LMatrix4d &orig = *(LMatrix4d *)var;

  vector_string words;
  tokenize(arg, words, ",");

  LMatrix3d mat;
  bool okflag = false;
  if (words.size() == 9) {
    okflag =
      string_to_double(words[0], mat[0][0]) &&
      string_to_double(words[1], mat[0][1]) &&
      string_to_double(words[2], mat[0][2]) &&
      string_to_double(words[3], mat[1][0]) &&
      string_to_double(words[4], mat[1][1]) &&
      string_to_double(words[5], mat[1][2]) &&
      string_to_double(words[6], mat[2][0]) &&
      string_to_double(words[7], mat[2][1]) &&
      string_to_double(words[8], mat[2][2]);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires nine numbers separated by commas.\n";
    return false;
  }

  orig *= LMatrix4d(mat);

  return true;
}

/**
 * Takes a min,max dynamic range.
 */
bool ImageTransformColors::
dispatch_range(const string &opt, const string &arg, void *var) {
  LMatrix4d &orig = *(LMatrix4d *)var;

  vector_string words;
  tokenize(arg, words, ",");

  double min, max;
  bool okflag = false;
  if (words.size() == 2) {
    okflag =
      string_to_double(words[0], min) &&
      string_to_double(words[1], max);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires two numbers separated by commas.\n";
    return false;
  }

  orig *= LMatrix4d::scale_mat(max - min) * LMatrix4d::translate_mat(min);

  return true;
}

/**
 * Accepts a componentwise scale.
 */
bool ImageTransformColors::
dispatch_scale(const string &opt, const string &arg, void *var) {
  LMatrix4d &orig = *(LMatrix4d *)var;

  vector_string words;
  tokenize(arg, words, ",");

  double r, g, b;
  bool okflag = false;
  if (words.size() == 3) {
    okflag =
      string_to_double(words[0], r) &&
      string_to_double(words[1], g) &&
      string_to_double(words[2], b);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires three numbers separated by commas.\n";
    return false;
  }

  orig *= LMatrix4d::scale_mat(r, g, b);

  return true;
}

/**
 * Accepts a componentwise add.
 */
bool ImageTransformColors::
dispatch_add(const string &opt, const string &arg, void *var) {
  LMatrix4d &orig = *(LMatrix4d *)var;

  vector_string words;
  tokenize(arg, words, ",");

  double r, g, b;
  bool okflag = false;
  if (words.size() == 3) {
    okflag =
      string_to_double(words[0], r) &&
      string_to_double(words[1], g) &&
      string_to_double(words[2], b);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires three numbers separated by commas.\n";
    return false;
  }

  orig *= LMatrix4d::translate_mat(r, g, b);

  return true;
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool ImageTransformColors::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the image file(s) to read on the command line.\n";
    return false;

  } else {
    // These only apply if we have specified any image files.
    if (_got_output_filename && args.size() == 1) {
      if (_got_output_dirname) {
        nout << "Cannot specify both -o and -d.\n";
        return false;
      } else if (_inplace) {
        nout << "Cannot specify both -o and -inplace.\n";
        return false;
      }

    } else {
      if (_got_output_filename) {
        nout << "Cannot use -o when multiple image files are specified.\n";
        return false;
      }

      if (_got_output_dirname && _inplace) {
        nout << "Cannot specify both -inplace and -d.\n";
        return false;

      } else if (!_got_output_dirname && !_inplace) {
        nout << "You must specify either -inplace or -d.\n";
        return false;
      }
    }
  }

  Args::const_iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    Filename filename = (*ai);
    if (!filename.exists()) {
      nout << "Image file not found: " << filename << "\n";
      return false;
    }
    _filenames.push_back(filename);
  }

  return true;
}

/**
 * Returns the output filename of the egg file with the given input filename.
 * This is based on the user's choice of -inplace, -o, or -d.
 */
Filename ImageTransformColors::
get_output_filename(const Filename &source_filename) const {
  if (_got_output_filename) {
    nassertr(!_inplace && !_got_output_dirname && _filenames.size() == 1, Filename());
    return _output_filename;

  } else if (_got_output_dirname) {
    nassertr(!_inplace, Filename());
    Filename result = source_filename;
    result.set_dirname(_output_dirname);
    return result;
  }

  nassertr(_inplace, Filename());
  return source_filename;
}

inline double
hue2rgb(double m1, double m2, double h) {
  h -= floor(h);
  if (h < 1.0/6.0) {
    return m1 + (m2 - m1) * h * 6.0;
  }
  if (h < 1.0/2.0) {
    return m2;
  }
  if (h < 2.0/3.0) {
    return m1 + (m2 - m1) * (2.0/3.0 - h) * 6;
  }
  return m1;
}

static LRGBColord
hls2rgb(const LRGBColord &hls) {
  double h = hls[0];
  double l = max(min(hls[1], 1.0), 0.0);
  double s = max(min(hls[2], 1.0), 0.0);

  double m2;
  if (l <= 0.5) {
    m2 = l * (s + 1.0);
  } else {
    m2 = l + s - l * s;
  }
  double m1 = l * 2 - m2;

  LRGBColord rgb(hue2rgb(m1, m2, h + 1.0/3.0),
                hue2rgb(m1, m2, h),
                hue2rgb(m1, m2, h - 1.0/3.0));
  return rgb;
}

static LRGBColord
rgb2hls(const LRGBColord &rgb) {
  double r = rgb[0];
  double g = rgb[1];
  double b = rgb[2];
  double h, l, s;

  double minval = min(min(r, g), b);
  double maxval = max(max(r, g), b);

  double rnorm = 0.0, gnorm = 0.0, bnorm = 0.0;
  double mdiff = maxval - minval;
  double msum  = maxval + minval;
  l = 0.5 * msum;
  if (maxval == minval) {
    // Grayscale.
    return LRGBColord(0.0, l, 0.0);
  }

  rnorm = (maxval - r) / mdiff;
  gnorm = (maxval - g) / mdiff;
  bnorm = (maxval - b) / mdiff;

  if (l < 0.5) {
    s = mdiff / msum;
  } else {
    s = mdiff / (2.0 - msum);
  }

  if (r == maxval) {
    h = (6.0 + bnorm - gnorm) / 6.0;
  } else if (g == maxval) {
    h = (2.0 + rnorm - bnorm) / 6.0;
  } else {
    h = (4.0 + gnorm - rnorm) / 6.0;
  }

  if (h > 1.0) {
    h -= 1.0;
  }

  return LRGBColord(h, l, s);
}

/**
 * Processes a single image in-place.
 */
void ImageTransformColors::
process_image(PNMImage &image) {
  if (_hls) {
    for (int yi = 0; yi < image.get_y_size(); ++yi) {
      for (int xi = 0; xi < image.get_x_size(); ++xi) {
        LRGBColord rgb = LCAST(double, image.get_xel(xi, yi));
        rgb = hls2rgb(_mat.xform_point(rgb2hls(rgb)));
        image.set_xel(xi, yi, LCAST(float, rgb));
      }
    }
  } else {
    for (int yi = 0; yi < image.get_y_size(); ++yi) {
      for (int xi = 0; xi < image.get_x_size(); ++xi) {
        LRGBColord rgb = LCAST(double, image.get_xel(xi, yi));
        rgb = _mat.xform_point(rgb);
        image.set_xel(xi, yi, LCAST(float, rgb));
      }
    }
  }
}

int main(int argc, char *argv[]) {
  ImageTransformColors prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
