/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageReader.cxx
 * @author drose
 * @date 2000-06-19
 */

#include "imageReader.h"

/**
 *
 */
ImageReader::
ImageReader() {
  clear_runlines();
  add_runline("[opts] imagename");
}

/**
 *
 */
bool ImageReader::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the image file to read on the command line.\n";
    return false;
  }

  if (args.size() > 1) {
    nout << "Specify only one image on the command line.\n";
    return false;
  }

  if (!_image.read(args[0])) {
    nout << "Unable to read image file " << args[0] << ".\n";
    exit(1);
  }

  return true;
}
