/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileParseData.cxx
 * @author drose
 * @date 2004-10-07
 */

#include "xFileParseData.h"
#include "xLexerDefs.h"


/**
 *
 */
XFileParseData::
XFileParseData() :
  _parse_flags(0)
{
  // Save the line number, column number, and line text in case we detect an
  // error later and want to report a meaningful message to the user.
  _line_number = x_line_number;
  _col_number = x_col_number;
  _current_line = x_current_line;
}

/**
 * Reports a parsing error message to the user, showing the line and column
 * from which this object was originally parsed.
 */
void XFileParseData::
yyerror(const std::string &message) const {
  xyyerror(message, _line_number, _col_number, _current_line);
}
