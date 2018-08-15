/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qtessInputFile.cxx
 * @author drose
 * @date 2003-10-13
 */

#include "qtessInputFile.h"
#include "config_egg_qtess.h"
#include "string_utils.h"

using std::string;

/**
 *
 */
QtessInputFile::
QtessInputFile() {
}

/**
 * reads the input file.
 */
bool QtessInputFile::
read(const Filename &filename) {
  _filename = Filename::text_filename(filename);
  _entries.clear();

  std::ifstream input;
  if (!_filename.open_read(input)) {
    qtess_cat.error()
      << "Unable to open input file " << _filename << "\n";
    return false;
  }

  string complete_line;

  int line_number = 0;
  string line;
  while (std::getline(input, line)) {
    line_number++;

    // Eliminate comments.  We have to scan the line repeatedly until we find
    // the first hash mark that's preceded by whitespace.
    size_t comment = line.find('#');
    while (comment != string::npos) {
      if (comment == 0 || isspace(line[comment - 1])) {
        line = line.substr(0, comment);
        comment = string::npos;

      } else {
        comment = line.find('#', comment + 1);
      }
    }

    // Check for a trailing backslash: continuation character.
    line = trim_right(line);
    if (!line.empty() && line[line.size() - 1] == '\\') {
      // We have a continuation character; go back and read some more.
      complete_line += line.substr(0, line.size() - 1);

    } else {
      // It's a complete line.  Begin parsing.
      line = trim(complete_line + line);
      complete_line = string();

      if (!line.empty()) {
        QtessInputEntry entry;

        // Scan for the first colon followed by whitespace.
        size_t colon = line.find(": ");
        if (colon == string::npos) {
          qtess_cat.error()
            << _filename << ": line " << line_number
            << " has no colon followed by whitespace.\n";
          return false;
        }
        if (colon == 0) {
          qtess_cat.error()
            << _filename << ": line " << line_number
            << " has no nodes.\n";
          return false;
        }

        // Split the line into two groups of words at the colon: names before
        // the colon, and params following it.
        vector_string names, params;
        extract_words(line.substr(0, colon), names);
        extract_words(line.substr(colon + 1), params);

        vector_string::const_iterator ni;
        for (ni = names.begin(); ni != names.end(); ++ni) {
          entry.add_node_name(*ni);
        }

        // Scan for things like ap, ad, ar, and pull them out of the stream.
        vector_string::iterator ci, cnext;
        ci = params.begin();
        while (ci != params.end()) {
          cnext = ci;
          ++cnext;

          string param = *ci;
          bool invert = false;
          if (param[0] == '!' && param.size() > 1) {
            invert = true;
            param = param.substr(1);
          }
          if (tolower(param[0]) == 'a' && param.size() > 1) {
            switch (tolower(param[1])) {
            case 'p':
              entry._auto_place = !invert;
              break;

            case 'd':
              entry._auto_distribute = !invert;
              break;

            case 'r':
              if (!string_to_double(param.substr(2), entry._curvature_ratio)) {
                qtess_cat.error()
                  << _filename << ": line " << line_number
                  << " - invalid field " << param << "\n";
                return false;
              }
              break;

            default:
              qtess_cat.error()
                << _filename << ": invalid parameters at line "
                << line_number << ".\n";
              return false;
            }
            params.erase(ci);
          } else {
            ci = cnext;
          }
        }

        if (!params.empty()) {
          bool okflag = true;
          if (cmp_nocase(params[0], "omit")==0) {
            entry.set_omit();

          } else if (cmp_nocase(params[0], "matchuu")==0) {
            entry.set_match_uu();
            if (params.size() > 1 && cmp_nocase(params[1], "matchvv")==0) {
              entry.set_match_vv();
            }

          } else if (cmp_nocase(params[0], "matchvv")==0) {
            entry.set_match_vv();
            if (params.size() > 1 && cmp_nocase(params[1], "matchuu")==0) {
              entry.set_match_uu();
            }

          } else if (cmp_nocase(params[0], "matchuv")==0) {
            entry.set_match_uv();
            if (params.size() > 1 && cmp_nocase(params[1], "matchvu")==0) {
              entry.set_match_vu();
            }

          } else if (cmp_nocase(params[0], "matchvu")==0) {
            entry.set_match_vu();
            if (params.size() > 1 && cmp_nocase(params[1], "matchuv")==0) {
              entry.set_match_uv();
            }

          } else if (cmp_nocase(params[0], "minu")==0) {
            // minu #: minimum tesselation in U.
            if (params.size() < 2) {
              okflag = false;
            } else {
              int value = 0;
              okflag = string_to_int(params[1], value);
              entry.set_min_u(value);
            }

          } else if (cmp_nocase(params[0], "minv")==0) {
            // minu #: minimum tesselation in V.
            if (params.size() < 2) {
              okflag = false;
            } else {
              int value = 0;
              okflag = string_to_int(params[1], value);
              entry.set_min_v(value);
            }

          } else if (tolower(params[0][0]) == 'i') {
            // "i#": per-isoparam tesselation.
            int value = 0;
            okflag = string_to_int(params[0].substr(1), value);
            entry.set_per_isoparam(value);

          } else if (params[0][params[0].length() - 1] == '%') {
            double value = 0.0;
            okflag = string_to_double(params[0].substr(0, params[0].length() - 1), value);
            entry.set_importance(value / 100.0);

          } else if (params.size() == 1) {
            // One numeric parameter: the number of triangles.
            int value = 0;
            okflag = string_to_int(params[0], value);
            entry.set_num_tris(value);

          } else if (params.size() >= 2) {
            // Two or more numeric parameters: the number of u by v quads,
            // followed by an optional list of specific isoparams.
            int u = 0, v = 0;
            okflag = string_to_int(params[0], u) && string_to_int(params[1], v);
            entry.set_uv(u, v, &params[2], params.size() - 2);

          } else {
            okflag = false;
          }

          if (!okflag) {
            qtess_cat.error()
              << _filename << ": invalid parameters at line "
              << line_number << ".\n";
            return false;
          }
        }
        _entries.push_back(entry);
      }
    }
  }

  if (qtess_cat.is_info()) {
    qtess_cat.info()
      << "read qtess parameter file " << _filename << ".\n";
    if (qtess_cat.is_debug()) {
      write(qtess_cat.debug(false));
    }
  }

  add_default_entry();

  return true;
}

/**
 * Returns a reference to the last entry on the list, which is the "default"
 * entry that will match any surface that does not get explicitly named in the
 * input file.
 */
QtessInputEntry &QtessInputFile::
get_default_entry() {
  if (_entries.empty()) {
    // No entries; create one.
    add_default_entry();
  }
  return _entries.back();
}


/**
 * Attempts to find a match for the given surface in the user input entries.
 * Searches in the order in which the entries were defined, and chooses the
 * first match.
 *
 * When a match is found, the surface is added to the entry's set of matched
 * surfaces.  Returns the type of the matching node if a match is found, or
 * T_undefined otherwise.
 */
QtessInputEntry::Type QtessInputFile::
match(QtessSurface *surface) {
  QtessInputEntry::Type type;

  if (_entries.empty()) {
    // No entries; create one.
    add_default_entry();
  }

  Entries::iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    type = (*ei).match(surface);
    if (type != QtessInputEntry::T_undefined) {
      return type;
    }
  }
  return QtessInputEntry::T_undefined;
}

/**
 * Determines the tesselation u,v amounts of each attached surface, and stores
 * this information in the surface pointer.  Returns the total number of tris
 * that will be produced.
 */
int QtessInputFile::
count_tris() {
  int total_tris = 0;

  Entries::iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    total_tris += (*ei).count_tris();
  }
  return total_tris;
}

/**
 *
 */
void QtessInputFile::
write(std::ostream &out, int indent_level) const {
  Entries::const_iterator ei;
  for (ei = _entries.begin(); ei != _entries.end(); ++ei) {
    (*ei).write(out, indent_level);
  }
}

/**
 * Adds one more entry to the end of the list, to catch all of the surfaces
 * that didn't get explicitly named.
 */
void QtessInputFile::
add_default_entry() {
  QtessInputEntry entry("*");
  entry.set_omit();
  _entries.push_back(entry);
}
