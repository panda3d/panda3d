// Filename: indexify.h
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

  string _front_title;
  Filename _roll_dir_root;
  string _photo_extension;
  string _movie_extension;
  Filename _font_filename;
  bool _generate_icons;
  double _font_aa_factor;

  typedef pvector<RollDirectory *> RollDirs;
  RollDirs _roll_dirs;

  PNMTextMaker *_text_maker;
};

#endif
