// Filename: fontSamples.cxx
// Created by:  drose (03Apr02)
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

#include "fontSamples.h"
#include "pnmTextMaker.h"
#include "default_font.h"
#include "pnmImage.h"
#include "notify.h"

#include <stdio.h>

#ifdef WIN32_VC
#define snprintf _snprintf
#endif

////////////////////////////////////////////////////////////////////
//     Function: FontSamples::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FontSamples::
FontSamples() {
  clear_runlines();
  add_runline("[opts] fontname [fontname ...]");

  set_program_description
    ("Generates one or more image files with one line of text from "
     "each of several given font files, as a font sample sheet.");

  add_option
    ("t", "text", 0,
     "Specifies the sample text to render using each font.",
     &FontSamples::dispatch_string, NULL, &_sample_text);

  add_option
    ("sh", "height", 0,
     "Specifies the pixel height of each sample.",
     &FontSamples::dispatch_int, NULL, &_sample_height);

  add_option
    ("nf", "filename", 0,
     "Specifies the filename of the font to be used to write each font's "
     "name.",
     &FontSamples::dispatch_filename, NULL, &_name_font_filename);

  add_option
    ("nh", "height", 0,
     "Specifies the pixel height of each font's name.",
     &FontSamples::dispatch_int, NULL, &_name_height);

  add_option
    ("fontaa", "factor", 0,
     "Specifies a scale factor to apply to the fonts  "
     "for the purpose of antialiasing the fonts a "
     "little better than FreeType can do by itself.  The letters are "
     "generated large and then scaled to their proper size.  Normally this "
     "should be a number in the range 3 to 4 for best effect.",
     &FontSamples::dispatch_double, NULL, &_font_aa_factor);

  add_option
    ("space", "y", 0,
     "Specifies the vertical space in pixels between font samples.",
     &FontSamples::dispatch_int, NULL, &_vert_space);

  add_option
    ("image", "x, y", 0,
     "Specifies the width and height in pixels of the resulting image(s).",
     &FontSamples::dispatch_int_pair, NULL, &_image_width);

  add_option
    ("o", "filename", 50, 
     "Specify the filename(s) to which the resulting image file will "
     "be written.  This should contain the string '%d' which will be "
     "filled in with the index number of the particular frame.",
     &FontSamples::dispatch_string, NULL, &_output_filename);

  _sample_text = "The quick brown fox jumped over the lazy dog's back.";
  _sample_height = 14;
  _name_height = 10;
  _font_aa_factor = 4.0;
  _vert_space = 10;
  _image_width = 600;
  _image_height = 600;
  _output_filename = "samples-%d.jpg";
  _name_text_maker = (PNMTextMaker *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: FontSamples::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
FontSamples::
~FontSamples() {
  if (_name_text_maker != (PNMTextMaker *)NULL) {
    delete _name_text_maker;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FontSamples::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool FontSamples::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the font filenames on the command line.\n";
    return false;
  }

  Args::const_iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    Filename filename = Filename::from_os_specific(*ai);
    filename.standardize();
    if (filename.exists()) {
      _font_filenames.push_back(filename);

    } else {
      nout << filename << " does not exist.\n";
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FontSamples::post_command_line
//       Access: Protected, Virtual
//  Description: This is called after the command line has been
//               completely processed, and it gives the program a
//               chance to do some last-minute processing and
//               validation of the options and arguments.  It should
//               return true if everything is fine, false if there is
//               an error.
////////////////////////////////////////////////////////////////////
bool FontSamples::
post_command_line() {
  if (_font_filenames.empty()) {
    nout << "No font filenames.\n";
    return false;
  }

  if (_name_height != 0) {
    if (!_name_font_filename.empty()) {
      _name_text_maker = new PNMTextMaker(_name_font_filename, 0);
      if (!_name_text_maker->is_valid()) {
	delete _name_text_maker;
	_name_text_maker = (PNMTextMaker *)NULL;
      }
    }
    
    if (_name_text_maker == (PNMTextMaker *)NULL) {
      _name_text_maker = new PNMTextMaker((const char *)default_font, 
                                          default_font_size, 0);
      if (!_name_text_maker->is_valid()) {
	nout << "Unable to open default font.\n";
	delete _name_text_maker;
	_name_text_maker = (PNMTextMaker *)NULL;
      }
    }
  }
  
  if (_name_text_maker != (PNMTextMaker *)NULL) {
    _name_text_maker->set_pixel_size(_name_height, _font_aa_factor);
  } else {
    _name_height = 0;
  }

  return ProgramBase::post_command_line();
}

////////////////////////////////////////////////////////////////////
//     Function: FontSamples::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void FontSamples::
run() {
  int output_index = 1;
  PNMImage output_image(_image_width, _image_height, 1);
  static const int output_filename_size = 1024;
  char output_filename[output_filename_size];

  bool all_ok = true;

  output_image.fill(1, 1, 1);
  int y = _vert_space;

  int vert_per_font = 
    _sample_height + _name_height + _vert_space;

  Filenames::iterator fi;
  for (fi = _font_filenames.begin(); fi != _font_filenames.end(); ++fi) {
    if (y > _image_height - vert_per_font) {
      // Write out the current image.
      snprintf(output_filename, output_filename_size,
	       _output_filename.c_str(), output_index);
      nout << "Writing " << output_filename << "\n";
      if (!output_image.write(output_filename)) {
	nout << "Unable to write to " << output_filename << "\n";
	exit(1);
      }
      output_index++;
      output_image.fill(1, 1, 1);
      y = _vert_space;
    }
    const Filename &filename = (*fi);
    PNMTextMaker *text_maker = new PNMTextMaker(filename, 0);
    if (!text_maker->is_valid()) {
      all_ok = false;
    } else {
      text_maker->set_pixel_size(_sample_height, _font_aa_factor);
      text_maker->generate_into(_sample_text, output_image, 
				16, y + _sample_height);
      if (_name_text_maker != (PNMTextMaker *)NULL) {
	string desc = filename.get_basename() + ": " + text_maker->get_name();
	_name_text_maker->generate_into(desc, output_image,
					16, y + _sample_height + _name_height);
      }
    }

    y += vert_per_font;
    delete text_maker;
  }

  if (!all_ok) {
    exit(1);
  }

  snprintf(output_filename, output_filename_size,
	   _output_filename.c_str(), output_index);
  nout << "Writing " << output_filename << "\n";
  if (!output_image.write(output_filename)) {
    nout << "Unable to write to " << output_filename << "\n";
    exit(1);
  }
}


int main(int argc, char *argv[]) {
  FontSamples prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
