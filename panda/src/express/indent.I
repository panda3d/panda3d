// Filename: indent.I
// Created by:  drose (15Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: write_long_list
//  Description: Writes a list of things to the indicated output
//               stream, with a space separating each item.  One or
//               more lines will be written, and the lines will
//               automatically be broken such that no line exceeds
//               max_col columns if possible.
////////////////////////////////////////////////////////////////////
template<class InputIterator>
void
write_long_list(ostream &out, int indent_level,
                InputIterator first, InputIterator last,
                string first_prefix, string later_prefix,
                int max_col) {
  if (later_prefix.empty()) {
    later_prefix = first_prefix;
  }

  if (first != last) {
    // We have to use an intermediate strstream object so we can
    // count the number of characters the item will have when it is
    // output.
    ostringstream item;
    item << *first;
    string str = item.str();

    indent(out, indent_level) << first_prefix << str;
    int col = indent_level + first_prefix.length() + str.length();

    ++first;

    while (first != last) {
      ostringstream item;
      item << *first;
      string str = item.str();

      col += 1 + str.length();
      if (col > max_col) {
        out << "\n";
        indent(out, indent_level) << later_prefix << str;
        col = indent_level + later_prefix.length() + str.length();

      } else {
        out << " " << str;
      }

      ++first;
    }
    out << "\n";
  }
}
