// Filename: imageWriter.cxx
// Created by:  drose (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "imageWriter.h"

////////////////////////////////////////////////////////////////////
//     Function: ImageWriter::Constructor
//       Access: Public
//  Description: Image-writing type programs *must* specify their
//               output file using -o.
////////////////////////////////////////////////////////////////////
ImageWriter::
ImageWriter() {
  clear_runlines();
  add_runline("[opts] outputimage");
  add_runline("[opts] -o outputimage");

  add_option
    ("o", "filename", 50, 
     "Specify the filename to which the resulting image file will be written.  "
     "If this is omitted, the last parameter on the command line is taken as "
     "the output filename.",
     &ImageWriter::dispatch_filename, &_got_output_filename, &_output_filename);
}


////////////////////////////////////////////////////////////////////
//     Function: ImageWriter::write_image
//       Access: Public
//  Description: Writes the generated to the user's specified output
//               filename.
////////////////////////////////////////////////////////////////////
void ImageWriter::
write_image(const PNMImage &image) {
  if (!image.write(_output_filename)) {
    nout << "Unable to write output image to " << _output_filename << "\n";
    exit(1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ImageWriter::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool ImageWriter::
handle_args(ProgramBase::Args &args) {
  if (!_got_output_filename) {
    if (args.size() != 1) {
      nout << "You must specify the filename to write with -o, or as "
           << "the last parameter on the command line.\n";
      return false;
    }
    _output_filename = args[0];
    _got_output_filename = true;

  } else {
    if (!args.empty()) {
      nout << "Unexpected arguments on command line:\n";
      copy(args.begin(), args.end(), ostream_iterator<string>(nout, " "));
      nout << "\r";
      return false;
    }
  }

  return true;
}

