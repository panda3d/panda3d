// Filename: txaLine.h
// Created by:  drose (30Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TXALINE_H
#define TXALINE_H

#include <pandatoolbase.h>

#include "paletteGroups.h"

#include <globPattern.h>
#include <eggTexture.h>

#include <vector>

class PNMFileType;
class EggFile;
class TextureImage;

////////////////////////////////////////////////////////////////////
//       Class : TxaLine
// Description : This is a single matching line in the .txa file.  It
//               consists of a list of names (texture names or egg
//               file names), followed by a colon and an optional size
//               and a set of keywords.
////////////////////////////////////////////////////////////////////
class TxaLine {
public:
  TxaLine();

  bool parse(const string &line);

  bool match_egg(EggFile *egg_file) const;
  bool match_texture(TextureImage *texture) const;

  void output(ostream &out) const;

private:
  typedef vector<GlobPattern> Patterns;
  Patterns _texture_patterns;
  Patterns _egg_patterns;

  enum SizeType {
    ST_none,
    ST_scale,
    ST_explicit_2,
    ST_explicit_3
  };

  SizeType _size_type;
  float _scale;
  int _x_size;
  int _y_size;
  int _num_channels;
  EggTexture::Format _format;

  bool _got_margin;
  int _margin;
  bool _got_coverage_threshold;
  double _coverage_threshold;

  enum Keyword {
    KW_omit,
    KW_nearest,
    KW_linear,
    KW_mipmap,
    KW_cont
  };

  typedef vector<Keyword> Keywords;
  Keywords _keywords;

  PaletteGroups _palette_groups;

  PNMFileType *_color_type;
  PNMFileType *_alpha_type;
};

INLINE ostream &operator << (ostream &out, const TxaLine &line) {
  line.output(out);
  return out;
}

#endif

