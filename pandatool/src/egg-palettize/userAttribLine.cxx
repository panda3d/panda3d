// Filename: userAttribLine.cxx
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#include "userAttribLine.h"
#include "string_utils.h"
#include "texture.h"
#include "attribFile.h"

#include <notify.h>

#include <ctype.h>
#include <fnmatch.h>

UserAttribLine::TextureName::
TextureName(const string &pattern) : _pattern(pattern) {
}

UserAttribLine::
UserAttribLine(const string &cline, AttribFile *af) : _attrib_file(af) {
  _is_old_style = false;

  // By default, all lines are marked 'was_used'.  That means they'll
  // be preserved across a -k on the command line.  Lines that name
  // textures will have _was_used set false until a texture actually
  // matches that line.
  _was_used = true;

  string line = cline;

  // First, strip off the comment.
  if (!line.empty()) {
    if (line[0] == '#') {
      _comment = line;
      line = "";
    } else {
      size_t pos = line.find(" #");
      if (pos != string::npos) {
	while (pos > 0 && isspace(line[pos])) {
	  pos--;
	}
	_comment = line.substr(pos + 1);
	line = line.substr(0, pos);
      }
    }
  }

  // Now, analyze the line.
  _line_type = LT_invalid;
  _xsize = 0;
  _ysize = 0;
  _scale_pct = 0.0;
  _msize = -1;
  _omit = false;

  bool is_valid = true;
  if (line.empty()) {
    _line_type = LT_comment;

  } else if (line[0] == ':') {
    is_valid = keyword_line(line);

  } else {
    is_valid = texture_line(line);
  }

  if (!is_valid) {
    _line_type = LT_invalid;
  }
}

UserAttribLine::
~UserAttribLine() {
}

bool UserAttribLine::
is_valid() const {
  return _line_type != LT_invalid;
}

bool UserAttribLine::
is_old_style() const {
  return _is_old_style;
}

bool UserAttribLine::
was_used() const {
  return _was_used;
}

void UserAttribLine::
write(ostream &out) const {
  switch (_line_type) {
  case LT_invalid:
    out << "*** invalid line ***\n";
    break;

  case LT_comment:
    break;

  case LT_margin:
    out << ":margin " << _msize;
    break;

  case LT_palette:
    out << ":palette " << _xsize << " " << _ysize;
    break;

  case LT_size:
    list_textures(out) << " : " << _xsize << " " << _ysize;
    if (_msize > 0) {
      out << " " << _msize;
    }
    if (_omit) {
      out << " omit";
    }
    break;

  case LT_scale:
    list_textures(out) << " : " << _scale_pct << "%";
    if (_msize > 0) {
      out << " " << _msize;
    }
    if (_omit) {
      out << " omit";
    }
    break;

  case LT_name:
    list_textures(out) << " :";
    if (_omit) {
      out << " omit";
    }
    break;

  default:
    nout << "Unexpected type: " << (int)_line_type << "\n";
    abort();
  };

  out << _comment << "\n";
}

bool UserAttribLine::
match_texture(Texture *texture, int &margin) {
  // See if the texture name matches any of the filename patterns on
  // this line.
  bool matched_any = false;
  TextureNames::const_iterator tni;
  for (tni = _texture_names.begin();
       tni != _texture_names.end() && !matched_any;
       ++tni) {
    if (fnmatch((*tni)._pattern.c_str(), texture->get_name().c_str(), 0) == 0) {
      matched_any = true;
    }
  }

  if (matched_any) {
    _was_used = true;

    // It does!  So do the right thing with this line.
    switch (_line_type) {
    case LT_invalid:
    case LT_comment:
    case LT_palette:
      return false;
      
    case LT_margin:
      margin = _msize;
      return false;
      
    case LT_size:
      texture->reset_req(_xsize, _ysize);
      texture->set_margin(_msize < 0 ? margin : _msize);
      if (_omit) {
	texture->set_omit(Texture::OR_omitted);
      }
      return true;
      
    case LT_scale:
      texture->scale_req(_scale_pct);
      texture->set_margin(_msize < 0 ? margin : _msize);
      if (_omit) {
	texture->set_omit(Texture::OR_omitted);
      }
      return true;
      
    case LT_name:
      if (_omit) {
	texture->set_omit(Texture::OR_omitted);
      }
      return true;
      
    default:
      nout << "Unexpected type: " << (int)_line_type << "\n";
      abort();
    }
  }

  return false;
}

ostream &UserAttribLine::
list_textures(ostream &out) const {
  if (!_texture_names.empty()) {
    out << _texture_names[0]._pattern;
    for (int i = 1; i < (int)_texture_names.size(); i++) {
      out << " " << _texture_names[i]._pattern;
    }
  }
  return out;
}

bool UserAttribLine::
keyword_line(const string &line) {
  vector<string> words = extract_words(line);
  assert(!words.empty());

  if (words[0] == ":margin") {
    _line_type = LT_margin;
    if (words.size() != 2) {
      nout << "Expected margin size.\n";
      return false;
    }

    _msize = atoi(words[1].c_str());

  } else if (words[0] == ":palette") {
    _line_type = LT_palette;
    if (words.size() != 3) {
      nout << "Expected xsize ysize of palette.\n";
      return false;
    }
    _xsize = atoi(words[1].c_str());
    _ysize = atoi(words[2].c_str());
    _attrib_file->_pal_xsize = _xsize;
    _attrib_file->_pal_ysize = _ysize;

  } else {
    nout << "Unknown keyword: " << words[0] << "\n";
    return false;
  }

  return true;
}

bool UserAttribLine::
texture_line(const string &line) {
  _was_used = false;

  // Scan for a colon followed by a space.

  size_t colon = line.find(": ");
  if (colon == string::npos) {
    return old_style_line(line);
  }

  // Split the line into two parts at the colon.
  vector<string> names = extract_words(line.substr(0, colon));
  vector<string> params = extract_words(line.substr(colon + 2));

  vector<string>::const_iterator ni;
  for (ni = names.begin(); ni != names.end(); ++ni) {
    _texture_names.push_back(TextureName(*ni));
  }

  if (!params.empty() && params[params.size() - 1] == "omit") {
    // If the last word is "omit", set the omit flag and remove the
    // word.
    _omit = true;
    params.pop_back();
  }

  if (params.empty()) {
    _line_type = LT_name;
    return true;
  }

  // Is it a percentage?
  if (!params[0].empty() && params[0][params[0].size() - 1] == '%') {
    _line_type = LT_scale;
    _scale_pct = atof(params[0].c_str());
    if (_scale_pct <= 0.0) {
      nout << "Invalid scale percentage for ";
      copy(names.begin(), names.end(), ostream_iterator<string>(nout, " "));
      nout << ": " << _scale_pct << "%\n";
      return false;
    }
    return true;
  }

  // It must be xsize ysize [margin]
  if (params.size() == 2) {
    _line_type = LT_size;
    _xsize = atoi(params[0].c_str());
    _ysize = atoi(params[1].c_str());
    if (_xsize <= 0 || _ysize <= 0) {
      nout << "Invalid texture size for ";
      copy(names.begin(), names.end(), ostream_iterator<string>(nout, " "));
      nout << ": " << _xsize << " " << _ysize << "\n";
      return false;
    }
    return true;
  }

  if (params.size() == 3) {
    _line_type = LT_size;
    _xsize = atoi(params[0].c_str());
    _ysize = atoi(params[1].c_str());
    _msize = atoi(params[2].c_str());
    if (_xsize <= 0 || _ysize <= 0) {
      nout << "Invalid texture size for ";
      copy(names.begin(), names.end(), ostream_iterator<string>(nout, " "));
      nout << ": " << _xsize << " " << _ysize << "\n";
      return false;
    }
    return true;
  }

  nout << "Expected xsize ysize [msize]\n";
  return false;
}


bool UserAttribLine::
old_style_line(const string &line) {
  vector<string> words = extract_words(line);
  assert(!words.empty());

  if (words.size() != 3 && words.size() != 4) {
    nout << "Colon omitted.\n";
    return false;
  }

  _is_old_style = true;
  _line_type = LT_size;
  _texture_names.push_back(TextureName(words[0]));
  _xsize = atoi(words[1].c_str());
  _ysize = atoi(words[2].c_str());
  if (words.size() > 3) {
    _msize = atoi(words[3].c_str());

    if (_msize < 0) {
      _msize = -1;
      _omit = true;
      
    } else if (_msize == _attrib_file->_default_margin) {
      _msize = -1;
    }
  } else {
    _msize = -1;
  }

  if (_xsize <= 0 || _ysize <= 0) {
    nout << "Invalid texture size for " << words[0] << ": "
	 << _xsize << " " << _ysize << "\n";
    return false;
  }

  return true;
}
