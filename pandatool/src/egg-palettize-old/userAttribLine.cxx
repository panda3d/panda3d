// Filename: userAttribLine.cxx
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#include "userAttribLine.h"
#include "string_utils.h"
#include "pTexture.h"
#include "paletteGroup.h"
#include "sourceEgg.h"
#include "attribFile.h"
#include "textureEggRef.h"

#include <notify.h>

#include <ctype.h>

UserAttribLine::
UserAttribLine(const string &cline, AttribFile *af) : _attrib_file(af) {
  _is_old_style = false;

  // By default, all lines are marked 'was_used'.  That means they'll
  // be preserved across a -k on the command line.  Lines that name
  // textures will have _was_used set false until a texture actually
  // matches that line.
  _was_used = true;

  _got_dirname = false;

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
  int i;

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

  case LT_group_relate:
    out << ":group " << _names[0];
    if (!_got_dirname) {
      out << " dir " << _dirname;
    }
    if (!_names.empty()) {
      out << " with";
      for (i = 1; i < (int)_names.size(); i++) {
	out << " " << _names[i];
      }
    }
    out << "\n";
    break;

  case LT_size:
    list_patterns(out) << " : " << _xsize << " " << _ysize;
    if (_msize > 0) {
      out << " " << _msize;
    }
    if (_omit) {
      out << " omit";
    }
    break;

  case LT_scale:
    list_patterns(out) << " : " << _scale_pct << "%";
    if (_msize > 0) {
      out << " " << _msize;
    }
    if (_omit) {
      out << " omit";
    }
    break;

  case LT_name:
    list_patterns(out) << " :";
    if (_omit) {
      out << " omit";
    }
    break;

  case LT_group_assign:
    list_patterns(out) << " : ";
    list_names(out);
    break;

  default:
    nout << "Unexpected type: " << (int)_line_type << "\n";
    abort();
  };

  out << _comment << "\n";
}

bool UserAttribLine::
get_size_request(PTexture *texture, int &margin) {
  if (_line_type == LT_group_assign) {
    // We don't care about group lines for the size check.
    return false;
  }

  // See if the texture name matches any of the filename patterns on
  // this line.
  string texture_name = texture->get_name();

  bool matched_any = false;
  Patterns::const_iterator pi;
  for (pi = _patterns.begin();
       pi != _patterns.end() && !matched_any;
       ++pi) {
    if ((*pi).matches(texture_name)) {
      matched_any = true;
    }

    // Also check if it matches any of the egg files this texture is
    // on.
    PTexture::Eggs::const_iterator ei;
    for (ei = texture->_eggs.begin(); 
	 ei != texture->_eggs.end() && !matched_any;
	 ++ei) {
      string egg_name = (*ei)->_egg->get_egg_filename().get_basename();
      if ((*pi).matches(egg_name)) {
	matched_any = true;
      }
    }
  }

  if (matched_any) {
    _was_used = true;

    // It does!  So do the right thing with this line.
    switch (_line_type) {
    case LT_invalid:
    case LT_comment:
    case LT_palette:
    case LT_group_relate:
      return false;
      
    case LT_margin:
      margin = _msize;
      return false;
      
    case LT_size:
      texture->reset_req(_xsize, _ysize);
      texture->set_margin(_msize < 0 ? margin : _msize);
      if (_omit) {
	texture->user_omit();
      }
      return true;
      
    case LT_scale:
      texture->scale_req(_scale_pct);
      texture->set_margin(_msize < 0 ? margin : _msize);
      if (_omit) {
	texture->user_omit();
      }
      return true;
      
    case LT_name:
      if (_omit) {
	texture->user_omit();
      }
      return true;
      
    default:
      nout << "Unexpected type: " << (int)_line_type << "\n";
      abort();
    }
  }

  return false;
}

bool UserAttribLine::
get_group_request(SourceEgg *egg) {
  if (_line_type != LT_group_assign) {
    // We're only looking for group lines now.
    return false;
  }

  // See if the egg filename matches any of the filename patterns on
  // this line.
  string egg_name = egg->get_egg_filename().get_basename();

  bool matched_any = false;
  Patterns::const_iterator pi;
  for (pi = _patterns.begin();
       pi != _patterns.end() && !matched_any;
       ++pi) {
    if ((*pi).matches(egg_name)) {
      matched_any = true;
    }
  }

  if (matched_any) {
    _was_used = true;

    // It does!  Record the list of required groups with all of the
    // textures on this egg file.
    nassertr(!_names.empty(), false);
    PaletteGroups groups;
    Names::const_iterator ni = _names.begin();
    PaletteGroup *group = _attrib_file->get_group(*ni);
    groups.insert(group);

    // The first-named group is preferred for any textures not already
    // on another group.
    PaletteGroup *preferred = group;

    while (ni != _names.end()) {
      group = _attrib_file->get_group(*ni);
      groups.insert(group);
      ++ni;
    }
    PaletteGroup::complete_groups(groups);
    egg->require_groups(preferred, groups);

    return true;
  }

  return false;
}

ostream &UserAttribLine::
list_patterns(ostream &out) const {
  if (!_patterns.empty()) {
    out << _patterns[0];
    for (int i = 1; i < (int)_patterns.size(); i++) {
      out << " " << _patterns[i];
    }
  }
  return out;
}

ostream &UserAttribLine::
list_names(ostream &out) const {
  if (!_names.empty()) {
    out << _names[0];
    for (int i = 1; i < (int)_names.size(); i++) {
      out << " " << _names[i];
    }
  }
  return out;
}

bool UserAttribLine::
keyword_line(const string &line) {
  vector_string words = extract_words(line);
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
      nout << "Expected :palette xsize ysize.\n";
      return false;
    }
    _xsize = atoi(words[1].c_str());
    _ysize = atoi(words[2].c_str());
    _attrib_file->_pal_xsize = _xsize;
    _attrib_file->_pal_ysize = _ysize;

  } else if (words[0] == ":group") {
    _line_type = LT_group_relate;
    if (words.size() < 2) {
      nout << "Expected :group name.\n";
      return false;
    }

    PaletteGroup *group = _attrib_file->get_group(words[1]);

    int kw = 2;
    while (kw < (int)words.size()) {
      if (words[kw] == "with") {
	kw++;
	
	while (kw < (int)words.size()) {
	  group->add_parent(_attrib_file->get_group(words[kw]));
	  kw++;
	}

      } else if (words[kw] == "dir") {
	kw++;
	_got_dirname = true;
	_dirname = words[kw];
	group->set_dirname(_dirname);
	kw++;

      } else {
	nout << "Invalid keyword: " << words[kw] << "\n";
	return false;
      }
    }

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
  vector_string names = extract_words(line.substr(0, colon));
  vector_string params = extract_words(line.substr(colon + 2));

  vector_string::const_iterator ni;
  for (ni = names.begin(); ni != names.end(); ++ni) {
    _patterns.push_back(GlobPattern(*ni));
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

  // Is it a group name?  If it is, this is an assignment of a texture
  // or egg file to one or more groups.
  if (!params[0].empty() && isalpha(params[0][0])) {
    _names = params;
    _line_type = LT_group_assign;
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
  vector_string words = extract_words(line);
  assert(!words.empty());

  if (words.size() != 3 && words.size() != 4) {
    nout << "Colon omitted.\n";
    return false;
  }

  _is_old_style = true;
  _line_type = LT_size;
  _patterns.push_back(GlobPattern(words[0]));
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
