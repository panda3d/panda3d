// Filename: stitchCommandProgram.cxx
// Created by:  drose (16Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "stitchCommandProgram.h"
#include "stitchImageCommandOutput.h"

////////////////////////////////////////////////////////////////////
//     Function: StitchCommandProgram::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
StitchCommandProgram::
StitchCommandProgram() {
  set_program_description
    ("This program reads a stitch command file, performs processing on the "
     "file (such as alignment of images according to points marked within a "
     "stitch region), and writes the resulting command file to standard "
     "output.  It does not actually operate on any images.\n"

     "The primary function of this program is to test the syntax of a "
     "command file, or to preprocess a command file so that a series of "
     "images (for instance, frames of a movie) may be easily transformed "
     "by the exact same operation.");
}

////////////////////////////////////////////////////////////////////
//     Function: StitchCommandProgram::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void StitchCommandProgram::
run() {
  StitchImageCommandOutput outputter;
  _command_file.process(outputter);
}


int main(int argc, char *argv[]) {
  StitchCommandProgram prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
