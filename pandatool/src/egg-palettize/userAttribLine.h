// Filename: userAttribLine.h
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#ifndef USERATTRIBLINE_H
#define USERATTRIBLINE_H

#include <pandatoolbase.h>

#include <vector>

class AttribFile;
class Texture;

////////////////////////////////////////////////////////////////////
// 	 Class : UserAttribLine
// Description : A single entry in the user part (the beginning) of
//               the attrib file, this defines how the user would like
//               some particular texture to be scaled.
////////////////////////////////////////////////////////////////////

//
// This might be a line of any of the following forms:
//
//   (blank line)
//   # Comment
//   :margin msize
//   :palette xsize ysize 
//   texturename xsize ysize msize
//   texturename [texturename ...] : xsize ysize [msize] [omit]
//   texturename [texturename ...] : scale% [msize] [omit]
//   texturename [texturename ...] : [omit]
//

class UserAttribLine {
public:
  UserAttribLine(const string &line, AttribFile *af);
  ~UserAttribLine();

  bool is_valid() const;
  bool is_old_style() const;
  bool was_used() const;

  void write(ostream &out) const;

  bool match_texture(Texture *texture, int &margin);

private:
  enum LineType {
    LT_invalid,
    LT_comment, 
    LT_margin, LT_palette,
    LT_size, LT_scale, LT_name
  };
  class TextureName {
  public:
    TextureName(const string &pattern);
    TextureName(const TextureName &copy) : 
      _pattern(copy._pattern) { }

    string _pattern;
  };

  typedef vector<TextureName> TextureNames;
  TextureNames _texture_names;

  ostream &list_textures(ostream &out) const;
  bool keyword_line(const string &line);
  bool texture_line(const string &line);
  bool old_style_line(const string &line);

  string _comment;
  LineType _line_type;
  int _xsize, _ysize;
  double _scale_pct;
  int _msize;
  bool _omit;
  bool _is_old_style;
  bool _was_used;
  
  AttribFile *_attrib_file;
};


#endif
