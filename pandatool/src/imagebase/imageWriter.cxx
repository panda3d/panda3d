// Filename: imageWriter.cxx
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
  add_runline("[opts] -s newimagesize");
  add_runline("[opts] -hq");
  add_runline("[opts] -filter_radius filter-radius");

  add_option
    ("o", "filename", 50,
     "Specify the filename to which the resulting image file will be written.  "
     "If this is omitted, the last parameter on the command line is taken as "
     "the output filename.",
     &ImageWriter::dispatch_filename, &_got_output_filename, &_output_filename);

  add_option
    ("s", "new size", 50,
     "Specify the width & height of the output image using the next 2 integers.  "
     "Ex: '-s 200,100' resizes to 200x100",
     &ImageWriter::dispatch_int_pair, &_bDoResize, &_newsize);

  add_option
    ("hq", "", 50,
     "Use High-Quality filtering to do image-resizing",
     &ImageWriter::dispatch_none, &_bUseHighQualityFiltering);

  add_option
    ("filter_radius", "filter-radius", 50,
     "float value specifying a filter radius to use for the High-Quality filter (ex: 1.0)",
     &ImageWriter::dispatch_double, NULL, &_filter_radius);

  _filter_radius = 1.0;
  _bDoResize = false;
}


////////////////////////////////////////////////////////////////////
//     Function: ImageWriter::write_image
//       Access: Public
//  Description: Writes the generated to the user's specified output
//               filename.
////////////////////////////////////////////////////////////////////
void ImageWriter::
write_image(const PNMImage &image) {
  if(_bDoResize) {
      PNMImage resized_image(_newsize[0], _newsize[1], image.get_num_channels(), image.get_maxval());
      if(_bUseHighQualityFiltering) {
          nout << "HQ filtering using filter-radius: " << _filter_radius << endl;
          resized_image.gaussian_filter_from(_filter_radius, image);
      } else {
          resized_image.quick_filter_from(image);
      }

      if (!resized_image.write(_output_filename)) {
          nout << "Unable to write output image to " << _output_filename << "\n";
          exit(1);
      }
      return;
  }

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
      Args::const_iterator ai;
      for (ai = args.begin(); ai != args.end(); ++ai) {
	nout << (*ai) << " ";
      }
      nout << "\r";
      return false;
    }
  }

  return true;
}

