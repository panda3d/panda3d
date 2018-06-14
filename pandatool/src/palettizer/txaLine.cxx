/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file txaLine.cxx
 * @author drose
 * @date 2000-11-30
 */

#include "txaLine.h"
#include "pal_string_utils.h"
#include "eggFile.h"
#include "palettizer.h"
#include "textureImage.h"
#include "sourceTextureImage.h"
#include "paletteGroup.h"

#include "pnotify.h"
#include "pnmFileType.h"

using std::string;

/**
 *
 */
TxaLine::
TxaLine() {
  _size_type = ST_none;
  _scale = 0.0;
  _x_size = 0;
  _y_size = 0;
  _aniso_degree = 0;
  _num_channels = 0;
  _format = EggTexture::F_unspecified;
  _force_format = false;
  _generic_format = false;
  _keep_format = false;
  _alpha_mode = EggRenderMode::AM_unspecified;
  _wrap_u = EggTexture::WM_unspecified;
  _wrap_v = EggTexture::WM_unspecified;
  _quality_level = EggTexture::QL_unspecified;
  _got_margin = false;
  _margin = 0;
  _got_coverage_threshold = false;
  _coverage_threshold = 0.0;
  _color_type = nullptr;
  _alpha_type = nullptr;
}

/**
 * Accepts a string that defines a line of the .txa file and parses it into
 * its constinuent parts.  Returns true if successful, false on error.
 */
bool TxaLine::
parse(const string &line) {
  size_t colon = line.find(':');
  if (colon == string::npos) {
    nout << "Colon required.\n";
    return false;
  }

  // Chop up the first part of the string (preceding the colon) into its
  // individual words.  These are patterns to match.
  vector_string words;
  extract_words(line.substr(0, colon), words);

  vector_string::iterator wi;
  for (wi = words.begin(); wi != words.end(); ++wi) {
    string word = (*wi);

    // If the pattern ends in the string ".egg", and only if it ends in this
    // string, it is deemed an egg pattern and will only be tested against egg
    // files.  If it ends in anything else, it is deemed a texture pattern and
    // will only be tested against textures.
    if (word.length() > 4 && word.substr(word.length() - 4) == ".egg") {
      GlobPattern pattern(word);
      pattern.set_case_sensitive(false);
      _egg_patterns.push_back(pattern);

    } else {
      // However, the filename extension, if any, is stripped off because the
      // texture key names nowadays don't include them.
      size_t dot = word.rfind('.');
      if (dot != string::npos) {
        word = word.substr(0, dot);
      }
      GlobPattern pattern(word);
      pattern.set_case_sensitive(false);
      _texture_patterns.push_back(pattern);
    }
  }

  if (_egg_patterns.empty() && _texture_patterns.empty()) {
    nout << "No texture or egg filenames given.\n";
    return false;
  }

  // Now chop up the rest of the string (following the colon) into its
  // individual words.  These are keywords and size indications.
  words.clear();
  extract_words(line.substr(colon + 1), words);

  wi = words.begin();
  while (wi != words.end()) {
    const string &word = *wi;
    nassertr(!word.empty(), false);

    if (isdigit(word[0])) {
      // This is either a new size or a scale percentage.
      if (_size_type != ST_none) {
        nout << "Invalid repeated size request: " << word << "\n";
        return false;
      }
      if (word[word.length() - 1] == '%') {
        // It's a scale percentage!
        _size_type = ST_scale;

        string tail;
        _scale = string_to_double(word, tail);
        if (!(tail == "%")) {
          // This is an invalid number.
          return false;
        }
        ++wi;

      } else {
        // Collect a number of consecutive numeric fields.
        pvector<int> numbers;
        while (wi != words.end() && isdigit((*wi)[0])) {
          const string &word = *wi;
          int num;
          if (!string_to_int(word, num)) {
            nout << "Invalid size: " << word << "\n";
            return false;
          }
          numbers.push_back(num);
          ++wi;
        }
        if (numbers.size() < 2) {
          nout << "At least two size numbers must be given, or a percent sign used to indicate scaling.\n";
          return false;

        } else if (numbers.size() == 2) {
          _size_type = ST_explicit_2;
          _x_size = numbers[0];
          _y_size = numbers[1];

        } else if (numbers.size() == 3) {
          _size_type = ST_explicit_3;
          _x_size = numbers[0];
          _y_size = numbers[1];
          _num_channels = numbers[2];

        } else {
          nout << "Too many size numbers given.\n";
          return false;
        }
      }

    } else {
      // The word does not begin with a digit; therefore it's either a keyword
      // or an image file type request.
      if (word == "omit") {
        _keywords.push_back(KW_omit);

      } else if (word == "nearest") {
        _keywords.push_back(KW_nearest);

      } else if (word == "linear") {
        _keywords.push_back(KW_linear);

      } else if (word == "mipmap") {
        _keywords.push_back(KW_mipmap);

      } else if (word == "cont") {
        _keywords.push_back(KW_cont);

      } else if (word == "margin") {
        ++wi;
        if (wi == words.end()) {
          nout << "Argument required for 'margin'.\n";
          return false;
        }

        const string &arg = (*wi);
        if (!string_to_int(arg, _margin)) {
          nout << "Not an integer: " << arg << "\n";
          return false;
        }
        if (_margin < 0) {
          nout << "Invalid margin: " << _margin << "\n";
          return false;
        }
        _got_margin = true;

      } else if (word == "aniso") {
        ++wi;
        if (wi == words.end()) {
          nout << "Integer argument required for 'aniso'.\n";
          return false;
        }

        const string &arg = (*wi);
        if (!string_to_int(arg, _aniso_degree)) {
          nout << "Not an integer: " << arg << "\n";
          return false;
        }
        if ((_aniso_degree < 2) || (_aniso_degree > 16)) {
          // make it an error to specific degree 0 or 1, which means no
          // anisotropy so it's probably an input mistake
          nout << "Invalid anistropic degree (range is 2-16): " << _aniso_degree << "\n";
          return false;
        }

        _keywords.push_back(KW_anisotropic);

      } else if (word == "coverage") {
        ++wi;
        if (wi == words.end()) {
          nout << "Argument required for 'coverage'.\n";
          return false;
        }

        const string &arg = (*wi);
        if (!string_to_double(arg, _coverage_threshold)) {
          nout << "Not a number: " << arg << "\n";
          return false;
        }
        if (_coverage_threshold <= 0.0) {
          nout << "Invalid coverage threshold: " << _coverage_threshold << "\n";
          return false;
        }
        _got_coverage_threshold = true;

      } else if (word.substr(0, 6) == "force-") {
        // Force a particular format, despite the number of channels in the
        // image.
        string format_name = word.substr(6);
        EggTexture::Format format = EggTexture::string_format(format_name);
        if (format != EggTexture::F_unspecified) {
          _format = format;
          _force_format = true;
        } else {
          nout << "Unknown image format: " << format_name << "\n";
          return false;
        }

      } else if (word == "generic") {
        // Genericize the image format by replacing bitcount-specific formats
        // with their generic equivalents, e.g.  rgba8 becomes rgba.
        _generic_format = true;

      } else if (word == "keep-format") {
        // Keep whatever image format was specified.
        _keep_format = true;

      } else {
        // Maybe it's a group name.
        PaletteGroup *group = pal->test_palette_group(word);
        if (group != nullptr) {
          _palette_groups.insert(group);

        } else {
          // Maybe it's a format name.  This suggests an image format, but may
          // be overridden to reflect the number of channels in the image.
          EggTexture::Format format = EggTexture::string_format(word);
          if (format != EggTexture::F_unspecified) {
            if (!_force_format) {
              _format = format;
            }
          } else {
            // Maybe it's an alpha mode.
            EggRenderMode::AlphaMode am = EggRenderMode::string_alpha_mode(word);
            if (am != EggRenderMode::AM_unspecified) {
              _alpha_mode = am;

            } else {
              // Maybe it's a quality level.
              EggTexture::QualityLevel ql = EggTexture::string_quality_level(word);
              if (ql != EggTexture::QL_unspecified) {
                _quality_level = ql;

              } else if (word.length() > 2 && word[word.length() - 2] == '_' &&
                         strchr("uv", word[word.length() - 1]) != nullptr) {
                // It must be a wrap mode for u or v.
                string prefix = word.substr(0, word.length() - 2);
                EggTexture::WrapMode wm = EggTexture::string_wrap_mode(prefix);
                if (wm == EggTexture::WM_unspecified) {
                  return false;
                }
                switch (word[word.length() - 1]) {
                case 'u':
                  _wrap_u = wm;
                  break;

                case 'v':
                  _wrap_v = wm;
                  break;
                }

              } else {
                // Maybe it's an image file request.
                if (!parse_image_type_request(word, _color_type, _alpha_type)) {
                  return false;
                }
              }
            }
          }
        }
      }
      ++wi;
    }
  }

  return true;
}

/**
 * Compares the patterns on the line to the indicated EggFile.  If they match,
 * updates the egg with the appropriate information.  Returns true if a match
 * is detected and the search for another line should stop, or false if a
 * match is not detected (or if the keyword "cont" is present, which means the
 * search should continue regardless).
 */
bool TxaLine::
match_egg(EggFile *egg_file) const {
  string name = egg_file->get_name();

  bool matched_any = false;
  Patterns::const_iterator pi;
  for (pi = _egg_patterns.begin();
       pi != _egg_patterns.end() && !matched_any;
       ++pi) {
    matched_any = (*pi).matches(name);
  }

  if (!matched_any) {
    // No match this line; continue.
    return false;
  }

  bool got_cont = false;
  Keywords::const_iterator ki;
  for (ki = _keywords.begin(); ki != _keywords.end(); ++ki) {
    switch (*ki) {
    case KW_omit:
      break;

    case KW_nearest:
    case KW_linear:
    case KW_mipmap:
    case KW_anisotropic:
      // These mean nothing to an egg file.
      break;

    case KW_cont:
      got_cont = true;
      break;
    }
  }

  egg_file->match_txa_groups(_palette_groups);

  if (got_cont) {
    // If we have the "cont" keyword, we should keep scanning for another
    // line, even though we matched this one.
    return false;
  }

  // Otherwise, in the normal case, a match ends the search for matches.
  egg_file->clear_surprise();

  return true;
}

/**
 * Compares the patterns on the line to the indicated TextureImage.  If they
 * match, updates the texture with the appropriate information.  Returns true
 * if a match is detected and the search for another line should stop, or
 * false if a match is not detected (or if the keyword "cont" is present,
 * which means the search should continue regardless).
 */
bool TxaLine::
match_texture(TextureImage *texture) const {
  string name = texture->get_name();

  bool matched_any = false;
  Patterns::const_iterator pi;
  for (pi = _texture_patterns.begin();
       pi != _texture_patterns.end() && !matched_any;
       ++pi) {
    matched_any = (*pi).matches(name);
  }

  if (!matched_any) {
    // No match this line; continue.
    return false;
  }

  SourceTextureImage *source = texture->get_preferred_source();
  TextureRequest &request = texture->_request;

  if (!request._got_size) {
    switch (_size_type) {
    case ST_none:
      break;

    case ST_scale:
      if (source != nullptr && source->get_size()) {
        request._got_size = true;
        request._x_size = std::max(1, (int)(source->get_x_size() * _scale / 100.0));
        request._y_size = std::max(1, (int)(source->get_y_size() * _scale / 100.0));
      }
      break;

    case ST_explicit_3:
      request._got_num_channels = true;
      request._num_channels = _num_channels;
      // fall through

    case ST_explicit_2:
      request._got_size = true;
      request._x_size = _x_size;
      request._y_size = _y_size;
      break;
    }
  }

  if (_got_margin) {
    request._margin = _margin;
  }

  if (_got_coverage_threshold) {
    request._coverage_threshold = _coverage_threshold;
  }

  if (_color_type != nullptr) {
    request._properties._color_type = _color_type;
    request._properties._alpha_type = _alpha_type;
  }

  if (_quality_level != EggTexture::QL_unspecified) {
    request._properties._quality_level = _quality_level;
  }

  if (_format != EggTexture::F_unspecified) {
    request._format = _format;
    request._force_format = _force_format;
    request._generic_format = false;
  }

  if (_generic_format) {
    request._generic_format = true;
  }

  if (_keep_format) {
    request._keep_format = true;
  }

  if (_alpha_mode != EggRenderMode::AM_unspecified) {
    request._alpha_mode = _alpha_mode;
  }

  if (_wrap_u != EggTexture::WM_unspecified) {
    request._wrap_u = _wrap_u;
  }
  if (_wrap_v != EggTexture::WM_unspecified) {
    request._wrap_v = _wrap_v;
  }

  bool got_cont = false;
  Keywords::const_iterator ki;
  for (ki = _keywords.begin(); ki != _keywords.end(); ++ki) {
    switch (*ki) {
    case KW_omit:
      request._omit = true;
      break;

    case KW_nearest:
      request._minfilter = EggTexture::FT_nearest;
      request._magfilter = EggTexture::FT_nearest;
      break;

    case KW_linear:
      request._minfilter = EggTexture::FT_linear;
      request._magfilter = EggTexture::FT_linear;
      break;

    case KW_mipmap:
      request._minfilter = EggTexture::FT_linear_mipmap_linear;
      request._magfilter = EggTexture::FT_linear_mipmap_linear;
      break;

    case KW_anisotropic:
      request._anisotropic_degree = _aniso_degree;
      break;

    case KW_cont:
      got_cont = true;
      break;
    }
  }

  texture->_explicitly_assigned_groups.make_union
    (texture->_explicitly_assigned_groups, _palette_groups);
  texture->_explicitly_assigned_groups.remove_null();

  if (got_cont) {
    // If we have the "cont" keyword, we should keep scanning for another
    // line, even though we matched this one.
    return false;
  }

  // Otherwise, in the normal case, a match ends the search for matches.
  texture->_is_surprise = false;

  return true;
}

/**
 *
 */
void TxaLine::
output(std::ostream &out) const {
  Patterns::const_iterator pi;
  for (pi = _texture_patterns.begin(); pi != _texture_patterns.end(); ++pi) {
    out << (*pi) << " ";
  }
  for (pi = _egg_patterns.begin(); pi != _egg_patterns.end(); ++pi) {
    out << (*pi) << " ";
  }
  out << ":";

  switch (_size_type) {
  case ST_none:
    break;

  case ST_scale:
    out << " " << _scale << "%";
    break;

  case ST_explicit_2:
    out << " " << _x_size << " " << _y_size;
    break;

  case ST_explicit_3:
    out << " " << _x_size << " " << _y_size << " " << _num_channels;
    break;
  }

  if (_got_margin) {
    out << " margin " << _margin;
  }

  if (_got_coverage_threshold) {
    out << " coverage " << _coverage_threshold;
  }

  Keywords::const_iterator ki;
  for (ki = _keywords.begin(); ki != _keywords.end(); ++ki) {
    switch (*ki) {
    case KW_omit:
      out << " omit";
      break;

    case KW_nearest:
      out << " nearest";
      break;

    case KW_linear:
      out << " linear";
      break;

    case KW_mipmap:
      out << " mipmap";
      break;

    case KW_cont:
      out << " cont";
      break;

    case KW_anisotropic:
      out << " aniso " << _aniso_degree;
      break;
    }
  }

  PaletteGroups::const_iterator gi;
  for (gi = _palette_groups.begin(); gi != _palette_groups.end(); ++gi) {
    out << " " << (*gi)->get_name();
  }

  if (_format != EggTexture::F_unspecified) {
    out << " " << _format;
    if (_force_format) {
      out << " (forced)";
    }
  }

  if (_color_type != nullptr) {
    out << " " << _color_type->get_suggested_extension();
    if (_alpha_type != nullptr) {
      out << "," << _alpha_type->get_suggested_extension();
    }
  }
}
