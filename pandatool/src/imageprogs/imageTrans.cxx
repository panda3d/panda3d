// Filename: imageTrans.cxx
// Created by:  drose (19Jun00)
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

#include "imageTrans.h"

////////////////////////////////////////////////////////////////////
//     Function: ImageTrans::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ImageTrans::
ImageTrans() {
  set_program_description
    ("This program reads an image file and writes an essentially equivalent "
     "image file to the file specified with -o.");
}

////////////////////////////////////////////////////////////////////
//     Function: ImageTrans::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ImageTrans::
run() {
  write_image();
}


int main(int argc, char *argv[]) {
  ImageTrans prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
