// Filename: stitchImageProgram.cxx
// Created by:  drose (16Mar00)
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

  add_option
    ("o", "name", 0,
     "Generates only the named output image.  This may be repeated to "
     "generate multiple images in one run.  If omitted, all output images "
     "in the file are generated.  The name may include filename globbing "
     "symbols, e.g. 'grid*'.  You should quote such names to protect them "
     "from shell expansion.",
     &StitchImageProgram::dispatch_output_name);

  add_option
    ("i", "name", 0,
     "Generates only the named input image or images, as above.",
     &StitchImageProgram::dispatch_input_name);

  add_option
    ("s", "xsize,ysize", 0,
     "Generates the output image(s) at the specified size, rather than the "
     "size specified within the .st file.",
     &StitchImageProgram::dispatch_int_pair, &_got_output_size, &_output_size);

  _filter_factor = 1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageProgram::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void StitchImageProgram::
run() {
  if (_got_output_size) {
    _outputter.set_output_size(_output_size[0], _output_size[1]);
  }
  _outputter.set_filter_factor(_filter_factor);
  _command_file.process(_outputter);
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageProgram::dispatch_output_name
//       Access: Protected, Static
//  Description: Dispatch function for an output image name.
////////////////////////////////////////////////////////////////////
bool StitchImageProgram::
dispatch_output_name(ProgramBase *self, const string &opt,
                     const string &arg, void *) {
  StitchImageProgram *prog = (StitchImageProgram *)self;
  prog->_outputter.add_output_name(arg);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageProgram::dispatch_input_name
//       Access: Protected, Static
//  Description: Dispatch function for an input image name.
////////////////////////////////////////////////////////////////////
bool StitchImageProgram::
dispatch_input_name(ProgramBase *self, const string &opt,
                    const string &arg, void *) {
  StitchImageProgram *prog = (StitchImageProgram *)self;
  prog->_outputter.add_input_name(arg);
  return true;
}


int main(int argc, char *argv[]) {
  StitchImageProgram prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
