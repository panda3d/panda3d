// Filename: imageReader.cxx
// Created by:  drose (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "imageReader.h"

////////////////////////////////////////////////////////////////////
//     Function: ImageReader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ImageReader::
ImageReader() {
  clear_runlines();
  add_runline("[opts] imagename");
}

////////////////////////////////////////////////////////////////////
//     Function: ImageReader::handle_args
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool ImageReader::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the image file to read on the command line.\n";
    return false;
  }

  if (args.size() > 1) {
    nout << "Specify only one image on the command line.\n";
    return false;
  }

  if (!_image.read(args[0])) {
    nout << "Unable to read image file.\n";
    return false;
  }

  return true;
}
