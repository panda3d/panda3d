// Filename: txaFile.h
// Created by:  drose (30Nov00)
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
  bool parse_powertwo_line(const vector_string &words);
  bool parse_imagetype_line(const vector_string &words);
  bool parse_shadowtype_line(const vector_string &words);
  bool parse_round_line(const vector_string &words);
  bool parse_remap_line(const vector_string &words);
  bool parse_cutout_line(const vector_string &words);
  bool parse_textureswap_line(const vector_string &words);

  typedef pvector<TxaLine> Lines;
  Lines _lines;
};

#endif

