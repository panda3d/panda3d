// Filename: txaFile.h
// Created by:  drose (30Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef TXAFILE_H
#define TXAFILE_H

#include "pandatoolbase.h"

#include "txaLine.h"

#include "filename.h"
#include "vector_string.h"

#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : TxaFile
// Description : This represents the .txa file (usually textures.txa)
//               that contains the user instructions for resizing,
//               grouping, etc. the various textures.
////////////////////////////////////////////////////////////////////
class TxaFile {
public:
  TxaFile();

  bool read(istream &in, const string &filename);

  bool match_egg(EggFile *egg_file) const;
  bool match_texture(TextureImage *texture) const;

  void write(ostream &out) const;

private:
  static int get_line_or_semicolon(istream &in, string &line);

  bool parse_group_line(const vector_string &words);
  bool parse_palette_line(const vector_string &words);
  bool parse_margin_line(const vector_string &words);
  bool parse_background_line(const vector_string &words);
  bool parse_coverage_line(const vector_string &words);
  bool parse_imagetype_line(const vector_string &words);
  bool parse_shadowtype_line(const vector_string &words);
  bool parse_round_line(const vector_string &words);
  bool parse_remap_line(const vector_string &words);
  bool parse_remapchar_line(const vector_string &words);

  typedef pvector<TxaLine> Lines;
  Lines _lines;
};

#endif

