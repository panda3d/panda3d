// Filename: stitchViewerProgram.cxx
// Created by:  drose (16Mar00)
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
