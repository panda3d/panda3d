// Filename: imageInfo.cxx
// Created by:  drose (13Mar03)
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

#include "imageInfo.h"
#include "pnmImageHeader.h"

////////////////////////////////////////////////////////////////////
//     Function: ImageInfo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ImageInfo::
ImageInfo() {
  set_program_description
    ("This program reads the headers of a series of one or more "
     "image files and reports the image sizes to standard output.");
}

////////////////////////////////////////////////////////////////////
//     Function: ImageInfo::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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
      nout << filename << ": " << header.get_x_size() << " x "
           << header.get_y_size() << " x " << header.get_num_channels()
           << " (maxval = " << header.get_maxval() << ")\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ImageInfo::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool ImageInfo::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "List one or more image filenames on command line.\n";
    return false;
  }
  _filenames = args;

  return true;
}


int main(int argc, char *argv[]) {
  ImageInfo prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
