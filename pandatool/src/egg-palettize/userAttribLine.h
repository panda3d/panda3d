// Filename: userAttribLine.h
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#ifndef USERATTRIBLINE_H
#define USERATTRIBLINE_H

#include <pandatoolbase.h>

#include <globPattern.h>

#include <vector>

class AttribFile;
class PTexture;
class SourceEgg;

////////////////////////////////////////////////////////////////////
// 	 Class : UserAttribLine
// Description : A single entry in the .txa file, this defines how the
//               user would like some particular texture to be scaled.
////////////////////////////////////////////////////////////////////

//
// This might be a line of any of the following forms:
//
//   (blank line)
//   # Comment
//   :margin msize
//   :palette xsize ysize 
//   :group groupname [dir dirname] [with groupname [groupname ...]]
//   texturename xsize ysize msize
//   texturename [texturename ...] : xsize ysize [msize] [omit]
//   texturename [texturename ...] : scale% [msize] [omit]
//   texturename [texturename ...] : [omit]
//   texturename [texturename ...] : groupname [groupname ...]
//   eggname [eggname ...] : groupname [groupname ...]
//

class UserAttribLine {
public:
  UserAttribLine(const string &line, AttribFile *af);
  ~UserAttribLine();

  bool is_valid() const;
  bool is_old_style() const;
  bool was_used() const;

  void write(ostream &out) const;

  bool get_size_request(PTexture *texture, int &margin);
  bool get_group_request(SourceEgg *egg);

private:
  enum LineType {
    LT_invalid,
    LT_comment, 
    LT_margin, LT_palette, LT_group_relate,
    LT_size, LT_scale, LT_name,
    LT_group_assign
  };

  typedef vector<GlobPattern> Patterns;
  Patterns _patterns;
  typedef vector<string> Names;
  Names _names;

  ostream &list_patterns(ostream &out) const;
  ostream &list_names(ostream &out) const;
  bool keyword_line(const string &line);
  bool texture_line(const string &line);
  bool old_style_line(const string &line);

  string _comment;
  LineType _line_type;
  int _xsize, _ysize;
  double _scale_pct;
  int _msize;
  string _dirname;
  bool _got_dirname;
  bool _omit;
  bool _is_old_style;
  bool _was_used;
  
  AttribFile *_attrib_file;
};


#endif
