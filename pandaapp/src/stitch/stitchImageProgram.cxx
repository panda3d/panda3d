// Filename: stitchImageProgram.cxx
// Created by:  drose (16Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "stitchImageProgram.h"
#include "stitchImageRasterizer.h"

////////////////////////////////////////////////////////////////////
//     Function: StitchImageProgram::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
StitchImageProgram::
StitchImageProgram() {
  set_program_description
    ("This program reads a stitch command file, performs whatever processing "
     "is indicated by the command file, and generates an output image for "
     "each image listed in an output_image section.\n"

     "The images are generated internally using a CPU-based rasterization "
     "algorithm (no graphics hardware is used).");

  add_option
    ("f", "factor", 0,
     "Scale the output images internally by the indicated factor in each "
     "dimension while generating them, and then reduce them to their final "
     "size on output.  This provides a simple mechanism for filtering "
     "the result.  The default is 1.0, or unfiltered, which runs relatively "
     "quickly but can give highly aliased results; specifying a larger number "
     "increases quality but also increases runtime and memory requirements "
     "roughly by the square of factor.  Usually 2 or 3 provide satisfactory "
     "results.",
     &StitchImageProgram::dispatch_double, NULL, &_filter_factor);

  _filter_factor = 1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageProgram::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void StitchImageProgram::
run() {
  StitchImageRasterizer outputter;
  outputter._filter_factor = _filter_factor;
  _command_file.process(outputter);
}


int main(int argc, char *argv[]) {
  StitchImageProgram prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
