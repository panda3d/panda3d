// Filename: imageTrans.cxx
// Created by:  drose (19Jun00)
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
