// Filename: stitchImageProgram.cxx
// Created by:  drose (16Mar00)
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
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageProgram::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void StitchImageProgram::
run() {
  StitchImageRasterizer outputter;
  _command_file.process(outputter);
}


int main(int argc, char *argv[]) {
  StitchImageProgram prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
