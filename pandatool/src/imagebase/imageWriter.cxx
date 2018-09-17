/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageWriter.cxx
 * @author drose
 * @date 2000-06-19
 */

#include "imageWriter.h"

/**
 * Image-writing type programs *must* specify their output file using -o.
 */
ImageWriter::
ImageWriter(bool allow_last_param) :
  WithOutputFile(allow_last_param, false, true)
{
  clear_runlines();
  if (_allow_last_param) {
    add_runline("[opts] outputimage");
  }
  add_runline("[opts] -o outputimage");

  std::string o_description;
  if (_allow_last_param) {
    o_description =
      "Specify the filename to which the resulting image file will be written.  "
      "If this option is omitted, the last parameter name is taken to be the "
      "name of the output file.";
  } else {
    o_description =
      "Specify the filename to which the resulting image file will be written.";
  }

  add_option
    ("o", "filename", 50, o_description,
     &ImageWriter::dispatch_filename, &_got_output_filename, &_output_filename);
}


/**
 * Writes the generated to the user's specified output filename.
 */
void ImageWriter::
write_image(const PNMImage &image) {
  if (!image.write(get_output_filename())) {
    nout << "Unable to write output image to "
         << get_output_filename() << "\n";
    exit(1);
  }
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool ImageWriter::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 0)) {
    return false;
  }

  if (!args.empty()) {
    nout << "Unexpected arguments on command line:\n";
    Args::const_iterator ai;
    for (ai = args.begin(); ai != args.end(); ++ai) {
      nout << (*ai) << " ";
    }
    nout << "\r";
    return false;
  }

  return true;
}
