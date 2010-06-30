// Filename: indexify.h
// Created by:  drose (07Apr02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef FONTSAMPLES_H
#define FONTSAMPLES_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "filename.h"
#include "pvector.h"

class PNMTextMaker;

////////////////////////////////////////////////////////////////////
//       Class : FontSamples
// Description : A program to generate one or more image files showing
//               samples of various fonts.
////////////////////////////////////////////////////////////////////
class FontSamples : public ProgramBase {
public:
  FontSamples();
  virtual ~FontSamples();

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();

public:
  void run();

  string _sample_text;
  int _sample_height;
  int _name_height;
  Filename _name_font_filename;
  double _font_aa_factor;
  int _vert_space;
  int _image_width;
  int _image_height;
  string _output_filename;

  typedef pvector<Filename> Filenames;
  Filenames _font_filenames;

  PNMTextMaker *_name_text_maker;
};

#endif
