/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageInfo.cxx
 * @author drose
 * @date 2003-03-13
 */

#include "imageInfo.h"
#include "pnmImageHeader.h"

/**
 *
 */
ImageInfo::
ImageInfo() {
  set_program_brief("report the size of image files");
  set_program_description
    ("This program reads the headers of a series of one or more "
     "image files and reports the image sizes to standard output.");

  add_option
    ("2", "", 0,
     "Report only images that have a non-power-of-two size in either "
     "dimension.  Images whose dimensions are both a power of two will "
     "not be mentioned.",
     &ImageInfo::dispatch_none, &_report_power_2, nullptr);
}

/**
 *
 */
void ImageInfo::
run() {
  Args::const_iterator ai;
  for (ai = _filenames.begin(); ai != _filenames.end(); ++ai) {
    Filename filename = (*ai);
    PNMImageHeader header;
    if (!header.read_header(filename)) {
      // Could not read the image header.
      if (filename.exists()) {
        nout << filename << ": could not read image.\n";
      } else {
        nout << filename << ": does not exist.\n";
      }
    } else {
      // Successfully read the image header.
      if (!_report_power_2 ||
          !is_power_2(header.get_x_size()) ||
          !is_power_2(header.get_y_size())) {
        nout << filename << ": " << header.get_x_size() << " x "
             << header.get_y_size() << " x " << header.get_num_channels()
             << " (maxval = " << header.get_maxval() << ")\n";
      }
    }
  }
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool ImageInfo::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "List one or more image filenames on command line.\n";
    return false;
  }
  _filenames = args;

  return true;
}

/**
 * Returns true if the indicated value is a power of 2, false otherwise.
 */
bool ImageInfo::
is_power_2(int value) const {
  return (value & (value - 1)) == 0;
}


int main(int argc, char *argv[]) {
  ImageInfo prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
