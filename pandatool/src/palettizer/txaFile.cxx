/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file txaFile.cxx
 * @author drose
 * @date 2000-11-30
 */

#include "txaFile.h"
#include "pal_string_utils.h"
#include "palettizer.h"
#include "paletteGroup.h"
#include "textureImage.h"

#include "pnotify.h"
#include "pnmFileTypeRegistry.h"

using std::string;

/**
 *
 */
TxaFile::
TxaFile() {
}

/**
 * Reads the indicated stream, and returns true if successful, or false if
 * there is an error.
 */
bool TxaFile::
read(std::istream &in, const string &filename) {
  string line;
  int line_number = 1;

  int ch = get_line_or_semicolon(in, line);
  while (ch != EOF || !line.empty()) {
    bool okflag = true;

    // Strip off the comment.
    size_t hash = line.find('#');
    if (hash != string::npos) {
      line = line.substr(0, hash);
    }
    line = trim_left(line);
    if (line.empty()) {
      // Empty lines are ignored.

    } else if (line[0] == ':') {
      // This is a keyword line.
      vector_string words;
      extract_words(line, words);
      if (words[0] == ":group") {
        okflag = parse_group_line(words);

      } else if (words[0] == ":palette") {
        okflag = parse_palette_line(words);

      } else if (words[0] == ":margin") {
        okflag = parse_margin_line(words);

      } else if (words[0] == ":background") {
        okflag = parse_background_line(words);

      } else if (words[0] == ":coverage") {
        okflag = parse_coverage_line(words);

      } else if (words[0] == ":powertwo") {
        okflag = parse_powertwo_line(words);

      } else if (words[0] == ":imagetype") {
        okflag = parse_imagetype_line(words);

      } else if (words[0] == ":shadowtype") {
        okflag = parse_shadowtype_line(words);

      } else if (words[0] == ":round") {
        okflag = parse_round_line(words);

      } else if (words[0] == ":remap") {
        okflag = parse_remap_line(words);

      } else if (words[0] == ":cutout") {
        okflag = parse_cutout_line(words);

      } else if (words[0] == ":textureswap") {
        okflag = parse_textureswap_line(words);

      } else {
        nout << "Invalid keyword " << words[0] << "\n";
        okflag = false;
      }

    } else {
      _lines.push_back(TxaLine());
      TxaLine &txa_line = _lines.back();

      okflag = txa_line.parse(line);
    }

    if (!okflag) {
      nout << "Error on line " << line_number << " of " << filename << "\n";
      return false;
    }
    if (ch == '\n') {
      line_number++;
    }
    ch = get_line_or_semicolon(in, line);
  }

  if (!in.eof()) {
    nout << "I/O error reading " << filename << "\n";
    return false;
  }

  return true;
}

/**
 * Searches for a matching line in the .txa file for the given egg file and
 * applies its specifications.  If a match is found, returns true; otherwise,
 * returns false.  Also returns false if all the matching lines for the egg
 * file include the keyword "cont".
 */
bool TxaFile::
match_egg(EggFile *egg_file) const {
  Lines::const_iterator li;
  for (li = _lines.begin(); li != _lines.end(); ++li) {
    if ((*li).match_egg(egg_file)) {
      return true;
    }
  }

  return false;
}

/**
 * Searches for a matching line in the .txa file for the given texture and
 * applies its specifications.  If a match is found, returns true; otherwise,
 * returns false.  Also returns false if all the matching lines for the
 * texture include the keyword "cont".
 */
bool TxaFile::
match_texture(TextureImage *texture) const {
  Lines::const_iterator li;
  for (li = _lines.begin(); li != _lines.end(); ++li) {
    if ((*li).match_texture(texture)) {
      return true;
    }
  }

  return false;
}

/**
 * Outputs a representation of the lines that were read in to the indicated
 * output stream.  This is primarily useful for debugging.
 */
void TxaFile::
write(std::ostream &out) const {
  Lines::const_iterator li;
  for (li = _lines.begin(); li != _lines.end(); ++li) {
    out << (*li) << "\n";
  }
}

/**
 * Reads the next line, or the next semicolon-delimited phrase, from the
 * indicated input stream.  Returns the character that marks the end of the
 * line, or EOF if the end of file has been reached.
 */
int TxaFile::
get_line_or_semicolon(std::istream &in, string &line) {
  line = string();
  int ch = in.get();
  char semicolon = ';';

  while (ch != EOF && ch != '\n' && ch != semicolon) {
    if (ch == '#') {
      // We don't consider a semicolon within a comment to be a line break.
      semicolon = EOF;
    }
    line += ch;
    ch = in.get();
  }

  return ch;
}

/**
 * Handles the line in a .txa file that begins with the keyword ":group" and
 * indicates the relationships between one or more groups.
 */
bool TxaFile::
parse_group_line(const vector_string &words) {
  vector_string::const_iterator wi;
  wi = words.begin();
  assert (wi != words.end());
  ++wi;

  const string &group_name = (*wi);
  PaletteGroup *group = pal->get_palette_group(group_name);
  ++wi;

  enum State {
    S_none,
    S_on,
    S_includes,
    S_dir,
    S_margin,
  };
  State state = S_none;

  bool first_on = true;

  while (wi != words.end()) {
    const string &word = (*wi);
    if (word == "with") {
      // Deprecated keyword: "with" means the same thing as "on".
      state = S_on;

    } else if (word == "on") {
      state = S_on;

    } else if (word == "includes") {
      state = S_includes;

    } else if (word == "dir") {
      state = S_dir;

    } else if (word == "margin") {
      state = S_margin;

    } else {
      switch (state) {
      case S_none:
        nout << "Invalid keyword: " << word << "\n";
        return false;

      case S_on:
        {
          PaletteGroup *on_group = pal->get_palette_group(word);
          if (first_on) {
            if (!group->has_dirname() && on_group->has_dirname()) {
              group->set_dirname(on_group->get_dirname());
            }
            first_on = false;
          }
          group->group_with(on_group);
        }
        break;

      case S_includes:
        pal->get_palette_group(word)->group_with(group);
        break;

      case S_dir:
        group->set_dirname(word);
        state = S_none;
        break;

      case S_margin:
        int margin_override;
        if (string_to_int(word, margin_override)) {
          group->set_margin_override(margin_override);
        }
        state = S_none;
        break;
      }

    }

    ++wi;
  }

  return true;
}

/**
 * Handles the line in a .txa file that begins with the keyword ":palette" and
 * indicates the appropriate size for the palette images.
 */
bool TxaFile::
parse_palette_line(const vector_string &words) {
  if (words.size() != 3) {
    nout << "Exactly two parameters required for :palette, the x and y "
         << "size of the palette images to generate.\n";
    return false;
  }

  if (!string_to_int(words[1], pal->_pal_x_size) ||
      !string_to_int(words[2], pal->_pal_y_size)) {
    nout << "Invalid palette size: " << words[1] << " " << words[2] << "\n";
    return false;
  }

  if (pal->_pal_x_size <= 0 || pal->_pal_y_size <= 0) {
    nout << "Invalid palette size: " << pal->_pal_x_size
         << " " << pal->_pal_y_size << "\n";
    return false;
  }

  return true;
}

/**
 * Handles the line in a .txa file that begins with the keyword ":margin" and
 * indicates the default margin size.
 */
bool TxaFile::
parse_margin_line(const vector_string &words) {
  if (words.size() != 2) {
    nout << "Exactly one parameter required for :margin, the "
         << "size of the default margin to apply.\n";
    return false;
  }

  if (!string_to_int(words[1], pal->_margin)) {
    nout << "Invalid margin: " << words[1] << "\n";
    return false;
  }

  if (pal->_margin < 0) {
    nout << "Invalid margin: " << pal->_margin << "\n";
    return false;
  }

  return true;
}

/**
 * Handles the line in a .txa file that begins with the keyword ":background"
 * and indicates the palette background color.
 */
bool TxaFile::
parse_background_line(const vector_string &words) {
  if (words.size() != 5) {
    nout << "Exactly four parameter required for :background: the "
         << "four [r g b a] components of the background color.\n";
    return false;
  }

  if (!string_to_double(words[1], pal->_background[0]) ||
      !string_to_double(words[2], pal->_background[1]) ||
      !string_to_double(words[3], pal->_background[2]) ||
      !string_to_double(words[4], pal->_background[3])) {
    nout << "Invalid color: "
         << words[1] << " " << words[2] << " "
         << words[3] << " " << words[4] << " " << "\n";
    return false;
  }

  return true;
}

/**
 * Handles the line in a .txa file that begins with the keyword ":coverage"
 * and indicates the default coverage threshold.
 */
bool TxaFile::
parse_coverage_line(const vector_string &words) {
  if (words.size() != 2) {
    nout << "Exactly one parameter required for :coverage, the "
         << "value for the default coverage threshold.\n";
    return false;
  }


  if (!string_to_double(words[1], pal->_coverage_threshold)) {
    nout << "Invalid coverage threshold: " << words[1] << "\n";
    return false;
  }

  if (pal->_coverage_threshold <= 0.0) {
    nout << "Invalid coverage threshold: " << pal->_coverage_threshold << "\n";
    return false;
  }

  return true;
}

/**
 * Handles the line in a .txa file that begins with the keyword ":powertwo"
 * and indicates whether textures should by default be forced to a power of
 * two.
 */
bool TxaFile::
parse_powertwo_line(const vector_string &words) {
  if (words.size() != 2) {
    nout << "Exactly one parameter required for :powertwo, either a 0 "
         << "or a 1.\n";
    return false;
  }

  int flag;
  if (!string_to_int(words[1], flag)) {
    nout << "Invalid powertwo flag: " << words[1] << "\n";
    return false;
  }

  if (flag != 0 && flag != 1) {
    nout << "Invalid powertwo flag: " << flag << "\n";
    return false;
  }

  pal->_force_power_2 = (flag != 0);

  return true;
}

/**
 * Handles the line in a .txa file that begins with the keyword ":imagetype"
 * and indicates the default image file type to convert palettes and textures
 * to.
 */
bool TxaFile::
parse_imagetype_line(const vector_string &words) {
  if (words.size() != 2) {
    nout << "Exactly one parameter required for :imagetype.\n";
    return false;
  }
  const string &imagetype = words[1];
  if (!parse_image_type_request(imagetype, pal->_color_type, pal->_alpha_type)) {
    nout << "\nKnown image types are:\n";
    PNMFileTypeRegistry::get_global_ptr()->write(nout, 2);
    nout << "\n";
    return false;
  }

  return true;
}

/**
 * Handles the line in a .txa file that begins with the keyword ":shadowtype"
 * and indicates the image file type to convert working copies of the palette
 * images to.
 */
bool TxaFile::
parse_shadowtype_line(const vector_string &words) {
  if (words.size() != 2) {
    nout << "Exactly one parameter required for :shadowtype.\n";
    return false;
  }
  const string &shadowtype = words[1];
  if (!parse_image_type_request(shadowtype, pal->_shadow_color_type,
                                pal->_shadow_alpha_type)) {
    nout << "\nKnown image types are:\n";
    PNMFileTypeRegistry::get_global_ptr()->write(nout, 2);
    nout << "\n";
    return false;
  }

  return true;
}

/**
 * Handles the line in a .txa file that begins with the keyword ":round" and
 * indicates how or whether to round up UV minmax boxes.
 */
bool TxaFile::
parse_round_line(const vector_string &words) {
  if (words.size() == 2) {
    if (words[1] == "no") {
      pal->_round_uvs = false;
      return true;
    } else {
      nout << "Invalid round keyword: " << words[1] << ".\n"
           << "Expected 'no', or a round fraction and fuzz factor.\n";
      return false;
    }
  }

  if (words.size() != 3) {
    nout << "Exactly two parameters required for :round, the fraction "
         << "to round to, and the fuzz factor.\n";
    return false;
  }

  if (!string_to_double(words[1], pal->_round_unit) ||
      !string_to_double(words[2], pal->_round_fuzz)) {
    nout << "Invalid rounding: " << words[1] << " " << words[2] << "\n";
    return false;
  }

  if (pal->_round_unit <= 0.0 || pal->_round_fuzz < 0.0) {
    nout << "Invalid rounding: " << pal->_round_unit
         << " " << pal->_round_fuzz << "\n";
    return false;
  }

  pal->_round_uvs = true;
  return true;
}

/**
 * Handles the line in a .txa file that begins with the keyword ":remap" and
 * indicates how or whether to remap UV coordinates in egg files to the unit
 * box.
 */
bool TxaFile::
parse_remap_line(const vector_string &words) {
  int i = 1;
  while (i < (int)words.size()) {
    const string &keyword = words[i];
    if (keyword == "char") {
      // Defining how to remap UV's for characters.
      i++;
      if (i == (int)words.size()) {
        nout << "Keyword expected following 'char'\n";
        return false;
      }
      pal->_remap_char_uv = Palettizer::string_remap(words[i]);
      if (pal->_remap_char_uv == Palettizer::RU_invalid) {
        nout << "Invalid remap keyword: " << words[i] << "\n";
        return false;
      }

    } else {
      // Defining how to remap UV's in general.
      pal->_remap_uv = Palettizer::string_remap(words[i]);
      if (pal->_remap_uv == Palettizer::RU_invalid) {
        nout << "Invalid remap keyword: " << words[i] << "\n";
        return false;
      }

      pal->_remap_char_uv = pal->_remap_uv;
    }

    i++;
  }

  return true;
}


/**
 * Handles the line in a .txa file that begins with the keyword ":cutout" and
 * indicates how to handle alpha-cutout textures: those textures that appear
 * to be mostly solid parts and invisible parts, with a thin border of
 * antialiased alpha along the boundary.
 */
bool TxaFile::
parse_cutout_line(const vector_string &words) {
  if (words.size() < 2 || words.size() > 3) {
    nout << ":cutout alpha-mode [ratio]\n";
    return false;
  }

  EggRenderMode::AlphaMode am = EggRenderMode::string_alpha_mode(words[1]);
  if (am == EggRenderMode::AM_unspecified) {
    nout << "Invalid cutout keyword: " << words[1] << "\n";
    return false;
  }
  pal->_cutout_mode = am;

  if (words.size() >= 3) {
    if (!string_to_double(words[2], pal->_cutout_ratio)) {
      nout << "Invalid cutout ratio: " << words[2] << "\n";
    }
  }

  return true;
}

/**
 * Handles the line in a .txa file that begins with the keyword ":textureswap"
 * and indicates the relationships between textures to be swapped.
 */
bool TxaFile::
parse_textureswap_line(const vector_string &words) {
  vector_string::const_iterator wi;
  wi = words.begin();
  assert (wi != words.end());
  ++wi;

  const string &group_name = (*wi);
  PaletteGroup *group = pal->get_palette_group(group_name);
  ++wi;

  string sourceTextureName = (*wi);
  ++wi;

  // vector_string swapTextures; copy(words.begin(), words.end(),
  // swapTextures); group->add_texture_swap_info(sourceTextureName,
  // swapTextures);
  size_t dot = sourceTextureName.rfind('.');
  if (dot != string::npos) {
    sourceTextureName = sourceTextureName.substr(0, dot);
  }
  group->add_texture_swap_info(sourceTextureName, words);

  return true;
}
