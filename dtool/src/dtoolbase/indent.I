/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file indent.I
 * @author drose
 * @date 1999-02-15
 */

/**
 * Writes a list of things to the indicated output stream, with a space
 * separating each item.  One or more lines will be written, and the lines
 * will automatically be broken such that no line exceeds max_col columns if
 * possible.
 */
template<class InputIterator>
void
write_long_list(std::ostream &out, int indent_level,
                InputIterator first, InputIterator last,
                std::string first_prefix, std::string later_prefix,
                int max_col) {
  if (later_prefix.empty()) {
    later_prefix = first_prefix;
  }

  if (first != last) {
    // We have to use an intermediate strstream object so we can count the
    // number of characters the item will have when it is output.
    std::ostringstream item;
    item << *first;
    std::string str = item.str();

    indent(out, indent_level) << first_prefix << str;
    int col = indent_level + (int)(first_prefix.length() + str.length());

    ++first;

    while (first != last) {
      std::ostringstream item;
      item << *first;
      std::string str = item.str();

      col += 1 + str.length();
      if (col > max_col) {
        out << "\n";
        indent(out, indent_level) << later_prefix << str;
        col = indent_level + (int)(later_prefix.length() + str.length());

      } else {
        out << " " << str;
      }

      ++first;
    }
    out << "\n";
  }
}
