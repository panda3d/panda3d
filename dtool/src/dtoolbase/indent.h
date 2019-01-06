/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file indent.h
 * @author drose
 * @date 1999-01-16
 */

#ifndef INDENT_H
#define INDENT_H

#include "dtoolbase.h"

/**
 * A handy function for doing text formatting.  This function simply outputs
 * the indicated number of spaces to the given output stream, returning the
 * stream itself.  Useful for indenting a series of lines of text by a given
 * amount.
 */
EXPCL_DTOOL_DTOOLBASE std::ostream &
indent(std::ostream &out, int indent_level);

/**
 * Writes a list of things to the indicated output stream, with a space
 * separating each item.  One or more lines will be written, and the lines
 * will automatically be broken such that no line exceeds max_col columns if
 * possible.
 */
template<class InputIterator>
void
write_long_list(std::ostream &out, int indent_level,
                InputIterator ifirst, InputIterator ilast,
                std::string first_prefix = "",
                std::string later_prefix = "",
                int max_col = 72);

#include "indent.I"

#endif
