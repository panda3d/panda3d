// Filename: txaFile.h
// Created by:  drose (30Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TXAFILE_H
#define TXAFILE_H

#include <pandatoolbase.h>

#include "txaLine.h"

#include <filename.h>
#include <vector_string.h>

#include <vector>

////////////////////////////////////////////////////////////////////
// 	 Class : TxaFile
// Description : This represents the .txa file (usually textures.txa)
//               that contains the user instructions for resizing,
//               grouping, etc. the various textures.
////////////////////////////////////////////////////////////////////
class TxaFile {
public:
  TxaFile();

  bool read(Filename filename);

  bool match_egg(EggFile *egg_file) const;
  bool match_texture(TextureImage *texture) const;

  void write(ostream &out) const;

private:
  bool parse_group_line(const vector_string &words);

  typedef vector<TxaLine> Lines;
  Lines _lines;
};

#endif

