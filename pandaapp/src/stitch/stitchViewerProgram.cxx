// Filename: stitchViewerProgram.cxx
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

#include "stitchViewerProgram.h"
#include "stitchImageConverter.h"

////////////////////////////////////////////////////////////////////
//     Function: StitchViewerProgram::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
StitchViewerProgram::
StitchViewerProgram() {
  set_program_description
    ("This program reads a stitch command file, performs whatever processing "
     "is indicated by the command file, and draws a 3-d representation of "
     "all of the input images described in the command file.  The output "
     "images are ignored.\n"

     "This program is primarily useful for showing the 3-d relationship "
     "between images that has been inferred from the stitch command file.");
}

////////////////////////////////////////////////////////////////////
//     Function: StitchViewerProgram::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void StitchViewerProgram::
run() {
  StitchImageVisualizer outputter;
  _command_file.process(outputter);
}


int main(int argc, char *argv[]) {
  StitchViewerProgram prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
