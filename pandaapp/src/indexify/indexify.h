// Filename: indexify.h
// Created by:  drose (03Apr02)
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

#ifndef INDEXIFY_H
#define INDEXIFY_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "filename.h"
#include "pvector.h"

class RollDirectory;
class PNMTextMaker;

////////////////////////////////////////////////////////////////////
//       Class : Indexify
// Description : A program to generate a series of thumbnail images
//               and HTML pages to view a number of image files in an
//               archive directory, before writing the archive to a
//               permanent medium like a CD.
////////////////////////////////////////////////////////////////////
class Indexify : public ProgramBase {
public:
  Indexify();
  virtual ~Indexify();

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();

  static bool dispatch_caption(const string &opt, const string &arg, void *var);

public:
  void run();
  void do_generate_images();
  void do_copy_reduced();

  string _front_title;
  Filename _roll_dir_root;
  string _photo_extension;
  string _movie_extension;
  string _sound_extension;
  Filename _font_filename;
  bool _generate_icons;
  double _font_aa_factor;
  bool _copy_reduced;

  typedef pvector<RollDirectory *> RollDirs;
  RollDirs _roll_dirs;

  PNMTextMaker *_text_maker;
};

#endif
