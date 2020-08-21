/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMiscFuncs.cxx
 * @author drose
 * @date 1999-01-16
 */

#include "pandabase.h"
#include "eggMiscFuncs.h"
#include "indent.h"

#include <ctype.h>

using std::ostream;
using std::string;


/**
 * Writes the string to the indicated output stream.  If the string contains
 * any characters special to egg, writes quotation marks around it.  If
 * always_quote is true, writes quotation marks regardless.
 */
ostream &
enquote_string(ostream &out, const string &str, int indent_level,
               bool always_quote) {
  indent(out, indent_level);

  // First, see if we need to enquote it.
  bool legal = !always_quote;
  string::const_iterator p;
  for (p = str.begin(); p != str.end() && legal; ++p) {
    legal = (isalnum(*p) || *p=='-' || *p=='_' || *p=='#' || *p=='.');
  }

  if (legal) {
    out << str;

  } else {
    out << '"';

    for (p = str.begin(); p != str.end(); ++p) {
      switch (*p) {
      case '"':
        // Can't output nested quote marks at all.
        out << "'";
        break;

      case '\n':
        // A newline necessitates ending the quotes, newlining, and beginning
        // again.
        out << "\"\n";
        indent(out, indent_level) << '"';
        break;

      default:
        out << *p;
      }
    }
    out << '"';
  }

  return out;
}


/**
 * A helper function to write out a 3x3 transform matrix.
 */
void
write_transform(ostream &out, const LMatrix3d &mat, int indent_level) {
  indent(out, indent_level) << "<Transform> {\n";

  indent(out, indent_level+2) << "<Matrix3> {\n";

  for (int r = 0; r < 3; r++) {
    indent(out, indent_level+3);
    for (int c = 0; c < 3; c++) {
      out << " " << mat(r, c);
    }
    out << "\n";
  }
  indent(out, indent_level+2) << "}\n";
  indent(out, indent_level) << "}\n";
}

/**
 * A helper function to write out a 4x4 transform matrix.
 */
void
write_transform(ostream &out, const LMatrix4d &mat, int indent_level) {
  indent(out, indent_level) << "<Transform> {\n";

  indent(out, indent_level+2) << "<Matrix4> {\n";

  for (int r = 0; r < 4; r++) {
    indent(out, indent_level+3);
    for (int c = 0; c < 4; c++) {
      out << " " << mat(r, c);
    }
    out << "\n";
  }
  indent(out, indent_level+2) << "}\n";
  indent(out, indent_level) << "}\n";
}
