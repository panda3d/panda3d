/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageFilter.cxx
 * @author drose
 * @date 2000-06-19
 */

#include "imageFilter.h"

/**
 *
 */
ImageFilter::
ImageFilter(bool allow_last_param) :
  ImageWriter(allow_last_param)
{
  clear_runlines();
  if (_allow_last_param) {
    add_runline("[opts] inputimage outputimage");
  }
  add_runline("[opts] -o outputimage inputimage");
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool ImageFilter::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 1)) {
    return false;
  }

  return ImageReader::handle_args(args);
}
