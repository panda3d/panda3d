// Filename: imageFilter.cxx
// Created by:  drose (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "imageFilter.h"

////////////////////////////////////////////////////////////////////
//     Function: ImageFilter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ImageFilter::
ImageFilter() {
  clear_runlines();
  add_runline("[opts] inputimage outputimage");
  add_runline("[opts] -o outputimage inputimage");
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFilter::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool ImageFilter::
handle_args(ProgramBase::Args &args) {
  if (!_got_output_filename) {
    if (args.size() != 2) {
      nout << "You must specify the input and output filenames on the "
           << "command line, or use -o to specify the output filename.\n";
      return false;
    }

    if (!_image.read(args[0])) {
      nout << "Unable to read image file.\n";
      return false;
    }

    _output_filename = args[1];
    _got_output_filename = true;
    return true;
  }
   
  return ImageReader::handle_args(args);
}
