// Filename: stitchCommandReader.cxx
// Created by:  drose (16Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "stitchCommandReader.h"

////////////////////////////////////////////////////////////////////
//     Function: StitchCommandReader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
StitchCommandReader::
StitchCommandReader() {
  clear_runlines();
  add_runline("[opts] input.st");
}


////////////////////////////////////////////////////////////////////
//     Function: StitchCommandReader::handle_args
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool StitchCommandReader::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the stitch command file to read on the\n"
         << "command line.\n";
    return false;
  }
  if (args.size() > 1) {
    nout << "You must specify only one stitch command file to read on the\n"
         << "command line.\n";
    return false;
  }

  return _command_file.read(args[0]);
}
